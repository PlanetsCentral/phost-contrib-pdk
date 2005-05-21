/*
 *   Portable Host Development Kit
 *   EXAMPLE PROGRAM #4
 *
 * This file provides a simple template for a portable MASTER program.
 * The program creates all of the game files necessary for a first hosting.
 * There are no configuration options, all 11 races are created, their
 * homeworlds are equally spaced along a circle of radius 500 and centered
 * at (2000,2000), etc. If you want this program to do more, then do it!
 *
 * This file is copyrighted by the PDK authors Andrew Sterian, Thomas Voigt
 * and Steffen Pietsch in 1995. You may copy and distribute this file
 * freely as long as you retain this notice and explicitly document any
 * changes you make to this file (along with your name and date) before
 * distributing modified versions.
 *
 * This is an example program for instructional purposes only. It is not
 * guaranteed to work correctly, or even to necessarily compile on all
 * systems. You use this program at your own risk.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <memory.h>
#include <phostpdk.h>

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

static void doPlanetRandomize(void);
static void doHomeworlds(void);
static void doFreeShips(void);

/* This array stores the planet ID of each race's homeworld */
static Uns16 Homeworld[RACE_NR+1];

int
main(int argc, char *argv[])
{
    Uns16 race;

    /* Parse our command-line parameters. First parameter, if any, is the
    game directory. Second parameter, if present, is the root directory. */

    switch (argc) {
     case 3:
        gRootDirectory = argv[2];
        /* FALL THROUGH */

     case 2:
        gGameDirectory = argv[1];
        /* FALL THROUGH */

     case 1:
        break;

     default:
        fprintf(stderr,"Usage: %s  [GameDirectory [RootDirectory]]\n", argv[0]);
        exit(1);
    }

    /* Initialize the PDK */
    InitPHOSTLib();

    /* We need to read in the global data files so we can inspect hull
       types, cargo capacities, etc. */
    if (! ReadGlobalData()) exit(1);

    /* These are the files we must create:
           PDATA.HST
           SHIP.HST
           BDATA.HST
           MINES.HST
           NEXTTURN.HST
           GEN.HST

       Call the Initialize...() routines to initialize these data structures.
       This is the only way we can call the Write_...() routines without
       first calling the Read_...() routines. */

    InitializePlanets();
    InitializeShips();
    InitializeBases();
    InitializeMines();
    InitializeHostGen();

    /* Set a random seed based on the number of seconds since Jan. 1, 1970
       just so we don't get the same universe every time. */
    SetRandomSeed((Uns16) time(0));

    /* Now, put some random minerals on planets, add natives, set
       temperature, etc. */
    doPlanetRandomize();

    /* Make some planets into homeworlds */
    doHomeworlds();

    /* Give each race two free ships */
    doFreeShips();

    /* Mark all players as active */
    for (race = 1; race <= RACE_NR; race++) {
        PutPlayerIsActive(race, True);
    }

    /* Write out our universe. */
    if (! Write_Planets_File(PLANET_NR)) exit(1);
    if (! Write_Ships_File(SHIP_NR)) exit(1);
    if (! Write_Bases_File(PLANET_NR,0)) exit(1);
    if (! Write_Mines_File()) exit(1);
    if (! Write_HostGen_File()) exit(1);

    /* The turntime file NEXTTURN.HST is of 0 length after mastering */
    (void) OpenOutputFile("nextturn.hst", GAME_DIR_ONLY);

    /* Close down. */
    FreePHOSTLib();

    return 0;
}

static void doPlanetRandomize(void)
{
    Uns16 planet;
    Uns16 cargo;
    char  FC[4];

    for (planet = 1; planet <= PLANET_NR; planet++) {
        if (! IsPlanetExist(planet)) continue;

        /* Start setting attributes */
        PutPlanetOwner(planet, NoOwner);

        sprintf(FC,"%03u",RandomRange(1000));  /* random friendly code */
        PutPlanetFC(planet, FC);

        PutPlanetMines(planet, 0);
        PutPlanetFactories(planet, 0);
        PutPlanetDefense(planet, 0);

        for (cargo = NEUTRONIUM; cargo <= MOLYBDENUM; cargo++) {
            PutPlanetCargo(planet, cargo, RandomRange(200));
            PutPlanetCore(planet, cargo, RandomRange(10000));
            PutPlanetDensity(planet, cargo, 10 + RandomRange(90));
        }

        PutPlanetColTax(planet, 0);
        PutPlanetNatTax(planet, 0);
        PutPlanetColHappy(planet, 80);
        PutPlanetNatHappy(planet, 80);

        /* Decide if this planet is to have natives. */
        if (RandomRange(100) < 50) {
            PutPlanetNativePopulation(planet, 1000000UL + RandomRange(32767)*300UL);
            PutPlanetNativeType(planet, 1 + RandomRange(NumNativeTypes));
            PutPlanetNatGovm(planet, 1 + RandomRange(NumGovmTypes));
        } else {
            PutPlanetNativePopulation(planet, 0);
        }

        /* Temperature */
        PutPlanetTemp(planet, RandomRange(101));
    }
}

static void doHomeworlds(void)
{
    Uns16 index, bestPlanet;
    Int16 X, Y, DX, DY;
    Uns32 bestDistance, distance;
    float angle;
    Uns16 planet;
    Uns16 assigned[RACE_NR+1];
    Uns16 race;
    Uns16 cargo;

    /* Start off with a random angle from 0 to 2*PI */
    angle = RandomReal() * 2*M_PI;

    /* Indicate that all races have not been assigned homeworlds */
    memset(assigned, 0, sizeof(assigned));

    for (index = 1; index <= RACE_NR; index++, angle += (2*M_PI/RACE_NR)) {
        /* Determine the (X,Y) co-ordinates that are 500 LY away from
           (2000,2000) at the current angle. */
        X = (Int16) (500.0*cos(angle)) + 2000;
        Y = (Int16) (500.0*sin(angle)) + 2000;

        /* Find the planet nearest to these co-ordinates. */
        bestPlanet = 0;
        bestDistance = 100000UL;
        for (planet = 1; planet <= PLANET_NR; planet++) {
            if (! IsPlanetExist(planet)) continue;

            DX = PlanetLocationX(planet) - X;
            DY = PlanetLocationY(planet) - Y;
            distance = (Uns32) ((Int32)DX*DX + (Int32)DY*DY);
            if (distance < bestDistance) {
                bestDistance = distance;
                bestPlanet = planet;
            }
        }
        passert(bestPlanet != 0);

        /* We found a homeworld. Decide upon who owns it. */
        do {
            race = RandomRange(RACE_NR)+1;
        } while (assigned[race]);
        assigned[race] = True;
        PutPlanetOwner(bestPlanet, race);

        /* Save the homeworld */
        Homeworld[race] = bestPlanet;

        /* Now set up minerals and stuff for this homeworld. */
        PutPlanetMines(bestPlanet, 100);
        PutPlanetFactories(bestPlanet, 150);
        PutPlanetDefense(bestPlanet, 100);

        for (cargo = NEUTRONIUM; cargo <= MOLYBDENUM; cargo++) {
            PutPlanetCargo(bestPlanet, cargo, 200+RandomRange(200));
            PutPlanetCore(bestPlanet, cargo, 1000+RandomRange(5000));
        }
        PutPlanetDensity(bestPlanet, NEUTRONIUM, 20);
        PutPlanetDensity(bestPlanet, TRITANIUM, 20);
        PutPlanetDensity(bestPlanet, DURANIUM, 15);
        PutPlanetDensity(bestPlanet, MOLYBDENUM, 95);

        /* Give them some colonists, supplies, and money */
        PutPlanetCargo(bestPlanet, COLONISTS, 1000000UL);
        PutPlanetCargo(bestPlanet, SUPPLIES, 1000);
        PutPlanetCargo(bestPlanet, CREDITS, 3000);

        PutPlanetColTax(bestPlanet, 6);
        PutPlanetNatTax(bestPlanet, 0);
        PutPlanetColHappy(bestPlanet, 80);

        PutPlanetNativePopulation(bestPlanet, 0);
        PutPlanetNativeType(bestPlanet, NoNatives);

        /* Temperature */
        if (race == 7) {
            /* Assume crystals like it hot */
            PutPlanetTemp(bestPlanet, 100);
        } else {
            PutPlanetTemp(bestPlanet, 50);
        }

        /* Now give this planet a starbase */
        CreateBase(bestPlanet);  /* automatically sets owner from planet */
        PutBaseDefense(bestPlanet, 10);
        PutBaseFighters(bestPlanet, 20);
        PutBaseOrder(bestPlanet, Refuel);
        PutBaseTech(bestPlanet, ENGINE_TECH, 7);

        /* Put some components in storage. */
        PutBaseHulls(bestPlanet, 1, 1);   /* One free ship in slot 1 */
        PutBaseHulls(bestPlanet, 2, 1);   /* One free ship in slot 2 */
        PutBaseEngines(bestPlanet, 1, 4); /* Four free StarDrive 1 engines */
        PutBaseEngines(bestPlanet, 3, 2); /* Two free StarDrive 3 engines */
        PutBaseBeams(bestPlanet, 2, 8);   /* Eight free X-Rays */
        PutBaseTubes(bestPlanet, 2, 3);   /* Three free Proton torpedo tubes */
        PutBaseTorps(bestPlanet, 2, 20);  /* 20 free Proton torpedoes */
    }
}

static void doFreeShips(void)
{
    Uns16 race;
    Uns16 ship;
    Uns16 hullNumber;
    char FC[4];
    Uns16 ammo;

    /* Give 2 free ships to each race. These ships are the first two that
       they can build with Blasters, Mark 5 torps, and up to 20 fish. */

    for (hullNumber = 1; hullNumber <= 2; hullNumber++) {
        for (race=1; race <= RACE_NR; race++) {
            ship = CreateShip(race);

            /* Give it a hull type */
            PutShipHull(ship, TrueHull(race, hullNumber));

            /* Give it a name */
            PutShipName(ship, HullName(TrueHull(race, hullNumber), 0));

            /* Give it a friendly code */
            sprintf(FC, "%03u", RandomRange(1000));
            PutShipFC(ship, FC);

            /* Fuel it */
            PutShipCargo(ship, NEUTRONIUM, HullFuelCapacity(ShipHull(ship)));

            /* Give it HyperDrive 6 engines */
            PutShipEngine(ship, 6);

            /* Give it crew */
            PutShipCrew(ship, HullCrewComplement(ShipHull(ship)));

            /* Give it beam weapons */
            if (HullBeamNumber(ShipHull(ship)) > 0) {
                PutShipBeamNumber(ship, HullBeamNumber(ShipHull(ship)));
                PutShipBeamType(ship, 4);  /* blasters */
            }

            /* Give it tubes or bays */
            if (HullBays(ShipHull(ship)) > 0) {
                PutShipBays(ship, HullBays(ShipHull(ship)));
                ammo = 20;
            } else if (HullTubeNumber(ShipHull(ship)) > 0) {
                PutShipTubeNumber(ship, HullTubeNumber(ShipHull(ship)));
                PutShipTorpType(ship, 7);  /* Mark 5 */
                ammo = 20;
            } else {
                ammo = 0;
            }

            /* Give it ammo */
            ammo = min(ammo, HullCargoCapacity(ShipHull(ship)));
            PutShipAmmunition(ship, ammo);

            /* Place it at the homeworld. */
            PutShipLocationX(ship, PlanetLocationX(Homeworld[race]));
            PutShipLocationY(ship, PlanetLocationY(Homeworld[race]));

            /* Clear its waypoint */
            PutShipRelWaypointX(ship, 0);
            PutShipRelWaypointY(ship, 0);
        }
    }
}


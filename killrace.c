#include <stdio.h>
#include <string.h>
#include <time.h>
#include "phostpdk.h"
/*
    Revision History:

    v3.1: Jan. 9, 1998  Andrew Sterian
          Added call to ResetShipRemoteControl
          Added call to ReadGlobalData

    v3.2: May 18, 2000 Piotr Winiarczyk
          Added -k option.
          Added killing of mine fields, planet structures, cash and supplies.
          RootDirectory read.

          June 9, 2001
          Planet FC is randomized.

          November 6, 2001
          Clean up code.
*/

#define  AND   &&
#define  OR    ||
#define  EQ    ==
#define  NEQ   !=
#define  GT    >
#define  GE    >=
#define  LT 	<
#define  LE    =<

// Flags PW
Boolean No_Kill_Confirmation;
// Version
Uns16 pMajor=3;
Uns16 pMinor=2;

static void
version(void)
{
   Info("Portable KILLRACE v%u.%u",pMajor, pMinor);
}

static void
usage(char *argv[])
{
	ErrorExit(
"Usage: %s [option] Race [GameDirectory [RootDirectory]]\n"
"\n"
"          Race -- race number from 1 through 11\n"
"\n"
"Removes ONE race from the game. All of the player's planets, ships,\n"
"and starbases, are removed.\n"
"\n"
"options:\n"
"        -h      -- Print help and exit\n"
"        -k      -- Don't ask for kill confirmation.\n"
"        -v      -- Print version and exit\n"
					,argv[0]);
}

static Uns16 gKillRace = 0;

static void processOptions(int argc, char *argv[])
{
    int c;

    No_Kill_Confirmation=False;

    while ((c=pgetopt(argc, argv, "hHvkK")) NEQ EOF) {
		switch (c) {
		 case 'h': case 'H': default:
			usage(argv);

		 case 'v':
			version();
			exit(1);
		 case 'k': case 'K':
                   No_Kill_Confirmation=True;
                   break;

		}
	}

    if (poptind < argc) {
        if (strlen(argv[poptind]) EQ 0) usage(argv);

        gKillRace = atoi(argv[poptind++]);
        if (gKillRace < 1 OR gKillRace > 11)
            ErrorExit("\nIllegal race number: '%s'", argv[poptind-1]);

	} else usage(argv);

    if (poptind < argc)
        gGameDirectory = argv[poptind++];

    if (poptind < argc)
        gRootDirectory = argv[poptind++];

    if (poptind < argc) usage(argv);
}

int
main(int argc, char *argv[])
{
    Uns16 lIx;
	int retcode = 1;
    Boolean lShipRemoved[SHIP_NR+1];
    char FC[4];
    time_t now;

    processOptions(argc, argv);

    version();

    InitPHOSTLib();

    Info("");

    if (! ReadGlobalData()) goto done;
    if (! ReadHostData())   goto done;

    if (! PlayerIsActive(gKillRace))
        ErrorExit("Player %u is not active in this game. Exiting.",gKillRace);

// PW
    if (No_Kill_Confirmation==False)
    {
    Info(
"Please confirm that you wish to remove all of Player %u's\n"
"ships, planets, and starbases from the game. It would be wise\n"
"to ensure that you have a backup before you proceed.\n"
"\n"
"Type a 'k' to confirm (then press ENTER): ", gKillRace);

    if (getchar() != 'k')
        ErrorExit("Player %u's stuff has been left intact. Exiting.",gKillRace);
    }

    memset(lShipRemoved, 0, sizeof(lShipRemoved));

    for (lIx = 1; lIx <= SHIP_NR; lIx++) {
        if (IsShipExist(lIx) AND ShipOwner(lIx) EQ gKillRace) {
            DeleteShip(lIx);
            ResetShipRemoteControl(lIx);
            lShipRemoved[lIx] = True;
        }

        if (IsPlanetExist(lIx) AND PlanetOwner(lIx) EQ gKillRace) {
            if (IsBaseExist(lIx)) {
                BuildQueueInvalidate(lIx);
                DeleteBase(lIx);
            }
            PutPlanetCargo(lIx, COLONISTS, 0);
            PutPlanetOwner(lIx, NoOwner);
//PW
            PutPlanetColHappy(lIx,100);
            PutPlanetCargo(lIx, SUPPLIES, 0);
            PutPlanetCargo(lIx, CREDITS, 0);
            PutPlanetMines    (lIx,0);
            PutPlanetFactories(lIx,0);
            PutPlanetDefense  (lIx,0);
            time(&now);
            SetRandomSeed(now % 32000 );
            sprintf(FC,"%03d",RandomRange(1000));
            PutPlanetFC  (lIx,FC);
        }
// PW
        if (IsMinefieldExist(lIx) AND MinefieldOwner(lIx) EQ gKillRace )
           PutMinefieldUnits(lIx,0);

    }
    for (lIx = 1; lIx <= RACE_NR; lIx++)
        AllyDropRequest(gKillRace, lIx);

    PutPlayerIsActive(gKillRace, False);

    /* Now go through and clear tow/intercept missions on ships which
       were towing/intercepting a ship that we just deleted. */
    for (lIx = 1; lIx <= SHIP_NR; lIx++) {
        if (! IsShipExist(lIx)) continue;

        if (    (ShipMission(lIx) EQ Tow)
            AND lShipRemoved[ShipTowTarget(lIx)]
           )
            PutShipMission(lIx, NoMission);

        if (    (ShipMission(lIx) EQ Intercept)
            AND lShipRemoved[ShipInterceptTarget(lIx)]
           )
            PutShipMission(lIx, NoMission);

    }

    if (! WriteHostData()) goto done;

	retcode = 0;

done:
	FreePHOSTLib();

	return retcode;
}


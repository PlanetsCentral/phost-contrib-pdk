/*
 *   Portable Host Development Kit
 *   EXAMPLE PROGRAM #6
 *
 * This program implements PTEXTSIM, a simple text-only battle simulator.
 * This program is meant to be an example of how to interface to the combat
 * module in the PDK (i.e., the Combat() routine). The program prompts
 * for battle parameters of 2 ships and then proceeds to describe the
 * battle in textual fashion. The battle commentary is displayed on the
 * screen and is also stored in a PTEXTSIM.LOG file in the game directory.
 *
 * This program is fairly brainless. It does not handle ship-to-planet
 * combat, does not add mass compensation for Fed crew bonus, etc. It could,
 * however, make a good starting point for something a bit more interesting
 * and robust.
 *
 * This file is copyrighted by Andrew Sterian in 1996. You may copy and
 * distribute this file freely as long as you retain this notice and
 * explicitly document any changes you make to this file (along with your
 * name and date) before distributing modified versions.
 *
 * This is an example program for instructional purposes only. It is not
 * guaranteed to work correctly, or even to necessarily compile on all
 * systems. You use this program at your own risk.
 */

#define PTEXTSIM_VERSION_MAJOR 1
#define PTEXTSIM_VERSION_MINOR 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <phostpdk.h>

#define  AND   &&
#define  OR    ||
#define  EQ    ==
#define  NEQ   !=
#define  GT    >
#define  GE    >=
#define  LT 	<
#define  LE    =<

static Uns16 gRandSeed;

/* Display a brief command-line usage summary. Then exit. */

static void
usage(char *argv[])
{
    fprintf(stderr,
"Usage: %s [options] [GameDirectory [RootDirectory]]\n"
"\n"
"options:\n"
"        -h        -- prints this help summary and exits\n"
"        -v        -- prints program version and exits\n"
        ,argv[0]);
    exit(1);
}

/* Process our command-line options */

static void
processOptions(int argc, char *argv[])
{
    int c;

    while ((c=pgetopt(argc, argv, "hHv")) NEQ EOF) {
        switch (c) {
         case 'h': case 'H': default:
            usage(argv);

         case 'v':
            printf("Portable Text-Only Battle Simulator Version %u.%u\n",
                PTEXTSIM_VERSION_MAJOR, PTEXTSIM_VERSION_MINOR);
            exit(0);
        }
    }

    if (poptind < argc) {
        gGameDirectory = argv[poptind++];
    }
    if (poptind < argc) {
        gRootDirectory = argv[poptind++];
    }
    if (poptind < argc) {
        fprintf(stderr,"Too many parameters. First extraneous parameter is '%s'\n"
                       "Type '%s -h' for a usage summary.\n",
                       argv[poptind], argv[0]);
        exit(1);
    }
}

/* Prompt the user for a 16-bit unsigned value. The lower and upper
   limits on the allowed value are specified, as is the default value
   which will be returned if the user just presses ENTER. */

static Uns16 promptValue(const char *pPrompt, Uns16 pLower, Uns16 pUpper,
                                                            Uns16 pDefault)
{
    char lResponse[80];

    do {
        printf("%s [%u]: ", pPrompt, pDefault);

        if (fgets(lResponse, 79, stdin)) {
            Uns32 lVal;
            char *lEndPtr;
            char *lPtr;

            for (lPtr=lResponse; isspace(*lPtr); lPtr++) /* NULL */ ;
            if (*lPtr EQ 0) return pDefault; /* User just pressed ENTER */

            lVal = strtoul(lResponse, &lEndPtr, 0);
            if (lEndPtr) {
                while (isspace(*lEndPtr)) lEndPtr++;

                if (*lEndPtr) {
                    printf("\n!!! Illegal numeric value !!!\n\n");
                    continue;
                }
            }
            if (lVal < pLower OR lVal > pUpper) {
                printf("\n!!! Not in the range [%u,%u] !!!\n\n",pLower,pUpper);
                continue;
            }
            return (Uns16) lVal;
        } else exit(0);
    } while (1);
}

/* Prompt the user for a string value. The default string to return is
   specified in pDefault, and it will be the return value if the user
   just presses ENTER. The length of the string is currently hardcoded
   to be a maximum of 80 characters. */

static char *promptString(const char *pPrompt, const char *pDefault)
{
    char lResponse[80];

    do {
        printf("%s [%s]: ", pPrompt, pDefault);

        if (fgets(lResponse, 79, stdin)) {
            char *lPtr;

            for (lPtr = lResponse+strlen(lResponse)-1; isspace(*lPtr); *lPtr-- = 0)
                /* NULL */ ;

            for (lPtr=lResponse; isspace(*lPtr); lPtr++) /* NULL */ ;
            if (*lPtr EQ 0)
                return strcpy(MemCalloc(strlen(pDefault)+1,1),pDefault);

            return strcpy(MemCalloc(strlen(lResponse)+1,1),lResponse);
        } else exit(0);
    } while (1);
}

/* Prompt the user for all combat parameters for one ship. Fill in
   a Combat_Struct structure. */

static Boolean getParameters(Combat_Struct *pShip)
{
    Uns16 lHull, lBeamNum, lTubeNum;
    char lShipDefName[30];

    pShip->IsPlanet = False;

    lHull = promptValue("Enter hull number (1-105, 0 to quit program)", 0, 105, 0);
    if (lHull EQ 0) return False;

    printf(">>> %s\n\n", HullName(lHull, 0));

    sprintf(lShipDefName, "ShipHull #%u", lHull);
    pShip->Name = promptString("Enter ship name", lShipDefName);
    pShip->Race = promptValue("Enter owner race", 1, 11, 3);
    pShip->Mass = promptValue("Enter combat mass", 1, 32767, HullMass(lHull));

    pShip->NumBeams = promptValue("Enter number of beams", 0, 20, HullBeamNumber(lHull));
    if (pShip->NumBeams > 0) {
        pShip->BeamTech = promptValue("Enter beam tech level", 1, 10, 10);
    } else {
        pShip->BeamTech = 0;
    }

    pShip->NumTubes = promptValue("Enter number of tubes", 0, 10, HullTubeNumber(lHull));
    if (pShip->NumTubes > 0) {
        pShip->TorpTech = promptValue("Enter tube tech level", 1, 10, 10);
    } else {
        pShip->TorpTech = 0;
    }

    if (pShip->NumTubes EQ 0) {
        pShip->NumBays = promptValue("Enter number of fighter bays", 0, 20, HullBays(lHull));
    } else {
        pShip->NumBays = 0;
    }

    pShip->NumFighters = 0;
    pShip->NumTorps = 0;

    if (pShip->NumTubes > 0) {
        pShip->NumTorps = promptValue("Enter number of torpedoes", 0, 32767, HullCargoCapacity(lHull));
    }
    if (pShip->NumBays > 0) {
        pShip->NumFighters = promptValue("Enter number of fighters", 0, 32767, HullCargoCapacity(lHull));
    }

    pShip->Crew = promptValue("Enter ship crew count", 2, 32767, HullCrewComplement(lHull));
    pShip->Shields = promptValue("Enter shield level", 0, 100, 100);
    pShip->Damage = promptValue("Enter damage level", 0, 100, 0);

    return True;
}

/* Displays all relevant info on both combatants. */

static void printCombatInfo(const Combat_Struct *pShip1, const Combat_Struct *pShip2,
                                Boolean pAfterBattle)
{
    static const char *lOutcomeStr[] = {
        "Victorious!",
        "Captured",
        "Destroyed!",
        "Out of ammunition"
    };

    if (pAfterBattle) {
        Info("\nBattle Results:\n");
    } else {
        Info("\nCombatants:\n");
    }
    Info("%-39s %-39s", pShip1->Name, pShip2->Name);
    Info("%-39s %-39s", RaceLongName(pShip1->Race,0), RaceLongName(pShip2->Race,0));

    if (! pAfterBattle) {
        Info("Mass: %-33u Mass: %u", pShip1->Mass, pShip2->Mass);
        Info("Beam Number: %-26u Beam Number: %u", pShip1->NumBeams, pShip2->NumBeams);
        Info("Beam Tech: %-28u Beam Tech: %u", pShip1->BeamTech, pShip2->BeamTech);
        Info("Tube Number: %-26u Tube Number: %u", pShip1->NumTubes, pShip2->NumTubes);
        Info("Tube Tech: %-28u Tube Tech: %u", pShip1->TorpTech, pShip2->TorpTech);
    }
    Info("Torpedoes: %-28u Torpedoes: %u", pShip1->NumTorps, pShip2->NumTorps);
    if (! pAfterBattle) {
        Info("Fighter Bays: %-25u Fighter Bays: %u", pShip1->NumBays, pShip2->NumBays);
    }
    Info("Fighters: %-29u Fighters: %u", pShip1->NumFighters, pShip2->NumFighters);
    Info("Crew: %-33u Crew: %u", pShip1->Crew, pShip2->Crew);
    Info("Shields: %-30u Shields: %u", pShip1->Shields, pShip2->Shields);
    Info("Damage: %-31u Damage: %u", pShip1->Damage, pShip2->Damage);

    if (pAfterBattle) {
        Info("Outcome: %-30s Outcome: %s", lOutcomeStr[pShip1->Outcome],
                                           lOutcomeStr[pShip2->Outcome]);
    }

    if (! pAfterBattle) Info("Random Seed: %u",gRandSeed);
    Info("\n");
}

static Combat_Struct gShip[2];
static void initDrawing(void);

int
main(int argc, char *argv[])
{
    processOptions(argc, argv);

    InitPHOSTLib();

    if (! ReadGlobalData()) exit(1);
    if (! ReadHostData()) exit(1);

    gLogFile = OpenOutputFile("ptextsim.log", GAME_DIR_ONLY | TEXT_MODE);

    do {
        printf("\nEnter parameters for Ship #1:\n");
        if (! getParameters(&gShip[0])) break;

        printf("\nEnter parameters for Ship #2:\n");
        if (! getParameters(&gShip[1])) break;

        gRandSeed = promptValue("Enter random seed", 0, 32767, RandomRange(32767));
        SetRandomSeed(gRandSeed);

        printCombatInfo(&gShip[0], &gShip[1], False /* after battle */);

        initDrawing();
        Combat(&gShip[0], &gShip[1]);

        printCombatInfo(&gShip[0], &gShip[1], True /* after battle */);

        MemFree(gShip[0].Name);
        MemFree(gShip[1].Name);
    } while (1);

    fclose(gLogFile);

    FreePHOSTLib();

    return 0;
}

static int gFighterStatus[2][50];
static double gShipDamage[2];
static Int32 gLastDist;

static void initDrawing(void)
{
    memset(gFighterStatus, 0, sizeof(gFighterStatus));
    gShipDamage[0] = gShip[0].Damage;
    gShipDamage[1] = gShip[1].Damage;
    gLastDist = 100000L;
}

void DrawInitBeams(Uns16 pShip, Uns16 pNumBeams, Uns16 pCharge)
{
    Info("%s is powering up %u beams at %u%% charge",
        gShip[pShip].Name, pNumBeams, pCharge);
}

void DrawInitTubes(Uns16 pShip, Uns16 pNumTubes, Uns16 pCharge)
{
    Info("%s is powering up %u tubes at %u%% charge",
        gShip[pShip].Name, pNumTubes, pCharge);
}

void DrawBeamRecharge(Uns16 pShip, Uns16 pBeam, Uns16 pCharge)
{
    /* Too microscopic an event to write about */
}

void DrawTubeRecharge(Uns16 pShip, Uns16 pTube, Uns16 pCharge)
{
    /* Too microscopic an event to write about */
}

void DrawBayRecharge(Uns16 pShip, Uns16 pBay, Uns16 pCharge)
{
    /* Too microscopic an event to write about */
}

void DrawFighterAttack(Uns16 pShip, Uns16 pFighter, Int32 pPos)
{
    if (gFighterStatus[pShip][pFighter] NEQ 1) {
        Info("%s launches a fighter!", gShip[pShip].Name);
        gFighterStatus[pShip][pFighter] = 1;
    }
}

void DrawEraseFighterAttack(Uns16 pShip, Uns16 pFighter, Int32 pPos)
{
    /* Don't care */
}

void DrawFighterReturn(Uns16 pShip, Uns16 pFighter, Int32 pPos)
{
    gFighterStatus[pShip][pFighter] = 2;
}

void DrawEraseFighterReturn(Uns16 pShip, Uns16 pFighter, Int32 pPos)
{
    /* Don't care */
}

void DrawShipShields(Uns16 pShip, Int32 pPos, double pShields)
{
    if (pShields EQ 0) {
        Info("%s's shields have fallen!", gShip[pShip].Name);
    }
}

void DrawShipDamage(Uns16 pShip, double pDamage)
{
    if (pDamage - gShipDamage[pShip] > 10) {
        Info("%s is now %5.1lf%% damaged", gShip[pShip].Name, pDamage);
        gShipDamage[pShip] = pDamage;
    }
}

void DrawShipCaptured(Uns16 pShip)
{
    Info("%s has been captured", gShip[pShip].Name);
}

void DrawShipDestroyed(Uns16 pShip, Int32 pPos, Boolean pIsPlanet)
{
    Info("%s has been destroyed", gShip[pShip].Name);
}

void DrawStalemateResult(Boolean pIsPlanet2)
{
    Info("Both ships are out of ammunition.");
}

void DrawShipCrew(Uns16 pShip, double pCrew)
{
    /* Echh. */
}

void DrawTorpNumber(Uns16 pShip, Uns16 pTorps)
{
    /* Uchh. */
}

void DrawFighterNumber(Uns16 pShip, Uns16 pFighters)
{
    /* Ichh. */
}

void DrawFighterHit(Uns16 pShip, Uns16 pFighter, Int32 pPos, Int32 pEnemyPos)
{
    Info("Fighter hit on %s!", gShip[1-pShip].Name);
}

void DrawFighterHitFighter(Int32 pPos, Uns16 pFighter1, Uns16 pFighter2)
{
    /* Not enough info to say anything useful */
}

void DrawTorpHit(Uns16 pShip, Int32 pPos, Int32 pEnemyPos)
{
    Info("%s scores a torp hit on the enemy", gShip[pShip].Name);
}

void DrawTorpMiss(Uns16 pShip, Int32 pPos, Int32 pEnemyPos)
{
    Info("%s fires a torpedo and misses the enemy", gShip[pShip].Name);
}

void DrawBeamHitFighter(Uns16 pShip, Uns16 pBeam, Int32 pPos,
                             Uns16 lFighter, Int32 pEnemyFighterPos)
{
    Info("%s destroys an incoming enemy fighter", gShip[pShip].Name);
}

void DrawBeamMissedFighter(Uns16 pShip, Uns16 pBeam, Int32 pPos,
                             Uns16 lFighter, Int32 pEnemyFighterPos)
{
    Info("%s fires upon but misses an enemy fighter", gShip[pShip].Name);
}

void DrawBeamHitShip(Uns16 pShip, Int32 pPos, Int32 pEnemyPos)
{
    Info("%s scores a beam weapon hit on the enemy", gShip[pShip].Name);
}

void DrawBeamMissedShip(Uns16 pShip, Int32 pPos, Int32 pEnemyPos)
{
    Info("%s fires beam weapons but misses the enemy", gShip[pShip].Name);
}

void DrawEraseShip(Uns16 pShip, Int32 pPos)
{
    /* Ooch. */
}

void DrawShip(Uns16 pShip, Int32 pPos)
{
    /* Ycch. */
}

void DrawDistance(Int32 pDist)
{
    if (gLastDist - pDist > 10000L) {
        Info("Enemy ships are now %ld meters apart", pDist);
        gLastDist = pDist;
    }
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pgetopt.h"
#include "phostpdk.h"
#include "private.h"

#define VERSION_MAJOR PHOST_VERSION_MAJOR
#define VERSION_MINOR PHOST_VERSION_MINOR

/*
    Revision History:

    v3.1: Jan. 9, 1998  Andrew Sterian
          Added call to ResetShipRemoteControl
          Added call to ReadGlobalData

*/

#define  AND   &&
#define  OR    ||
#define  EQ    ==
#define  NEQ   !=
#define  GT    >
#define  GE    >=
#define  LT 	<
#define  LE    =<

static void
version(void)
{
	printf("Portable KILLRACE v%u.%u\n",VERSION_MAJOR,VERSION_MINOR);
}

static void
usage(char *argv[])
{
	fprintf(stderr,
"Usage: %s [option] Race [GameDirectory]\n"
"\n"
"          Race -- race number from 1 through 11\n"
"\n"
"Removes ONE race from the game. All of the player's planets, ships,\n"
"and starbases, are removed.\n"
"\n"
"options:\n"
"        -h      -- Print help and exit\n"
"        -v      -- Print version and exit\n"
					,argv[0]);
	exit(1);
}

static Uns16 gKillRace = 0;

static void processOptions(int argc, char *argv[])
{
	int c;

    while ((c=pgetopt(argc, argv, "hHv")) NEQ EOF) {
		switch (c) {
		 case 'h': case 'H': default:
			usage(argv);

		 case 'v':
			version();
			exit(1);
		}
	}

    if (poptind < argc) {
        if (strlen(argv[poptind]) EQ 0) usage(argv);

        gKillRace = atoi(argv[poptind++]);
        if (gKillRace < 1 OR gKillRace > 11) {
            fprintf(stderr,"\nIllegal race number: '%s'\n", argv[poptind-1]);
            exit(1);
        }
	} else usage(argv);

    if (poptind < argc) {
        gGameDirectory = argv[poptind++];
	}

    if (poptind < argc) usage(argv);
}

int
main(int argc, char *argv[])
{
    Uns16 lIx;
	int retcode = 1;
    Boolean lShipRemoved[SHIP_NR+1];

	processOptions(argc, argv);

	version();

	InitPHOSTLib();

    fprintf(stderr,"\n");

    if (! ReadGlobalData()) goto done;
    if (! ReadHostData()) goto done;

    if (! PlayerIsActive(gKillRace)) {
        fprintf(stderr,"Player %u is not active in this game. Exiting.\n",
                            gKillRace);
        goto done;
    }

    fprintf(stderr,
"Please confirm that you wish to remove all of Player %u's\n"
"ships, planets, and starbases from the game. It would be wise\n"
"to ensure that you have a backup before you proceed.\n"
"\n"
"Type a 'k' to confirm (then press ENTER): ", gKillRace);

    if (getchar() != 'k') {
        fprintf(stderr,"Player %u's stuff has been left intact. Exiting.\n",
                            gKillRace);
        exit(1);
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
            ChangePlanetOwner(lIx, NoOwner);
        }
    }
    for (lIx = 1; lIx <= OLD_RACE_NR; lIx++) {
        AllyDropRequest(gKillRace, lIx);
    }
    PutPlayerIsActive(gKillRace, False);

    /* Now go through and clear tow/intercept missions on ships which
       were towing/intercepting a ship that we just deleted. */
    for (lIx = 1; lIx <= SHIP_NR; lIx++) {
        if (! IsShipExist(lIx)) continue;

        if (    (ShipMission(lIx) EQ Tow)
            AND lShipRemoved[ShipTowTarget(lIx)]
           ) {
            PutShipMission(lIx, NoMission);
        }
        if (    (ShipMission(lIx) EQ Intercept)
            AND lShipRemoved[ShipInterceptTarget(lIx)]
           ) {
            PutShipMission(lIx, NoMission);
        }
    }

    if (! WriteHostData()) goto done;

	retcode = 0;

done:
	FreePHOSTLib();

	return retcode;
}


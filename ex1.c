/*
 *   Portable Host Development Kit
 *   EXAMPLE PROGRAM #1
 *
 * This program demonstrates basic usage of the PDK. It opens up some of
 * the data files, counts up the number of ships and planets for each
 * player and reports these statistics to the player in a message.
 *
 * This program is meant to be executed AFTER a host run. It writes the
 * messages directly to the player's RST file.
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
#include <memory.h>
#include <phostpdk.h>

int
main(int argc, char *argv[])
{
    Int16 dummy1, dummy2;
    Uns16 ships[RACE_NR+1], planets[RACE_NR+1], bases[RACE_NR+1], ix;
    Uns16 race;
    char  message[128];
    char  *messagePtr[2];

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

    if (! ReadGlobalData()) return 1;
    if (! ReadHostData()) return 1;

    /* Now go through and count items for each player. First initialize
       all counts to 0. */
    memset(ships, 0, sizeof(ships));
    memset(planets, 0, sizeof(planets));
    memset(bases, 0, sizeof(bases));

    for (ix=1; ix <= SHIP_NR; ix++) {
        if (IsShipExist(ix)) {
            ships[ShipOwner(ix)]++;
        }
    }

    for (ix=1; ix <= PLANET_NR; ix++) {
        if (IsPlanetExist(ix)) {
            planets[PlanetOwner(ix)]++;
            if (IsBaseExist(ix)) {
                bases[BaseOwner(ix)]++;
            }
        }
    }

    /* Send a message to each player with his counts. */
    for (race=1; race <= RACE_NR; race++) {
        Uns16 lScore;

        if (! PlayerIsActive(race)) continue;

        sprintf(message,
           "You have:\x0D"
           "    %u ships\x0D"
           "    %u planets\x0D"
           "    %u starbases\x0D",
           ships[race], planets[race], bases[race]);

        messagePtr[0] = message;

        lScore = ships[race] + planets[race] + bases[race];
        if (lScore < 10) {
            messagePtr[1] = "Your empire is puny. Give up.\x0D";
        } else if (lScore < 50) {
            messagePtr[1] = "I bet your enemies are expanding faster\x0D"
                            "than you are.\x0D";
        } else if (lScore < 100) {
            messagePtr[1] = "Here comes a Gorbie, ready for some fun?\x0D";
        } else if (lScore < 150) {
            messagePtr[1] = "I think your ally is about to change his\x0D"
                            "mind.\x0D";
        } else {
            messagePtr[1] = "No-one can stop you now. That's bad.\x0D"
                            "I had better delete all your bases.\x0D";
        }
        if (! WriteMessageToRST(race, 2, messagePtr)) {
            fprintf(stderr,"Unable to write messages to RST file for player %u\n",
                                                        race);
        }
    }

    /* Close down. No need to write any files back out to disk. */
    FreePHOSTLib();

    return 0;
}

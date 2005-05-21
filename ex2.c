/*
 *   Portable Host Development Kit
 *   EXAMPLE PROGRAM #2
 *
 * This file causes minefields to decay in a method different from the
 * normal host behavior. This program assumes that the decay rate of
 * mines and web mines has been set to 0 in the configuration (and prints
 * a warning if this isn't true). The method of decay in this program
 * causes a minefield's radius to decay by a constant amount every turn,
 * rather than having the number of minefield units decay by a constant
 * amount. This program must be run BEFORE host processing, e.g.,
 *
 *              MDECAY  gamedir
 *              PHOST   gamedir
 *
 * The amount of radius decay is configurable for each race. This
 * configuration is stored in an external file, MDECAY.INI, in the game
 * directory. The format of the file is as follows:
 *                MDecay1 MDecay2 ... MDecay11
 *                WDecay1 WDecay2 ... WDecay11
 * where MDecayN represents the minefield radius decay for race N, and
 * WDecayN represents the web mine radius decay for race N. Each decay
 * may be a floating point number.
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
    Uns16 count;
    float mdecay[RACE_NR+1], wdecay[RACE_NR+1];
    FILE *infile;
    float radius;
    Uns32 units;

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

    /* Try to find our config file. It's an error if it doesn't exist */
    infile = OpenInputFile("mdecay.ini", GAME_DIR_ONLY | TEXT_MODE);

    /* Now read 22 numbers from the file. They are floating point numbers.
       First read normal mine decays. */
    for (count=1; count <= RACE_NR; count++) {
        if (1 != fscanf(infile, "%f", mdecay + count)) {
            fprintf(stderr,
            "Mine decay for player %u is illegal or does not exist.\n",count+1);
            exit(1);
        }
    }

    /* Now read web mine decays. */
    for (count=1; count <= RACE_NR; count++) {
        if (1 != fscanf(infile, "%f", wdecay + count)) {
            fprintf(stderr,
            "Web mine decay for player %u is illegal or does not exist.\n",count+1);
            exit(1);
        }
    }

    if (! ReadGlobalData()) return 1;
    if (! ReadHostData()) return 1;

    if (gPconfigInfo->MineDecay != 0 || gPconfigInfo->WebDecay != 0) {
        fprintf(stderr,"WARNING: The mine/web decay rates have not been set to 0\n");
    }

    /* Go through each mine and decay it. */
    for (count = 1; count <= GetNumMinefields(); count++) {
        if (! IsMinefieldExist(count)) continue;

        radius = MinefieldRadius(count);
        units  = MinefieldUnits(count);

        if (IsMinefieldWeb(count)) {
            radius -= wdecay[MinefieldOwner(count)];
        } else {
            radius -= mdecay[MinefieldOwner(count)];
        }

        if (radius <= 0) {
            /* Minefield is gone */
            PutMinefieldUnits(count, 0);
        } else {
            /* Calculate the number of units lost as a result of the change
               in radius. */
            Uns32 unitsLost;

            unitsLost = units - radius*radius;
            PutMinefieldUnits(count, units - unitsLost);
        }
    }

    /* Write the mines file back to disk. No need to write the config file to
       disk since we haven't made any modifications. */
    if (! Write_Mines_File()) return 1;

    /* Close down. */
    FreePHOSTLib();

    return 0;
}


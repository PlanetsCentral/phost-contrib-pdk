/*
 *   Portable Host Development Kit
 *   EXAMPLE PROGRAM #10
 *
 * Hull functions testing program by Maurits van Rees. 2004-07-30.
 *
 * This program is based on the original EXAMPLE PROGRAM #1 from the
 * Portable Host Development Kit. It was only used as a template
 * though. This program does something completely different. It tests
 * some new functions in the PDK, like HullDoesAlchemy instead of
 * ShipDoesAlchemy. HullDoesAlchemy does not need host data in order
 * to work, so it can be used by client programs. Host data is read
 * anyway - if it exists - to test new functions like
 * ShipIsUnclonable.
 *
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
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <phostpdk.h>

extern void ReadHullfunc(void);
/* Maybe do #include <private.h> instead. Goal is using ReadHullfunc()
   without getting compile time warnings.
*/


int
main(int argc, char *argv[])
{
    Uns16 race;
    int i;

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
    if (! ReadHostData()) ReadHullfunc();
    /* When you are only testing the Hull* functions, a simple
       ReadHullfunc(); is enough instead of Read*Data(). */

    
    race = 9; /* Robots */
    for (i = 1; i <= HULL_NR; i++){
        if (HullDoesAlchemy(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullDoesAlchemy\n", i, race);}
        if (HullDoesRefinery(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullDoesRefinery\n", i, race);}
        if (HullDoesAdvancedRefinery(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullDoesAdvancedRefinery\n", i, race);}
        if (HullHeatsTo50(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullHeatsTo50\n", i, race);}
        if (HullCoolsTo50(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullCoolsTo50\n", i, race);}
        if (HullHeatsTo100(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullHeatsTo100\n", i, race);}
        if (HullCanHyperwarp(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullCanHyperwarp\n", i, race);}
        if (HullIsGravitonic(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullIsGravitonic\n", i, race);}
        if (HullScansAllWormholes(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullScansAllWormholes\n", i, race);}
        if (HullIsGamblingShip(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullIsGamblingShip\n", i, race);}
        if (HullIsAntiCloaking(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullIsAntiCloaking\n", i, race);}
        if (HullDoesImperialAssault(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullDoesImperialAssault\n", i, race);}
        if (HullDoesChunneling(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullDoesChunneling\n", i, race);}
        if (HullHasRamScoop(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullHasRamScoop\n", i, race);}
        if (HullDoesFullBioscan(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullDoesFullBioscan\n", i, race);}
        if (HullHasAdvancedCloak(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullHasAdvancedCloak\n", i, race);}
        if (HullCanCloak(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullCanCloak\n", i, race);}
        if (HullDoesBioscan(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullDoesBioscan\n", i, race);}
        if (HullHasGloryDevice(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullHasGloryDevice\n", i, race);}
        if (HullHasHEGloryDevice(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullHasHEGloryDevice\n", i, race);}
        if (HullIsUnclonable(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullIsUnclonable\n", i, race);}
        if (HullIsCloneableOnce(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullIsCloneableOnce\n", i, race);}
        if (HullIsUngiveable(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullIsUngiveable\n", i, race);}
        if (HullIsGiveableOnce(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullIsGiveableOnce\n", i, race);}
        if (HullHasLevel2Tow(i, race)){
            fprintf(stdout,"Hull %3d, race %2d: HullHasLevel2Tow\n", i, race);}
    }

    for (i = 1; i <= SHIP_NR; i++){
        if (IsShipExist(i) && ShipIsUnclonable(i)){
            fprintf(stdout,"Ship %3d: ShipIsUnclonable\n", i);}
    }

    /* Close down. No need to write any files back out to disk. */
    FreePHOSTLib();

    return 0;
}

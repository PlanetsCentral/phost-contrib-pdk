/*
 *   Portable Host Development Kit
 *   EXAMPLE PROGRAM #8
 *
 *  This program demonstrates use of score counting procedures
 *  of PDK. This file is based upon ex2.c
 *
 * This file is copyrighted by Piotr Winiarczyk.
 * You may copy and distribute this file freely as long as you retain
 * this notice and explicitly document any changes you make to this
 * file (along with your name and date) before distributing modified versions.
 *
 * This is an example program for instructional purposes only. It is not
 * guaranteed to work correctly, or even to necessarily compile on all
 * systems. You use this program at your own risk.
 */

//#include <stdio.h>
//#include <stdlib.h>
//#include <memory.h>
#include "phostpdk.h"

int
main(int argc, char *argv[])
{
    Uns16 Race;
    Uns32 ShipScore;
    Uns32 PlanetScore;
    Uns32 BaseScore;
    Uns32 MinefieldScore;
    Uns32 TotalScore;

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
        fprintf(stderr,"Usage: %s [GameDirectory [RootDirectory]]\n", argv[0]);
        exit(1);
    }

    /* Initialize the PDK */
    InitPHOSTLib();

    if (! ReadGlobalData()) return 1;
    if (! ReadHostData()) return 1;

    Info("PTScores - used");
    Info("");

    Info(" #   Ships Planets   Bases   Mines   Total");
    Info("------------------------------------------");
    for ( Race=1; Race <=RACE_NR; Race++)
    if (PlayerIsActive(Race))
    {
      ShipScore      = RaceScoreForShips(Race,USED_POINTS,ALL_SHIPS);
      PlanetScore    = RaceScoreForPlanets(Race,USED_POINTS);
      BaseScore      = RaceScoreForBases(Race,USED_POINTS);
      MinefieldScore = RaceScoreForMinefields(Race,USED_POINTS,True);
      TotalScore     = ShipScore + PlanetScore + BaseScore + MinefieldScore;
    
      Info("%2d %7d %7d %7d %7d %7d",Race,ShipScore,PlanetScore,
       BaseScore,MinefieldScore,TotalScore);
    }
    Info("------------------------------------------");

    Info("");
    Info("THost score");
    Info("");

    Info(" #   Ships Planets   Bases   Total");
    Info("------------------------------------");
    for ( Race=1; Race <=RACE_NR; Race++)
    if (PlayerIsActive(Race))
    {
      ShipScore      = RaceScoreForShips(Race,THOST_POINTS,ALL_SHIPS);
      PlanetScore    = RaceScoreForPlanets(Race,THOST_POINTS);
      BaseScore      = RaceScoreForBases(Race,THOST_POINTS);
      TotalScore     = ShipScore + PlanetScore + BaseScore ;
    
      Info("%2d %7d %7d %7d %7d",Race,ShipScore,PlanetScore,
       BaseScore,TotalScore);
    }
    Info("------------------------------------------");


    FreePHOSTLib();

    return 0;
}


#ifndef _PTSCORE_H_
#define _PTSCORE_H_

#include "phostpdk.h"

#ifndef MAXDIR
#define MAXDIR 255
#endif
#undef BETA_RELEASE

#define NUM_SCORES 11

typedef enum {UNUSED_SCORE=0, TOTAL_SCORE=0, USED_SCORE=1, PERCENTAGE_SCORE=2,
              BASE_SCORE=3, PLANET_SCORE=4, SHIP_SCORE=5, CAPITAL_SCORE=6,
              NumPointScores=7,
              PLANETS=7, BASES=8, SHIPS=9, FREIGHTERS=10, NumScoringMethods=11}
Score_Def;

typedef Uns32 Score_Array[NUM_SCORES];

/*
 * The following constants represent how many each object is worth.
 * The USED score of a class of objects (planets, etc.) can be modified
 * by a factor, so if you want an economy score then increase the
 * planet and freighter modifiers, but if you want a military score then
 * increase the warship modifier.
 *
 * Space mines are handled special : A mine field is always worth the score
 * as if it would be made out of the most efficient torps (where efficiency
 * means the ratio mines_per_torp/torp_score
 *
 * To leave the possibility of fractional values you can change the
 * ScoreModifier : All scores will be divided by this value.
 * So if you set it to 10, multiply every value by 10 but set CreditScore
 * to 5 then everything will remain as it was except that credits are worth
 * 0.5 points.
 */
static const Uns32 FactoryScore=4,       /* score for one factory */
                   MineScore=5,          /* score for one planetary mine */
                   DefenseScore=11,      /* score for one defense post */
                   BaseDefScore=13,      /* score for one base defense post */
                   MineralScore=3,       /* score for one kt of a mineral */
                   FuelScore=0,          /* score for one kt fuel */
                   SupplyScore=1,        /* score for one kt supplies */
                   CreditScore=1,        /* score for one MC */
                   FighterScore=20,      /* score for one fighter */
                   BaseBuiltScore=3486,  /* base construction : 402 T, 120 D, 340 M, 900 MC */

                   BaseModifier=1,       /* factor for starbase score */
                   PlanetModifier=1,     /* factor for planetary score */
                   FreighterModifier=1,  /* factor for ships without weapons */
                   WarshipModifier=1,    /* factor for ships with weapons */
                   SpaceMineModifier=1,  /* score for one space mine */

                   ScoreDivisor=1;

static char ScoreNames[NUM_SCORES][10]={"total","used","%","base","planet",
                                        "ships","warships","plan","base",
                                        "cap", "ship"};

static const char    *gVersionString="v1.4";

extern Score_Array  gScores[12];
extern Uns16        gTeams[12];
extern Boolean      IsScoreEnabled[NUM_SCORES];
extern Score_Def    SortKey;

void LoadTeams();
void GenerateTeams();
void GenerateScore();

#endif

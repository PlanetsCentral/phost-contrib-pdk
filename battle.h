#ifndef _BATTLE_H_
#define _BATTLE_H_

#include "phostpdk.h"
#include "private.h"

#define          MAX_FIGHTER_LIMIT   50

/* This defines the various things that a fighter could be doing. Currently,
   there are MAX_FIGHTER_OUT fighter positions which may either be
   occupied or free at any time. If a fighter position is free (F_IDLE)
   then a fighter can be launched in that position. Otherwise, the fighter
   position is not free and the fighter occupying that position is either
   attacking (F_ATTACKING, heading towards the enemy) or returning
   (F_RETURNING, returning back to the ship, away from the enemy). */
typedef enum {
    F_IDLE=0,           /* This fighter position is idle and ready for use */
    F_ATTACKING,        /* This fighter is attacking in this position */
    F_RETURNING         /* The fighter in this position is returning to the ship */
} FStatus_Def;

/* This defines the battle status for each ship */
typedef struct {
        /* Static items. Initialized once and do not change thereafter. */
    Uns16   mBeamChargeRate;            /* Beam recharge rate */
    Uns16   mTubeChargeRate;            /* Torp tube recharge rate */
    Uns16   mBayChargeRate;             /* Fighter bay recharge rate */
    Uns16   mBeamHitOdds;               /* Probability of beams hitting */
    Uns16   mTorpHitOdds;               /* Probability of torps hitting */

        /* Dynamic items. Can change every battle tick. */
    Int32   mPos;           /* Position along 1-D X-axis, measured in meters
                               with the center of the screen being X=0 */
    Int16   mDir;           /* Movement direction (+1, -1) for ship and fighters */
                            /* Ship 0 (on the left) moves to the right (mDir=1) */
                            /* while ship 1 (on the right) moves to the left (mDir = -1) */
                            /* unless it's a planet, in which case it doesn't move */
    Uns16   mBeamCharge[MAX_BEAMS];     /* Charge level X10 of each beam */
    Uns16   mTubeCharge[MAX_TUBES];     /* Charge level X10 of each tube */
    Uns16   mBayCharge[MAX_PLANET_BAYS];       /* Charge level X10 of fighter bays */
    FStatus_Def mFStatus[MAX_FIGHTER_LIMIT]; /* Status of each possible fighter */
    Uns16   mFLaunched;                 /* Number of fighters out of ship */
    Int32   mFPos[MAX_FIGHTER_LIMIT];     /* Fighter position along 1-D X-axis */
    Uns16   mFStrike[MAX_FIGHTER_LIMIT];  /* Number of fighter hits left for each fighter */
    Uns32   mNextLaunchTick;            /* Tick time of next allowable launch */
    double  mShields;                   /* Current fractional ship shields */
    double  mDamage;                    /* Current fractional ship damage */
    double  mCrew;                      /* Current fractional ship crew */
    Combat_Struct *mInfo;               /* Pointer to combat values */
} BattleStatus_Struct;

#endif /* _BATTLE_H_ */


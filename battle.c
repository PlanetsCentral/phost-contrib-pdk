
/****************************************************************************
All files in this distribution are Copyright (C) 1995-2000 by the program
authors: Andrew Sterian, Thomas Voigt, and Steffen Pietsch.
You can reach the PHOST team via email (phost@gmx.net).

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*****************************************************************************/

#include <memory.h>
#include <math.h>
#include "phostpdk.h"
#include "private.h"

typedef struct {
  Uns16 BayRechargeRate;
  Int16 BayRechargeBonus;
  Uns16 BeamRechargeRate;
  Int16 BeamRechargeBonus;
  Uns16 TubeRechargeRate;
  Uns16 BeamHitFighterCharge;
  Uns16 BeamHitShipCharge;
  Uns32 TorpFiringRange;
  Uns32 BeamFiringRange;
  Uns16 TorpHitOdds;
  Uns16 BeamHitOdds;
  Int16 BeamHitBonus;
  Uns16 StrikesPerFighter;
  Uns16 FighterKillOdds;
  Uns16 FighterBeamExplosive;
  Uns16 FighterBeamKill;
  Uns16 ShipMovementSpeed;
  Uns16 FighterMovementSpeed;
  Uns16 BayLaunchInterval;
  Uns16 MaxFightersLaunched;
  Int16 TorpHitBonus;
  Int16 TubeRechargeBonus;
  Uns16 ShieldDamageScaling;
  Uns16 HullDamageScaling;
  Uns16 CrewKillScaling;
  Int16 ExtraFighterBays;
  Uns32 BeamHitFighterRange;
  Uns32 FighterFiringRange;
} CombatConfig_Struct;

#ifndef MICROSQUISH

/* This is the configurable version of MAX_FIGHTER_LIMIT. We must always have
    MAX_FIGHTER_OUT <= MAX_FIGHTER_LIMIT
*/
#define MAX_FIGHTER_OUT(x) (gCombatConfig[x]->MaxFightersLaunched)

/* This is the speed at which ships travel relative to the time tick of
   combat */
#define SHIP_MOVEMENT_SPEED(x) (gCombatConfig[x]->ShipMovementSpeed)

/* This is the fighter travel speed relative to the time tick of combat */
#define FIGHTER_MOVEMENT_SPEED(x) (gCombatConfig[x]->FighterMovementSpeed)

/* These are the starting positions for the two ships */
static const Int16 STARTING_POSITION[2] = { -29000, 29000 }; /* meters */

/* This is the minimum distance that two ships will reach between each other. */
#define STANDOFF_DISTANCE (gConfigInfo->StandoffDistance)

/* This defines the minimum charge level needed for a beam to fire
   on an enemy fighter or on the enemy ship, and for torps. As for
   the recharge rates, these are multiplied by 10. That is, 500 means
   the beam must be 50% charged.
*/
#define MIN_BEAM_CHARGE_FOR_FIGHTERS(x) (gCombatConfig[x]->BeamHitFighterCharge)
#define MIN_BEAM_CHARGE_FOR_SHIPS(x)    (gCombatConfig[x]->BeamHitShipCharge)
static const Uns16 MIN_TUBE_CHARGE = 1000; /* tubes only fire when fully

                                              charged */

/* This defines the minimum distances for ships to fire beams/torps and
   for ships to fire on enemy fighters and for fighters to hit ships */
#define MIN_BEAM_FIGHTER_HIT_DISTANCE(x) (gCombatConfig[x]->BeamHitFighterRange)
#define MIN_TORP_SHIP_HIT_DISTANCE(x)  (gCombatConfig[x]->TorpFiringRange)
#define MIN_BEAM_SHIP_HIT_DISTANCE(x)  (gCombatConfig[x]->BeamFiringRange)
#define MIN_FIGHTER_SHIP_HIT_DISTANCE(x) (gCombatConfig[x]->FighterFiringRange)

/* This is an aborted attempt at giving planets a boost by letting them
   fire torps on enemy ships before enemy ships have that chance. Changing
   the + 0000 below to some positive number gives them this advantage. */
#define MIN_TORP_PLANET_HIT_DISTANCE(x) (gCombatConfig[x]->TorpFiringRange + 0000)

/* This defines the fighter kill/damage power */
#define FIGHTER_DAMAGE_POWER(x) (gCombatConfig[x]->FighterBeamExplosive)
#define FIGHTER_KILL_POWER(x)   (gCombatConfig[x]->FighterBeamKill)

/* This defines the number of strikes that one fighter gets on the enemy
   during a single pass */
#define FIGHTER_HITS(x)    (gCombatConfig[x]->StrikesPerFighter)

/* This is the percentage chance that a fighter which is at
   the same X-position as an enemy fighter will kill the enemy fighter */
#define FIGHTER_HIT_FIGHTER_PROBABILITY(x) (gCombatConfig[x]->FighterKillOdds)

/* This defines the minimum inter-fighter launch time (in ticks). That is,
   fighters will be launched no faster than 1 every TICKS_PER_FIGHTER_LAUNCH
   ticks */
#define TICKS_PER_FIGHTER_LAUNCH(x) (gCombatConfig[x]->BayLaunchInterval)

#include "battle.h"

/* This value cannot be changed arbitrarily. Make sure you look at all
   the places where it is used before changing it. Prior to version 2.4,
   this value was 125. As of version 2.4, this value is 128. */
static int METERS_PER_PIXEL = 128;

/* This array will hold the status structures for our two combatants */
static BattleStatus_Struct *gInitialStatus[2] = { 0, 0 };

/* This array holds pointers to status as used by the combat routines.
   Initially, they will probably just be set to gInitialStatus but they
   don't have to be. */
static BattleStatus_Struct *gStatus[2] = { 0, 0 };

/* These hold configuration information for the current two combatants.
   These will change from battle to battle as different players fight. */
static CombatConfig_Struct *gCombatConfig[2] = { 0, 0 };

/* This is the clock tick of the battle. */
static Uns32 gBattleTick = 0;

/* This function returns the probability (in the range [0,100+]) that a beam
   weapon hits its target. */

static Uns16
BeamHitOdds(Uns16 pBeamTech, const CombatConfig_Struct * pConfig)
{
  Int16 lOdds;

  passert(pConfig NEQ 0);

  lOdds =
        (Int16) pConfig->BeamHitOdds +
        (Int16) (((Int32) BeamKillPower(pBeamTech) +
              (Int32) BeamDestructivePower(pBeamTech))
        * (Int32) (pConfig->BeamHitBonus)
        / 100L);
  lOdds = max(lOdds, 0);
  return (Uns16) lOdds;
}

/* This function returns the probability (in the range [0,100+]) that a
   torp hits its target */

static Uns16
TorpHitOdds(Uns16 pTorpTech, const CombatConfig_Struct * pConfig)
{
  Int16 lOdds;

  passert(pConfig NEQ 0);

  lOdds =
        (Int16) pConfig->TorpHitOdds +
        (Int16) (((Int32) TorpKillPower(pTorpTech) +
              (Int32) TorpDestructivePower(pTorpTech))
        * (Int32) (pConfig->TorpHitBonus)
        / 100L);
  lOdds = max(lOdds, 0);
  return (Uns16) lOdds;
}

/* This function returns a recharge value for fighter bays. A fully charged
   bay is at a level of 1000. */

static Uns16
BayRechargeRate(Uns16 pNumBays, const CombatConfig_Struct * pConfig)
{
  Int16 lRate;

  passert(pConfig NEQ 0);

  lRate =
        (Int16) pConfig->BayRechargeRate +
        (pConfig->BayRechargeBonus * (Int16) pNumBays);
  lRate = max(lRate, 1);

  return (Uns16) lRate;
}

/* This function returns a recharge value for beam banks. A fully charged
   bank is at a level of 1000. */

static Uns16
BeamRechargeRate(Uns16 pBeamTech, const CombatConfig_Struct * pConfig)
{
  Int16 lRate;

  passert(pConfig NEQ 0);

  lRate =
        (Int16) pConfig->BeamRechargeRate +
        (Int16) ((Int32) (BeamKillPower(pBeamTech) +
              BeamDestructivePower(pBeamTech)) *
        (Int32) (pConfig->BeamRechargeBonus) / 100L);
  lRate = max(lRate, 1);
  return (Uns16) lRate;
}

/* This function returns a recharge value for torp tubes. */

static Uns16
TubeRechargeRate(Uns16 pTubeTech, const CombatConfig_Struct * pConfig)
{
  Int16 lRate;

  passert(pConfig NEQ 0);

  lRate =
        (Int16) pConfig->TubeRechargeRate +
        (Int16) ((Int32) (TorpKillPower(pTubeTech) +
              TorpDestructivePower(pTubeTech)) *
        (Int32) (pConfig->TubeRechargeBonus) / 100L);
  lRate = max(lRate, 1);
  return (Uns16) lRate;
}

/* This function returns the amount of shield damage suffered by a ship as
   a result of a combat hit */

static double
ShieldDamage(Uns16 pBoom, Uns16 pMass, const CombatConfig_Struct * pConfig)
{
  double lShieldDamage;

  passert(pConfig NEQ 0);

  lShieldDamage = (double) pBoom * (double) (pConfig->ShieldDamageScaling)
        / (double) (pMass + 1);

  /* Limiting for sanity */
  if (lShieldDamage > 10000)
    lShieldDamage = 10000;

  if (!gConfigInfo->AlternativeCombat) {
    /* Any kind of hit will do at least 1 percent damage */
    return (Uns16) (lShieldDamage + 1.5); /* Tim's formula */
  }
  else {
    return lShieldDamage;
  }
}

/* This function returns the amount of hull damage (i.e., "damage") suffered
   by a ship as a result of a combat hit */

static double
HullDamage(Uns16 pBoom, Uns16 pMass, const CombatConfig_Struct * pConfig)
{
  double lDamage;

  passert(pConfig NEQ 0);

  if (gConfigInfo->AlternativeCombat) {
    lDamage = (double) pBoom;
  }
  else {
    lDamage = ShieldDamage(pBoom, pMass, pConfig);
  }

  lDamage = lDamage * (double) (pConfig->HullDamageScaling)
        / (double) (pMass + 1);

  /* Limiting for sanity */
  if (lDamage > 10000)
    lDamage = 10000;

  if (!gConfigInfo->AlternativeCombat) {
    /* Do at least 1% damage */
    return (Uns16) (lDamage + 1.5); /* Tim's formula */
  }
  else {
    return lDamage;
  }
}

/* This function returns the number of crew killed as a result of a combat
   hit */

static double
CrewKilled(Uns16 pKill, Uns16 pMass, const CombatConfig_Struct * pConfig)
{
  double lCrewKill;

  passert(pConfig NEQ 0);

  lCrewKill = (double) pKill * (double) (pConfig->CrewKillScaling)
        / (double) (pMass + 1);

  if (!gConfigInfo->AlternativeCombat) {
    return (Uns16) (lCrewKill + 0.5); /* Tim's formula */
  }
  else {
    return lCrewKill;
  }
}

/* This routine recharges the fighter bays. */

static void
doBayRecharge(void)
{
  Uns16 lCount;
  Uns16 lBay;
  Uns16 lBayLimit;

  for (lCount = 0; lCount < 2; lCount++) {
    BattleStatus_Struct *lCurrPtr = gStatus[lCount];

    lBayLimit = lCurrPtr->mInfo->NumBays;
    for (lBay = 0; lBay < lBayLimit; lBay++) {
      if (lCurrPtr->mBayCharge[lBay] < 1000) {
        lCurrPtr->mBayCharge[lBay] += RandomRange(lCurrPtr->mBayChargeRate);

        DrawBayRecharge(lCount, lBay, lCurrPtr->mBayCharge[lBay] / 10);
      }
    }
  }
}

/* This routine recharges each beam weapon */

static void
doBeamRecharge(void)
{
  Uns16 lCount;
  Uns16 lBeamLimit;

  for (lCount = 0; lCount < 2; lCount++) {
    BattleStatus_Struct *lCurrPtr = gStatus[lCount];
    Uns16 lBeam;

    lBeamLimit = lCurrPtr->mInfo->NumBeams;
    for (lBeam = 0; lBeam < lBeamLimit; lBeam++) {
      if (lCurrPtr->mBeamCharge[lBeam] < 1000) {
        lCurrPtr->mBeamCharge[lBeam] +=
              RandomRange(lCurrPtr->mBeamChargeRate);

        DrawBeamRecharge(lCount, lBeam, lCurrPtr->mBeamCharge[lBeam] / 10);
      }
    }
  }
}

/* This routine recharges each torp tube */

static void
doTubeRecharge(void)
{
  Uns16 lCount;
  Uns16 lTubeLimit;

  for (lCount = 0; lCount < 2; lCount++) {
    BattleStatus_Struct *lCurrPtr = gStatus[lCount];
    Uns16 lTube;

    lTubeLimit = lCurrPtr->mInfo->NumTubes;
    for (lTube = 0; lTube < lTubeLimit; lTube++) {
      if (lCurrPtr->mTubeCharge[lTube] < 1000) {
        lCurrPtr->mTubeCharge[lTube] +=
              RandomRange(lCurrPtr->mTubeChargeRate);

        DrawTubeRecharge(lCount, lTube, lCurrPtr->mTubeCharge[lTube] / 10);
      }
    }
  }
}

/* This routine launches a fighter if available. To launch a fighter, the
   following must be true:
        1. The bay charge level of a bay must be at least 1000
        2. There must be an empty slot in the mFStatus array for this ship
        3. The ship has fighters (if it has fighters, it has bays)
*/

static void
doFighterLaunch(void)
{
  Uns16 lCount;
  Uns16 lFighter;
  Uns16 lBay;
  Uns16 lBayLimit;

  for (lCount = 0; lCount < 2; lCount++) {
    BattleStatus_Struct *lCurrPtr = gStatus[lCount];

    if ((lCurrPtr->mInfo->NumFighters EQ 0)
          OR(lCurrPtr->mFLaunched EQ MAX_FIGHTER_OUT(lCount))
          OR(lCurrPtr->mNextLaunchTick > gBattleTick) /* NEW */
          )
      continue;

    /* First, find a bay that can launch a fighter */
    lBayLimit = lCurrPtr->mInfo->NumBays;
    for (lBay = 0; lBay < lBayLimit; lBay++) {
      if (lCurrPtr->mBayCharge[lBay] >= 1000)
        break;
    }
    if (lBay >= lBayLimit)
      continue;

    /* Found a bay, now find a fighter slot */
    for (lFighter = 0; lFighter < MAX_FIGHTER_OUT(lCount); lFighter++) {
      if (lCurrPtr->mFStatus[lFighter] EQ F_IDLE) {
        /* Launch it */
        lCurrPtr->mFStatus[lFighter] = F_ATTACKING;
        lCurrPtr->mFLaunched++;
        lCurrPtr->mInfo->NumFighters--;
        lCurrPtr->mBayCharge[lBay] = 0;
        lCurrPtr->mFPos[lFighter] = lCurrPtr->mPos;
        lCurrPtr->mFStrike[lFighter] = FIGHTER_HITS(lCount);
        lCurrPtr->mNextLaunchTick =
              gBattleTick + TICKS_PER_FIGHTER_LAUNCH(lCount);

        DrawFighterNumber(lCount, lCurrPtr->mInfo->NumFighters);
        DrawFighterAttack(lCount, lFighter, lCurrPtr->mPos);
        DrawBayRecharge(lCount, lBay, 0);
        break;
      }
    }
    passert(lFighter < MAX_FIGHTER_OUT(lCount)); /* Must have found one */
  }
}

/* This routine is responsible for moving fighters along towards the enemy
   or back towards the home ship */

static void
doMoveFighters(void)
{
  Uns16 lCount;
  Uns16 lFighter;
  Uns16 lRemaining;

  for (lCount = 0; lCount < 2; lCount++) {
    BattleStatus_Struct *lCurrPtr = gStatus[lCount];

    /* Quick check to save a long search */
    if ((lRemaining = lCurrPtr->mFLaunched) EQ 0)
      continue;

    for (lFighter = 0; lRemaining > 0 AND lFighter < MAX_FIGHTER_OUT(lCount);
          lFighter++) {
      switch (lCurrPtr->mFStatus[lFighter]) {
      case F_IDLE:
        break;

      case F_ATTACKING:
        /* Erase the fighter at the old position, update the position then
           draw at the new position */
        DrawEraseFighterAttack(lCount, lFighter, lCurrPtr->mFPos[lFighter]);

        lCurrPtr->mFPos[lFighter] += (Int32) FIGHTER_MOVEMENT_SPEED(lCount)
              * lCurrPtr->mDir;
        DrawFighterAttack(lCount, lFighter, lCurrPtr->mFPos[lFighter]);
        lRemaining--;
        break;

      case F_RETURNING:
        /* Same concept as above but check for fighter travelling back behind 
           our ship. Once the fighter crosses our ship position, bring it
           back into the ship */
        DrawEraseFighterReturn(lCount, lFighter, lCurrPtr->mFPos[lFighter]);

        lCurrPtr->mFPos[lFighter] -= (Int32) FIGHTER_MOVEMENT_SPEED(lCount)
              * lCurrPtr->mDir;
        if (((lCurrPtr->mDir > 0)
                    AND(lCurrPtr->mFPos[lFighter] < lCurrPtr->mPos)
              )
              OR((lCurrPtr->mDir < 0)
                    AND(lCurrPtr->mFPos[lFighter] > lCurrPtr->mPos)
              )
              ) {
          /* Fighter has returned to ship */
          lCurrPtr->mFStatus[lFighter] = F_IDLE;
          lCurrPtr->mFLaunched--;
          lCurrPtr->mInfo->NumFighters++;
          DrawFighterNumber(lCount, lCurrPtr->mInfo->NumFighters);
        }
        else {
          DrawFighterReturn(lCount, lFighter, lCurrPtr->mFPos[lFighter]);
        }
        lRemaining--;
        break;
      }
    }
  }
}

/* This routine is called whenever a ship suffers damage, be it from
   a beam weapon, torpedo, or fighter. pShip is the ship suffering
   damage while pKill and pBoom are the kill power and destructive
   power of the damage, respectively. If the ship-to-damage is destroyed
   or captured, this routine returns True, otherwise False. */

static Boolean
doDamageShip(Uns16 pShip, Uns16 pKill, Uns16 pBoom)
{
  /* This variable indicates how much of the kill and destructive power
     actually causes damage as opposed to just lowering shields. It is 100 if 
     the ship has no shields, 0 if the hit hasn't lowered the shields yet, or 
     something in between if the shields were lowered by this hit but there's 
     hit power left over */
  Uns16 lDamagePercent = 100;
  double lCrewKill;
  BattleStatus_Struct *lShipPtr = gStatus[pShip];

  if (pKill EQ 0)
    pKill = 1;                  /* Bad things happen if these are 0 */
  if (pBoom EQ 0)
    pBoom = 1;

  /* First, lower shields if any */
  if (lShipPtr->mShields > 0) {
    double lShieldDamage = ShieldDamage(pBoom, lShipPtr->mInfo->Mass,
          gCombatConfig[pShip]);

    if (lShieldDamage >= lShipPtr->mShields) {
      /* Pro-rate the amount of oomph from the blast that is left over to do
         damage */
      lDamagePercent = (Uns16)
            (100UL * (lShieldDamage - lShipPtr->mShields) / lShieldDamage);

      lShipPtr->mShields = 0;
      DrawShipShields(pShip, lShipPtr->mPos, 0);

      /* Redraw the ship to update the shields */
      DrawEraseShip(pShip, lShipPtr->mPos);
      DrawShip(pShip, lShipPtr->mPos);
    }
    else {
      lShipPtr->mShields -= lShieldDamage;
      DrawShipShields(pShip, lShipPtr->mPos, lShipPtr->mShields);
      return False;             /* No crew kill until shields down */
    }
  }

  /* Now do damage, if any */
  if (lDamagePercent > 0) {
    double lDamage = HullDamage(pBoom, lShipPtr->mInfo->Mass,
          gCombatConfig[pShip]);

    lDamage = lDamage * lDamagePercent / 100;

    if (((lDamage + lShipPtr->mDamage >= 99.5)
                AND(EffRace(lShipPtr->mInfo->Race) NEQ Lizards)
          )
          OR(lDamage + lShipPtr->mDamage >= 149.5)
          ) {
      lShipPtr->mInfo->Outcome = COMBAT_DESTROYED;
      lShipPtr->mDamage += lDamage;
      DrawShipDestroyed(pShip, lShipPtr->mPos, lShipPtr->mInfo->IsPlanet);
      return True;
    }
    else {
      lShipPtr->mDamage += lDamage;
      DrawShipDamage(pShip, lShipPtr->mDamage);
    }
  }

  /* Now kill us some crew */
  if (lShipPtr->mInfo->IsPlanet)
    return False;

  lCrewKill = CrewKilled(pKill, lShipPtr->mInfo->Mass, gCombatConfig[pShip]);
  lCrewKill = lCrewKill * lDamagePercent / 100;

  if (lShipPtr->mCrew - lCrewKill < 0.5) {
    lShipPtr->mCrew = 0;
    lShipPtr->mInfo->Outcome = COMBAT_CAPTURED;
    DrawShipCaptured(pShip);
    return True;
  }
  else {
    lShipPtr->mCrew -= lCrewKill;
    DrawShipCrew(pShip, lShipPtr->mCrew);
  }

  return False;
}

/* This routine destroys fighters with other fighters. For a fighter to
   destroy another fighter, it must be at the same pixel location on the
   screen (so that the laser beam from one fighter to the other is perfectly
   vertical). This is pretty arbitrary but, hey, what isn't? */

static void
doFighterFighterHits(void)
{
  Uns16 lFighter,
    lFighter2;
  Int16 lPos;
  Int16 lQuantPos1[MAX_FIGHTER_LIMIT],
    lQuantPos2[MAX_FIGHTER_LIMIT];
  Uns16 lMaxFighters = max(MAX_FIGHTER_OUT(0), MAX_FIGHTER_OUT(1));
  Uns16 lHitProbability;
  Uns16 l2Hit1Probability;

  Uns16 lP1 = FIGHTER_HIT_FIGHTER_PROBABILITY(0);
  Uns16 lP2 = FIGHTER_HIT_FIGHTER_PROBABILITY(1);

  /* We do not allow the case of both fighters firing at each other. This
     makes 1-hit-2 or 2-hit-1 determination easier and simplifies the code. */
  lHitProbability = (lP1 * (100 - lP2) + lP2 * (100 - lP1)) / 100;
  if (lHitProbability NEQ 0) {
    /* Enforce fairness in the face of roundoff error */
    if (lP1 EQ lP2) {
      l2Hit1Probability = 50;
    }
    else {
      /* Bayes' rule */
      l2Hit1Probability = (100 - lP1) * lP2 / lHitProbability;
    }
  }
  else
    return;                     /* means that lP1==lP2==0 or 100...nuthin' to 
                                   do in this routine */

  /* Both ships must have fighters out for this to happen */
  if ((gStatus[0]->mFLaunched EQ 0)
        OR(gStatus[1]->mFLaunched EQ 0)
        )
    return;

  /* Perform position quantization ahead of time to save mucho time later.
     Each element of lQuantPos indicates the quantized position of the
     corresponding fighter. If the fighter is idle, the position is given as
     -32767. */

  passert(METERS_PER_PIXEL EQ 128);

  for (lFighter = 0; lFighter < lMaxFighters; lFighter++) {
    if (gStatus[0]->mFStatus[lFighter] EQ F_IDLE) {
      lQuantPos1[lFighter] = -32767;
    }
    else {
      /* Shift right by 7 bits is really division by METERS_PER_PIXEL */
      lQuantPos1[lFighter] = (Int16) (gStatus[0]->mFPos[lFighter] >> 7);
    }
    if (gStatus[1]->mFStatus[lFighter] EQ F_IDLE) {
      lQuantPos2[lFighter] = -32767;
    }
    else {
      /* Shift right by 7 bits is really division by METERS_PER_PIXEL */
      lQuantPos2[lFighter] = (Int16) (gStatus[1]->mFPos[lFighter] >> 7);
    }
  }

  /* For each fighter belonging to ship 0, find a fighter from ship 1 that
     occupies the same point in space. If such fighters are found, then a
     coin is flipped to decide which fighter is destroyed. */

  for (lFighter = 0; lFighter < lMaxFighters; lFighter++) {
    if (lQuantPos1[lFighter] EQ - 32767)
      continue;

    lPos = lQuantPos1[lFighter];

    /* With random probability, a fighter-to-fighter hit does not succeed so
       there is no point in searching for an enemy fighter at the same
       position. In PHOST 3 we must take into account differing
       probabilities. We computed hit probability above according to model
       that a hit occurs when A fires OR B fires but NOT BOTH. */
    if (RandomRange(100) >= lHitProbability)
      continue;

    /* We've got our fighter. Search for an enemy fighter at the same
       position on the screen (i.e., pixel values are the same) */
    for (lFighter2 = 0; lFighter2 < lMaxFighters; lFighter2++) {
      if (lPos EQ lQuantPos2[lFighter2]) {
        /* Do the kill. Decide who loses. With asymmetrical probabilities, we 
           must weight one side more (or less) than the other, as necessary.
           This was computed above as l2Hit1Probability. */
        Uns16 lLoser = (RandomRange(100) >= l2Hit1Probability) ? 1 : 0;

        Uns16 lLoserFighter = (lLoser EQ 1) ? lFighter2 : lFighter;
        BattleStatus_Struct *lLoserPtr = gStatus[lLoser];

        DrawFighterHitFighter(gStatus[0]->mFPos[lFighter], lFighter,
              lFighter2);
        if (lLoserPtr->mFStatus[lLoserFighter] EQ F_ATTACKING) {
          DrawEraseFighterAttack(lLoser, lLoserFighter,
                lLoserPtr->mFPos[lLoserFighter]);
        }
        else {
          DrawEraseFighterReturn(lLoser, lLoserFighter,
                lLoserPtr->mFPos[lLoserFighter]);
        }
        lLoserPtr->mFStatus[lLoserFighter] = F_IDLE;
        lLoserPtr->mFLaunched--;

        /* If it's the second fighter that lost, set its quantized position
           to -32767. We don't have to worry if the first fighter lost since
           we're not looking at it again. */
        if (lLoser) {
          lQuantPos2[lFighter2] = -32767;
        }
        break;
      }
    }
  }
}

/* This routine uses fighters to do damage to the enemy ship/planet. Each
   fighter is checked for minimum distance to the enemy and whether or not
   it has strikes left. If it meets the criteria, it fires on the enemy */

static Boolean
doFighterDamage(void)
{
  Uns16 lCount;
  Uns16 lFighter;
  Uns16 lRemaining;

  for (lCount = 0; lCount < 2; lCount++) {
    BattleStatus_Struct *lCurrPtr = gStatus[lCount];
    BattleStatus_Struct *lOtherPtr = gStatus[1 - lCount];

    /* Quick check to save a long search */
    if ((lRemaining = lCurrPtr->mFLaunched) EQ 0)
      continue;

    for (lFighter = 0; lRemaining > 0 AND lFighter < MAX_FIGHTER_OUT(lCount);
          lFighter++) {
      switch (lCurrPtr->mFStatus[lFighter]) {
      case F_IDLE:
        break;
      case F_RETURNING:
        lRemaining--;
        break;
      case F_ATTACKING:
        lRemaining--;

        /* Check distance to target */
        if (labs(lOtherPtr->mPos - lCurrPtr->mFPos[lFighter])
              <= MIN_FIGHTER_SHIP_HIT_DISTANCE(lCount)) {
          DrawFighterHit(lCount, lFighter, lCurrPtr->mFPos[lFighter],
                lOtherPtr->mPos);

          /* If this fighter has no more hits, return to the ship */
          if (--lCurrPtr->mFStrike[lFighter] EQ 0) {
            lCurrPtr->mFStatus[lFighter] = F_RETURNING;
            DrawEraseFighterAttack(lCount, lFighter,
                  lCurrPtr->mFPos[lFighter]);
            DrawFighterReturn(lCount, lFighter, lCurrPtr->mFPos[lFighter]);
          }

          if (doDamageShip(1 - lCount, FIGHTER_KILL_POWER(lCount),
                      FIGHTER_DAMAGE_POWER(lCount))) {
            return True;
          }
        }
        break;
      }
    }
  }
  return False;
}

/* This routine launches torps against the enemy. Torps travel instantaneously
   and do damage immediately */

static Boolean
doTorpLaunch(void)
{
  Uns16 lCount;
  Uns32 lDistance = 0;
  Uns16 lTorpPower = (gConfigInfo->AlternativeCombat) ? 1 : 2;
  Uns16 lTubeLimit;

  for (lCount = 0; lCount < 2; lCount++) {
    BattleStatus_Struct *lCurrPtr = gStatus[lCount];
    BattleStatus_Struct *lOtherPtr = gStatus[1 - lCount];
    Uns16 lTube;

    if (lCount EQ 0) {
      if ((lCurrPtr->mInfo->NumTorps EQ 0)
            AND(lOtherPtr->mInfo->NumTorps EQ 0)
            )
        return False;
      lDistance = lOtherPtr->mPos - lCurrPtr->mPos;
      if (lDistance > MIN_TORP_SHIP_HIT_DISTANCE(0))
        continue;
    }
    else {
      if (lCurrPtr->mInfo->IsPlanet) {
        if (lDistance > MIN_TORP_PLANET_HIT_DISTANCE(1))
          return False;
      }
      else if (lDistance > MIN_TORP_SHIP_HIT_DISTANCE(1))
        return False;
    }

    if (lCurrPtr->mInfo->NumTorps EQ 0)
      continue;

    lTubeLimit = lCurrPtr->mInfo->NumTubes;
    for (lTube = 0; lTube < lTubeLimit; lTube++) {
      if (lCurrPtr->mTubeCharge[lTube] >= MIN_TUBE_CHARGE) {
        /* Fire a torpedo */
        lCurrPtr->mInfo->NumTorps--;
        lCurrPtr->mTubeCharge[lTube] = 0;
        DrawTubeRecharge(lCount, lTube, 0);
        DrawTorpNumber(lCount, lCurrPtr->mInfo->NumTorps);

        if (RandomRange(100) <= lCurrPtr->mTorpHitOdds) {
          DrawTorpHit(lCount, lCurrPtr->mPos, lOtherPtr->mPos);

          if (doDamageShip(1 - lCount,
                      lTorpPower * TorpKillPower(lCurrPtr->mInfo->TorpTech),
                      lTorpPower *
                      TorpDestructivePower(lCurrPtr->mInfo->TorpTech))
                ) {
            return True;
          }
        }
        else {
          DrawTorpMiss(lCount, lCurrPtr->mPos, lOtherPtr->mPos);
        }
        break;
      }
    }
  }
  return False;
}

/* This routine fires a beam weapon from one ship directly to the other
   ship or to oncoming fighters. This routine returns True if the enemy
   ship is destroyed or captured. */

static Boolean
doBeamFire(void)
{
  Uns16 lCount;
  Int32 lShipDistance = 0;
  Uns16 lBeamLimit;

  for (lCount = 0; lCount < 2; lCount++) {
    BattleStatus_Struct *lCurrPtr = gStatus[lCount];
    BattleStatus_Struct *lOtherPtr = gStatus[1 - lCount];
    Uns16 lBeam;

    if (lCount EQ 0) {
      lShipDistance = lOtherPtr->mPos - lCurrPtr->mPos;
    }

    lBeamLimit = lCurrPtr->mInfo->NumBeams;
    for (lBeam = 0; lBeam < lBeamLimit; lBeam++) {
      Uns16 lFighter;

      /* If beam meets minimum charge level for fighters, try to fire on a
         fighter. Ships must be close enough to fighters for this to happen.
         Find closest other fighter to see if this is the case. */

      if (lCurrPtr->mBeamCharge[lBeam]
            >= MIN_BEAM_CHARGE_FOR_FIGHTERS(lCount)) {
        /* Beams can miss */
        Uns16 lMissed = RandomRange(100) >= lCurrPtr->mBeamHitOdds;

        /* First, look for attacking fighters. In fact, look for the CLOSEST
           attacking fighter. */
        Int16 lClosest = -1;    /* closest attacking fighter */
        Int32 lClosestPos = 100000UL; /* closest att. fighter dist. */

        for (lFighter = 0; lFighter < MAX_FIGHTER_OUT(1 - lCount); lFighter++) {
          /* In any case, we fire on the closest fighter. The only decision
             is whether we fire on the closest attacking fighter, or whether
             it's just the closest fighter, period. */

          if (gConfigInfo->FireOnAttackFighters) {
            /* First look only for attackers */
            if (lOtherPtr->mFStatus[lFighter] NEQ F_ATTACKING) {
              continue;
            }
          }
          else {
            /* Look for any fighter */
            if (lOtherPtr->mFStatus[lFighter] EQ F_IDLE) {
              continue;
            }
          }
          if (labs(lOtherPtr->mFPos[lFighter] - lCurrPtr->mPos) < lClosestPos) {
            lClosestPos = labs(lOtherPtr->mFPos[lFighter] - lCurrPtr->mPos);
            lClosest = lFighter;
          }
        }
        if (lClosest >= 0) {
          /* If closest fighter is out of range, don't fire */

          if (lClosestPos > MIN_BEAM_FIGHTER_HIT_DISTANCE(lCount)) {
            /* Just wait until the fighter reaches us */
            continue;
          }

          if (lMissed) {
            DrawBeamMissedFighter(lCount, lBeam, lCurrPtr->mPos, lClosest,
                  lOtherPtr->mFPos[lClosest]);
          }
          else {
            DrawBeamHitFighter(lCount, lBeam, lCurrPtr->mPos, lClosest,
                  lOtherPtr->mFPos[lClosest]);
            if (lOtherPtr->mFStatus[lClosest] EQ F_ATTACKING) {
              DrawEraseFighterAttack(1 - lCount, lClosest,
                    lOtherPtr->mFPos[lClosest]);
            }
            else {
              DrawEraseFighterReturn(1 - lCount, lClosest,
                    lOtherPtr->mFPos[lClosest]);
            }
          }

          /* Beam fires at the fighter */
          lCurrPtr->mBeamCharge[lBeam] = 0;
          DrawBeamRecharge(lCount, lBeam, 0);

          if (!lMissed) {       /* NEW */
            lOtherPtr->mFStatus[lClosest] = F_IDLE;
            lOtherPtr->mFLaunched--;
          }

          goto next_ship;       /* NEW */
        }

        /* Didn't find any attacking fighters. What about returning fighters */
        if (gConfigInfo->FireOnAttackFighters) {
          for (lFighter = 0; lFighter < MAX_FIGHTER_OUT(1 - lCount);
                lFighter++) {
            if (lOtherPtr->mFStatus[lFighter] EQ F_RETURNING) {
              if (lMissed) {    /* NEW */
                DrawBeamMissedFighter(lCount, lBeam, lCurrPtr->mPos, lFighter,
                      lOtherPtr->mFPos[lFighter]);
              }
              else {
                DrawBeamHitFighter(lCount, lBeam, lCurrPtr->mPos, lFighter,
                      lOtherPtr->mFPos[lFighter]);
                DrawEraseFighterReturn(1 - lCount, lFighter,
                      lOtherPtr->mFPos[lFighter]);
              }

              /* Beam fires at the fighter */
              lCurrPtr->mBeamCharge[lBeam] = 0;
              DrawBeamRecharge(lCount, lBeam, 0);
              if (!lMissed) {   /* NEW */
                lOtherPtr->mFStatus[lFighter] = F_IDLE;
                lOtherPtr->mFLaunched--;
              }
              goto next_ship;   /* NEW */
            }
          }
        }
      }

      /* Didn't fire on a fighter. Let's try firing on the ship. However, if
         there are enemy fighters launched, do not waste beam weapons firing
         on a ship. Save them up until they reach the charge level necessary
         to fire on fighters. */
      if (lOtherPtr->mFLaunched > 0)
        continue;

      if ((lCurrPtr->mBeamCharge[lBeam] >= MIN_BEAM_CHARGE_FOR_SHIPS(lCount))
            AND(lShipDistance <= MIN_BEAM_SHIP_HIT_DISTANCE(lCount))
            ) {
        /* Beams can miss */
        Uns16 lMissed = RandomRange(100) >= lCurrPtr->mBeamHitOdds;
        Uns16 lKill,
          lBoom;

        /* Pro-rate destructive power by beam charge level */
        lKill = BeamKillPower(lCurrPtr->mInfo->BeamTech)
              * (lCurrPtr->mBeamCharge[lBeam] / 10) / 100;
        lBoom = BeamDestructivePower(lCurrPtr->mInfo->BeamTech)
              * (lCurrPtr->mBeamCharge[lBeam] / 10) / 100;

        /* Give Privateers 3X Kill bonus */
        /* NEW */
        if (EffRace(lCurrPtr->mInfo->Race) EQ Privateers) {
          lKill *= 3;
        }

        if (lMissed) {          /* NEW */
          DrawBeamMissedShip(lCount, lCurrPtr->mPos, lOtherPtr->mPos);
        }
        else {
          DrawBeamHitShip(lCount, lCurrPtr->mPos, lOtherPtr->mPos);
        }

        lCurrPtr->mBeamCharge[lBeam] = 0;
        DrawBeamRecharge(lCount, lBeam, 0);

        /* doDamageShip() returns True if other ship destroyed/captured */
        if (!lMissed AND doDamageShip(1 - lCount, lKill, lBoom)) {
          return True;
        }
        goto next_ship;         /* NEW */
      }
    }
  next_ship:
    ;
  }
  return False;
}

/* This routine moves each ship towards the point X=0 (except for planets)
   at the ship movement rate */

static void
doMoveShips(void)
{
  Uns16 lCount;
  Uns32 lDistance;

  for (lCount = 0; lCount < 2; lCount++) {
    BattleStatus_Struct *lCurrPtr;
    BattleStatus_Struct *lOtherPtr;

    lCurrPtr = gStatus[lCount];
    lOtherPtr = gStatus[1 - lCount];

    if (lCount EQ 0) {
      lDistance = labs(lOtherPtr->mPos - lCurrPtr->mPos);
      if (lDistance <= STANDOFF_DISTANCE)
        return;
    }

    if (!lCurrPtr->mInfo->IsPlanet) {
      DrawEraseShip(lCount, lCurrPtr->mPos);
      lCurrPtr->mPos += (Int32) lCurrPtr->mDir * SHIP_MOVEMENT_SPEED(lCount);

      lDistance -= SHIP_MOVEMENT_SPEED(lCount);
      if (lDistance < STANDOFF_DISTANCE) {
        Uns32 lDiff = STANDOFF_DISTANCE - lDistance;

        lCurrPtr->mPos -= (Int32) lCurrPtr->mDir * lDiff;
        DrawShip(lCount, lCurrPtr->mPos);
        return;
      }

      DrawShip(lCount, lCurrPtr->mPos);
    }
  }
}

/* This routine checks each ship to see if it has the capability to inflict
   damage on the enemy (i.e., it either has beams, or it has torps/fighters
   left). If not, the routine sets the combat outcome of this ship to
   COMBAT_NOAMMO to indicate that it is not a threat */

static Boolean
checkShipHasAmmo(Uns16 pShip)
{
  Combat_Struct *lInfo = gStatus[pShip]->mInfo;

  if ((lInfo->NumBeams > 0)
        OR(lInfo->NumFighters > 0)
        OR(lInfo->NumTorps > 0)
        OR(gStatus[pShip]->mFLaunched > 0)
        ) {
    return True;
  }
  else {
    lInfo->Outcome = COMBAT_NOAMMO;
    return False;
  }
}

/* This routine performs all necessary actions during one battle tick.
   Ships are moved, fighters are moved, beams are fired, torps are launched,
   etc. The return value is TRUE if battle is complete i.e., one or both
   ships have been destroyed or captured, or both ships have no more ammo. */

static Boolean
DoBattleTick(BattleStatus_Struct * pS1, BattleStatus_Struct * pS2)
{
  gStatus[0] = pS1;
  gStatus[1] = pS2;

  /* Non-violent stuff */
  doBayRecharge();
  doBeamRecharge();
  doTubeRecharge();
  doFighterLaunch();

  /* Violent stuff. These routines return True if one of the ships has been
     destroyed or captured. */

  if (doFighterDamage())
    return True;
  if (doTorpLaunch())
    return True;
  if (doBeamFire())
    return True;
  doFighterFighterHits();

  /* More non-violent stuff */
  doMoveFighters();
  doMoveShips();

  DrawDistance(gStatus[1]->mPos - gStatus[0]->mPos);
  return (!checkShipHasAmmo(0)) AND(!checkShipHasAmmo(1));
}

/* This routine runs an entire battle. It calls DoBattleTick
   repeatedly until the battle is complete. For PHOST, that's
   all there is. For PVCR, this routine resides in PVCRDRAW.C
   and is responsible for handling keystrokes etc. The routine
   returns True if the user wants to immediately quit by
   pressing Esc. Obviously, for the PHOST version, this will
   never happen. */

static Boolean
DisplayBattle(BattleStatus_Struct * pS1, BattleStatus_Struct * pS2)
{
  Boolean lBattleDone;

  gBattleTick = 0;

  do {
    gBattleTick++;
    lBattleDone = DoBattleTick(pS1, pS2);
  } while (!lBattleDone);

  return False;
}

static void
getPlayerCombatConfig(Uns16 pPlayer, CombatConfig_Struct * pConfig)
{
  /* If player is out of range then this is either a Jumpgate alien or an
     unassigned race(?). In either case, make it player 1 by default. Why
     not. Also, do the same for version 2.x or 1.x VCR's. */
  if ((pPlayer < 1)
        OR(pPlayer > OLD_RACE_NR)
        ) {
    pPlayer = 1;
  }
  passert(pConfig NEQ 0);
  passert(gConfigInfo NEQ 0);

  pConfig->BayRechargeRate = gConfigInfo->BayRechargeRate[pPlayer];
  pConfig->BayRechargeBonus = gConfigInfo->BayRechargeBonus[pPlayer];
  pConfig->BeamRechargeRate = gConfigInfo->BeamRechargeRate[pPlayer];
  pConfig->BeamRechargeBonus = gConfigInfo->BeamRechargeBonus[pPlayer];
  pConfig->TubeRechargeRate = gConfigInfo->TubeRechargeRate[pPlayer];
  pConfig->BeamHitFighterCharge = gConfigInfo->BeamHitFighterCharge[pPlayer];
  pConfig->BeamHitShipCharge = gConfigInfo->BeamHitShipCharge[pPlayer];
  pConfig->TorpFiringRange = gConfigInfo->TorpFiringRange[pPlayer];
  pConfig->BeamFiringRange = gConfigInfo->BeamFiringRange[pPlayer];
  pConfig->TorpHitOdds = gConfigInfo->TorpHitOdds[pPlayer];
  pConfig->BeamHitOdds = gConfigInfo->BeamHitOdds[pPlayer];
  pConfig->BeamHitBonus = gConfigInfo->BeamHitBonus[pPlayer];
  pConfig->StrikesPerFighter = gConfigInfo->StrikesPerFighter[pPlayer];
  pConfig->FighterKillOdds = gConfigInfo->FighterKillOdds[pPlayer];
  pConfig->FighterBeamExplosive = gConfigInfo->FighterBeamExplosive[pPlayer];
  pConfig->FighterBeamKill = gConfigInfo->FighterBeamKill[pPlayer];
  pConfig->ShipMovementSpeed = gConfigInfo->ShipMovementSpeed[pPlayer];
  pConfig->FighterMovementSpeed = gConfigInfo->FighterMovementSpeed[pPlayer];
  pConfig->BayLaunchInterval = gConfigInfo->BayLaunchInterval[pPlayer];
  pConfig->MaxFightersLaunched = gConfigInfo->MaxFightersLaunched[pPlayer];
  pConfig->TorpHitBonus = gConfigInfo->TorpHitBonus[pPlayer];
  pConfig->TubeRechargeBonus = gConfigInfo->TubeRechargeBonus[pPlayer];
  pConfig->ShieldDamageScaling = gConfigInfo->ShieldDamageScaling[pPlayer];
  pConfig->HullDamageScaling = gConfigInfo->HullDamageScaling[pPlayer];
  pConfig->CrewKillScaling = gConfigInfo->CrewKillScaling[pPlayer];
  pConfig->ExtraFighterBays = gConfigInfo->ExtraFighterBays[pPlayer];
  pConfig->BeamHitFighterRange = gConfigInfo->BeamHitFighterRange[pPlayer];
  pConfig->FighterFiringRange = gConfigInfo->FighterFiringRange[pPlayer];
}

/* This is the main combat driver routine. It sets up the graphics
   screen and initializes the dynamic combat structures. Then, it
   enters a loop whereby all combat functions are performed in
   time-slice fashion until one ship is either destroyed or captured
   or both ships run out of ammunition (i.e., there is no hope for
   progress) */

void
Combat(Combat_Struct * pShip1, Combat_Struct * pShip2)
{
  Uns16 lCount;
  char lName[64];
  Boolean lTerminate;           /* True if user wants to immediately quit */

  /* Allocate our status structures if necessary */
  if (gInitialStatus[0] EQ 0) {
    for (lCount = 0; lCount < 2; lCount++)
      gInitialStatus[lCount] =
            (BattleStatus_Struct *) MemAlloc(sizeof(BattleStatus_Struct));
  }

  /* Allocate combat config structures if necessary */
  if (gCombatConfig[0] EQ 0) {
    for (lCount = 0; lCount < 2; lCount++) {
      gCombatConfig[lCount] =
            (CombatConfig_Struct *) MemAlloc(sizeof(CombatConfig_Struct));
    }
  }

  /* Fill combat config structures with player-dependent configs */
  getPlayerCombatConfig(pShip1->Race, gCombatConfig[0]);
  getPlayerCombatConfig(pShip2->Race, gCombatConfig[1]);

  passert(MAX_FIGHTER_OUT(0) <= MAX_FIGHTER_LIMIT);
  passert(MAX_FIGHTER_OUT(1) <= MAX_FIGHTER_LIMIT);

  /* First, initialize the dynamic status structures for both ships */
  for (lCount = 0; lCount < 2; lCount++) {
    BattleStatus_Struct *lCurrPtr = gInitialStatus[lCount];

    /* Everything is 0 by default */
    memset(lCurrPtr, 0, sizeof(BattleStatus_Struct));

    lCurrPtr->mInfo = lCount EQ 0 ? pShip1 : pShip2;

    lCurrPtr->mPos = STARTING_POSITION[lCount];
    lCurrPtr->mDir = (lCount EQ 0) ? +1 : -1;
    /* NEW */
    lCurrPtr->mShields = lCurrPtr->mInfo->Shields; /* Fractional support */
    lCurrPtr->mDamage = lCurrPtr->mInfo->Damage; /* Fractional support */
    lCurrPtr->mCrew = lCurrPtr->mInfo->Crew; /* Fractional support */

    DrawShipShields(lCount, STARTING_POSITION[lCount],
          lCurrPtr->mInfo->Shields);
    DrawShipDamage(lCount, lCurrPtr->mInfo->Damage);

    if (!lCurrPtr->mInfo->IsPlanet)
      DrawShipCrew(lCount, lCurrPtr->mInfo->Crew);

    if (lCurrPtr->mInfo->NumBeams > 0) {
      /* Compute the beam hit probability and beam recharge rate */
      lCurrPtr->mBeamHitOdds =
            BeamHitOdds(lCurrPtr->mInfo->BeamTech, gCombatConfig[lCount]);
      lCurrPtr->mBeamChargeRate =
            BeamRechargeRate(lCurrPtr->mInfo->BeamTech,
            gCombatConfig[lCount]);
    }

    if (lCurrPtr->mInfo->NumTubes > 0) {
      /* Compute the torp recharge rate and hit probability */
      lCurrPtr->mTorpHitOdds =
            TorpHitOdds(lCurrPtr->mInfo->TorpTech, gCombatConfig[lCount]);
      lCurrPtr->mTubeChargeRate =
            TubeRechargeRate(lCurrPtr->mInfo->TorpTech,
            gCombatConfig[lCount]);

      DrawTorpNumber(lCount, lCurrPtr->mInfo->NumTorps);
    }

    if (lCurrPtr->mInfo->NumBays > 0) {
      /* Compute the bay recharge rate */
      lCurrPtr->mBayChargeRate =
            BayRechargeRate(lCurrPtr->mInfo->NumBays, gCombatConfig[lCount]);

      DrawFighterNumber(lCount, lCurrPtr->mInfo->NumFighters);
    }

    /* If shields aren't 100%, then beams are initially discharged */
    {
      Uns16 lBeam;

      for (lBeam = 0; lBeam < lCurrPtr->mInfo->NumBeams; lBeam++)
        if (lCurrPtr->mInfo->Shields EQ 100)
          lCurrPtr->mBeamCharge[lBeam] = 1000;
    }

    /* Charge tubes */
    /* For some reason, this code is compiled incorrectly by the Borland C++
       3.1 and 4.5 compilers with the -Ol optimization option. DON'T USE IT! */
    {
      Uns16 lTube;

      for (lTube = 0; lTube < lCurrPtr->mInfo->NumTubes; lTube++)
        if (lCurrPtr->mInfo->Shields EQ 100)
          lCurrPtr->mTubeCharge[lTube] = 1000;
    }

    /* Charge fighter bays */
    {
      Uns16 lBay;

      for (lBay = 0; lBay < lCurrPtr->mInfo->NumBays; lBay++) {
        DrawBayRecharge(lCount, lBay, 100);
        lCurrPtr->mBayCharge[lBay] = 0; /* !!!!!!!!!!!!! */
      }
    }
    DrawInitBeams(lCount, lCurrPtr->mInfo->NumBeams,
          lCurrPtr->mBeamCharge[0]);
    DrawInitTubes(lCount, lCurrPtr->mInfo->NumTubes,
          lCurrPtr->mTubeCharge[0]);
  }

  /* Start the festivities */
  pShip1->Outcome = COMBAT_VICTOR;
  pShip2->Outcome = COMBAT_VICTOR;

  DrawShip(0, gInitialStatus[0]->mPos);
  DrawShip(1, gInitialStatus[1]->mPos);

  lTerminate = DisplayBattle(gInitialStatus[0], gInitialStatus[1]);

  /* Festivities are over, now we have to sort out the results. */
  for (lCount = 0; lCount < 2; lCount++) {
    BattleStatus_Struct *lCurrPtr = gInitialStatus[lCount];
    Uns16 lFighter;

    /* Erase all the fighters */
    for (lFighter = 0; lFighter < MAX_FIGHTER_OUT(lCount); lFighter++) {
      switch (lCurrPtr->mFStatus[lFighter]) {
      case F_IDLE:
        break;
      case F_ATTACKING:
        DrawEraseFighterAttack(lCount, lFighter, lCurrPtr->mFPos[lFighter]);
        break;
      case F_RETURNING:
        DrawEraseFighterReturn(lCount, lFighter, lCurrPtr->mFPos[lFighter]);
        break;
      }
    }

    /* Round so that on-screen display matches visible results */
    if (lCurrPtr->mDamage > 0)
      lCurrPtr->mDamage += 0.5;
    if (lCurrPtr->mShields > 0)
      lCurrPtr->mShields += 0.5;
    if (lCurrPtr->mCrew > 0)
      lCurrPtr->mCrew += 0.5;

    DrawShipDamage(lCount, lCurrPtr->mDamage);
    if (!lCurrPtr->mInfo->IsPlanet) {
      DrawShipCrew(lCount, lCurrPtr->mCrew);
    }

    if (lCurrPtr->mDamage >= 100) {
      lCurrPtr->mInfo->Outcome = COMBAT_DESTROYED;
      DrawShipDestroyed(lCount, lCurrPtr->mPos, lCurrPtr->mInfo->IsPlanet);
    }
    else {
      /* If the ship was captured, leave the status as captured since this is 
         more important than whether or not it has ammo. Otherwise, if this
         ship thinks it's the victor, downgrade its result to NOAMMO if
         warranted */

      if (lCurrPtr->mInfo->Outcome EQ COMBAT_VICTOR) {
        (void) checkShipHasAmmo(lCount); /* sets NOAMMO result if appropriate 
                                          */
      }

      /* Reincorporate fighters back into the ship */
      if (lCurrPtr->mInfo->NumBays > 0) {
        lCurrPtr->mInfo->NumFighters += lCurrPtr->mFLaunched;
        DrawFighterNumber(lCount, lCurrPtr->mInfo->NumFighters);
      }
    }

    /* Convert fractional shields/damage/crew back to integers */
    /* NEW */
    lCurrPtr->mInfo->Shields = (Uns16) (lCurrPtr->mShields);
    lCurrPtr->mInfo->Damage = min((Uns16) (lCurrPtr->mDamage), 100);
    lCurrPtr->mInfo->Crew = (Uns16) (lCurrPtr->mCrew);
  }

  /* If both ships ran out of ammo, display a message */
  if ((gInitialStatus[0]->mInfo->Outcome EQ COMBAT_NOAMMO)
        AND(gInitialStatus[1]->mInfo->Outcome EQ COMBAT_NOAMMO)
        ) {
    DrawStalemateResult(gInitialStatus[1]->mInfo->IsPlanet);
  }
}

#endif /* MICROSQUISH */

/*************************************************************
  $HISTORY:$
**************************************************************/

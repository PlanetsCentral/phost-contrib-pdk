
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

#include "phostpdk.h"
#include "private.h"
#include "assert.h"

static Boolean gReadAuxData = False;
static Boolean gNoAuxData = False;
static Boolean gAuxdataLoaded = False;
static Boolean gCloakcLoaded = False;

static Uns16 *gScanInfo = 0;
static Uns16 *gCombatShields = 0;

static Uns8 gVersionMajor = 0;
static Uns8 gVersionMinor = 0;

static Uns8 gAuxilliaryInfo[16];

static void InitScanInfo(void);
static void InitBaseNativeInfo(void);
static void InitAlliances(void);
static void InitBuildQueue(void);
static void InitActivityLevel(void);
static void InitRemoteControl(void);
static void InitCLOAKC(void);
static void FreeScanInfo(void);
static void FreeBaseNativeInfo(void);
static void FreeBuildQueue(void);
static void FreeRemoteControl(void);
static void FreeCLOAKC(void);

static Boolean gInitialized = False;

static void
FreeAuxData(void)
{
  FreeScanInfo();
  FreeBaseNativeInfo();
  FreeBuildQueue();
  FreeRemoteControl();
  FreeCLOAKC();

  gInitialized = False;
}

static void
InitAuxData(void)
{
  if (gInitialized)
    return;

  InitScanInfo();
  InitBaseNativeInfo();
  InitAlliances();
  InitBuildQueue();
  InitActivityLevel();
  InitRemoteControl();
  InitCLOAKC();

  RegisterCleanupFunction(FreeAuxData);

  gInitialized = True;
}

/*
 *     S C A N    I N F O
 */

static void
InitScanInfo(void)
{
  gScanInfo = (Uns16 *) MemCalloc(SHIP_NR + 1, sizeof(Uns16));
}

static void
FreeScanInfo(void)
{
  MemFree(gScanInfo);
  gScanInfo = 0;
}

/* This routine is called to save the ship scan information to the auxilliary
   data file. */

static Boolean
WriteScanInfo(FILE * pOutfile)
{
  InitAuxData();

  EndianSwap16(gScanInfo, SHIP_NR + 1);
  if (1 NEQ fwrite(gScanInfo, (SHIP_NR + 1) * sizeof(Uns16), 1, pOutfile)) {
    return False;
  }
  EndianSwap16(gScanInfo, SHIP_NR + 1);
  return True;
}

/* This routine reads the previous turn's ship information from the
   auxilliary data file. Currently, we do this only to determine if a ship
   was cloaked on the previous turn. */

static Boolean
ReadScanInfo(FILE * pInfile)
{
  InitAuxData();

  passert(gScanInfo NEQ 0);
  if (1 NEQ fread(gScanInfo, (SHIP_NR + 1) * sizeof(Uns16), 1, pInfile)) {
    return False;
  }
  EndianSwap16(gScanInfo, SHIP_NR + 1);
  return True;
}

static void
InitCLOAKC(void)
{
  gCombatShields = (Uns16 *) MemCalloc(SHIP_NR + 1, sizeof(Uns16));
}

static void
FreeCLOAKC(void)
{
  MemFree(gCombatShields);
  gCombatShields = 0;
}

/* This function gets at bit 15 of the ship scan info, which PHOST v2.11
   and higher uses to set the cloak state. Now, if we don't have an
   AUXDATA file loaded, or if it is v2.10 or lower, just do a check
   on the mission of the ship. Otherwise, use the MSB of the scan info. */

Boolean
IsShipCloaked(Uns16 pShip)
{
  Uns16 lMajor,
    lMinor;
  Boolean lIsAuxdataPresent = GameFilesVersion(&lMajor, &lMinor);

  /* Preconditions for valid cloak info: (a) No auxdata (HOST) and CLOAKC.HST 
     was loaded OR (b) auxdata present (PHOST) and version > 2.10 */

  if (((!lIsAuxdataPresent) AND(!gCloakcLoaded))
        OR(lMajor < 2)
        OR((lMajor <= 2) AND(lMinor < 11))
        ) {
    /* Just use the mission. */
    return (ShipCanCloak(pShip)
          AND((ShipMission(pShip) EQ Cloak)
                OR((EffRace(ShipOwner(pShip)) EQ Birdmen)
                      AND(ShipMission(pShip) EQ SpecialMission)
                )
          )
          );
  }
  else {
    passert(gScanInfo NEQ 0);

    return (gScanInfo[pShip] & 0x8000U) ? True : False;
  }
}

#if 0
void
MarkShipCloaking(Uns16 pShip, Boolean pIsCloaking)
{
  passert((pShip >= 1) AND(pShip <= SHIP_NR));

  if (pIsCloaking)
    gScanInfo[pShip] |= 0x8000U;
  else
    gScanInfo[pShip] &= 0x0FFFU;

  gShipInfo[pShip].mIsCloaked = pIsCloaking;

  /* If we want the ship to be uncloaked for all phases, mark the cloak as
     having failed so we don't re-cloak in a later phase */
  if (pIsCloaking EQ False)
    gShipInfo[pShip].mCloakHasFailed = True;
}
#endif

/*
 *    S H I P    B U I L D Q U E U E
 */

static BaseOrder_Struct *gBaseOrder = 0;
static Uns16 gNumBaseOrders = 0;

/* This routine is responsible for returning the top of the ship build queue.
   The build order is passed back to the caller along with the ID of the
   base that owns the order. 0 is returned if the queue is empty. This
   routine does *NOT* remove the order from the queue */

Uns16
BuildQueueBaseID(Uns16 pPosition)
{
  BaseOrder_Struct *QueueMember;
  
  QueueMember=BuildQueueAccess(pPosition);

  return QueueMember->mBase;
}

Uns32
BuildQueuePriority(Uns16 pPosition)
{
  BaseOrder_Struct *QueueMember;
  
  QueueMember=BuildQueueAccess(pPosition);

  return QueueMember->mPriority;
}

Boolean
BuildQueueIsCloning(Uns16 pPosition)
{
  BaseOrder_Struct *QueueMember;
  
  QueueMember=BuildQueueAccess(pPosition);

  return QueueMember->mByCloning;
}

RaceType_Def
BuildQueueShipOwner(Uns16 pPosition)
{
  BaseOrder_Struct *QueueMember;
  
  QueueMember=BuildQueueAccess(pPosition);

  return QueueMember->mShipOwner;
}

RaceType_Def
BuildQueueOwner(Uns16 pPosition)
{
  BaseOrder_Struct *QueueMember;
  
  QueueMember=BuildQueueAccess(pPosition);

  return BaseOwner(QueueMember->mBase);
}

Uns16
BuildQueueHullType(Uns16 pPosition)
{
  BaseOrder_Struct *QueueMember;
  
  QueueMember=BuildQueueAccess(pPosition);

/* Clonning is a special case */

  return (QueueMember->mByCloning?
  (&QueueMember->mOrder)->mHull:
  TrueHull(BaseOwner(QueueMember->mBase),(&QueueMember->mOrder)->mHull));
}

Uns16
BuildQueueEngineType(Uns16 pPosition)
{
  BaseOrder_Struct *QueueMember;
  
  QueueMember=BuildQueueAccess(pPosition);

  return (&QueueMember->mOrder)->mEngineType;
}

Uns16
BuildQueueBeamType(Uns16 pPosition)
{
  BaseOrder_Struct *QueueMember;
  
  QueueMember=BuildQueueAccess(pPosition);

  return (&QueueMember->mOrder)->mBeamType;
}

Uns16
BuildQueueTorpType(Uns16 pPosition)
{
  BaseOrder_Struct *QueueMember;
  
  QueueMember=BuildQueueAccess(pPosition);

  return (&QueueMember->mOrder)->mTubeType;
}

Uns16
BuildQueueBeamNumber(Uns16 pPosition)
{
  BaseOrder_Struct *QueueMember;
  
  QueueMember=BuildQueueAccess(pPosition);

  return (&QueueMember->mOrder)->mNumBeams;
}

Uns16
BuildQueueTorpNumber(Uns16 pPosition)
{
  BaseOrder_Struct *QueueMember;
  
  QueueMember=BuildQueueAccess(pPosition);

  return (&QueueMember->mOrder)->mNumTubes;
}

void
PutBuildQueuePriority(Uns16 pPosition, Uns32 pPriority)
{
  BaseOrder_Struct *QueueMember;

  QueueMember=BuildQueueAccess(pPosition);

  QueueMember->mPriority = pPriority;
}


Uns16
BuildQueuePeek(BaseOrder_Struct * pOrder)
{
  InitAuxData();

  passert(pOrder NEQ 0);
  if (gNumBaseOrders EQ 0)
    return 0;

  passert(gBaseOrder[0].mBase NEQ 0);
  memcpy(pOrder, gBaseOrder, sizeof(BaseOrder_Struct));
  return gBaseOrder[0].mBase;
}

BaseOrder_Struct *
BuildQueueAccess(Uns16 pMember)
{
  InitAuxData();

  if (pMember >= gNumBaseOrders)
    return 0;

  return gBaseOrder + pMember;
}

static void
invalidateOrder(Uns16 pOrder)
{
  BaseOrder_Struct *pN = gBaseOrder + pOrder;

  passert(gNumBaseOrders > pOrder);

  if (--gNumBaseOrders > pOrder) {
    /* Must be 'memmove' -- it's an overlapping copy */
    memmove(pN, pN + 1, (gNumBaseOrders - pOrder) * sizeof(BaseOrder_Struct));
  }
  gBaseOrder[gNumBaseOrders].mBase = 0;
}

/* This routine adds a build order to the build order queue. There are three
   cases to consider:
 1. The base has no pending order already in the queue
   In this case, we simply add the given build order to the back
   of the queue

 2. The base has a pending order in the queue and it is the same
    order as the one we have been given.
   All this means is that another turn has elapsed and the ship
   still hasn't been built. Do nothing.

 3. The base has a pending order in the queue and it is different from
    the one we have been given.
   The base has cancelled its previous order and submitted a new
   one. We invalidate the old build order and add the new one to
   the back of the queue.
*/

void
BuildQueuePush(BaseOrder_Struct * pOrderPtr)
{
  Uns16 lCount;
  Uns16 lOwner;
  Uns32 lPriority,
    lDiff;
  BaseOrder_Struct *lCurrPtr;

  InitAuxData();

  passert((pOrderPtr NEQ 0) AND IsBaseExist(pOrderPtr->mBase));

  /* Try to find the given base in the queue */
  for (lCount = 0; lCount < gNumBaseOrders; lCount++) {
    if (gBaseOrder[lCount].mBase EQ pOrderPtr->mBase)
      break;
  }

  /* If not found, just add to the end */
  if (lCount >= gNumBaseOrders) {
    /* Add the current order to the back of the queue */
    passert(gNumBaseOrders < PLANET_NR);

    memcpy(gBaseOrder + gNumBaseOrders, pOrderPtr, sizeof(BaseOrder_Struct));
    gNumBaseOrders++;
    return;
  }

  /* Now, this base is already in the queue. Is the order the same or is it
     different? Can't just compare BaseOrder structures because priorities
     may well be different. Compare BuildOrder structures then remaining
     fields. */
  lCurrPtr = gBaseOrder + lCount;

  if ((memcmp(&lCurrPtr->mOrder, &pOrderPtr->mOrder,
                    sizeof(BuildOrder_Struct)) EQ 0)
        AND(lCurrPtr->mByCloning EQ pOrderPtr->mByCloning)
        AND(lCurrPtr->mShipOwner EQ pOrderPtr->mShipOwner)
        )
    return;                     /* Nothing to do -- they're the same */

  /* The orders are different. Subtract penalty priority from the order */
  lOwner = lCurrPtr->mShipOwner;
  lPriority = lCurrPtr->mPriority;
  lDiff = gConfigInfo->SBQBuildChangePenalty[lOwner];

  if (lDiff >= lPriority) {
    lPriority = 0;
  }
  else {
    lPriority -= lDiff;
  }

  /* Add the new order */
  memcpy(lCurrPtr, pOrderPtr, sizeof(BaseOrder_Struct));

  /* And now fix up the priority */
  lCurrPtr->mPriority = lPriority;
}

/* This routine removes a base's build order from the queue. If the base
   has no build order pending, this routine does nothing */

void
BuildQueueInvalidate(Uns16 pBase)
{
  Uns16 lCount;

  InitAuxData();

  /* Try to find the given base in the queue */
  for (lCount = 0; lCount < gNumBaseOrders; lCount++) {
    if (gBaseOrder[lCount].mBase EQ pBase)
      break;
  }
  if (lCount >= gNumBaseOrders)
    return;

  invalidateOrder(lCount);
}

static void
InitBuildQueue(void)
{
  gBaseOrder =
        (BaseOrder_Struct *) MemCalloc(PLANET_NR, sizeof(BaseOrder_Struct));
}

static void
FreeBuildQueue(void)
{
  MemFree(gBaseOrder);
  gBaseOrder = 0;
}

Uns16
ShipsInBuildQueue(void)
{
  InitAuxData();

  return gNumBaseOrders;
}

static Boolean
ReadBuildQueue(FILE * pFile)
{
  passert(gBaseOrder NEQ 0);

  {
#ifdef __MSDOS__
    if (PLANET_NR NEQ fread(gBaseOrder, sizeof(BaseOrder_Struct), PLANET_NR,
                pFile)) goto bad_read;
#else
    Uns16 lPlanet;

    for (lPlanet = 0; lPlanet < PLANET_NR; lPlanet++) {
      if (!DOSReadStruct(BaseOrderStruct_Convert, NumBaseOrderStruct_Convert,
                  gBaseOrder + lPlanet, pFile))
        goto bad_read;
    }
#endif
  }

  /* Count the number of current base orders by searching over all orders and 
     stopping when the base number is 0 */

  for (gNumBaseOrders = 0; gBaseOrder[gNumBaseOrders].mBase NEQ 0;
        gNumBaseOrders++)       /* NO ACTION */
    ;

  return True;

bad_read:
  return False;
}

static Boolean
WriteBuildQueue(FILE * pFile)
{
  passert(gBaseOrder NEQ 0);

#ifdef __MSDOS__
  if (PLANET_NR NEQ fwrite(gBaseOrder, sizeof(BaseOrder_Struct), PLANET_NR,
              pFile)) goto bad_write;
#else
  {
    Uns16 lPlanet;

    for (lPlanet = 0; lPlanet < PLANET_NR; lPlanet++) {
      if (!DOSWriteStruct(BaseOrderStruct_Convert, NumBaseOrderStruct_Convert,
                  gBaseOrder + lPlanet, pFile))
        goto bad_write;
    }
  }
#endif

  return True;

bad_write:
  return False;
}

static int
prioritySortFunc(const void *p1, const void *p2)
{
  const BaseOrder_Struct *l1 = (const BaseOrder_Struct *) p1;
  const BaseOrder_Struct *l2 = (const BaseOrder_Struct *) p2;

  Int32 lDiff = (Int32) (l2->mPriority) - (Int32) (l1->mPriority);

  return sgn(lDiff);
}

void
SortBuildQueue(void)
{
  InitAuxData();

  if (gNumBaseOrders)
    qsort(gBaseOrder, gNumBaseOrders, sizeof(gBaseOrder[0]),
          prioritySortFunc);
}

/*
 *    V E R S I O N  I N F O
 */

static Boolean
ReadAuxVersionInfo(FILE * pInFile)
{
  /* 
     Format of versioning/turn sequence info: Byte 0 -- major version Byte 1
     -- minor version Byte 2 -- Turntime_Struct (should match nextturn.hst)
     Byte 22 -- First battle info Byte 24 -- Auxilliary info (14 bytes) Byte
     38 -- next section begins */

  Uns8 lVersion[2];             /* Element 0: major, element 1: minor */
  Turntime_Struct lTurntime;
  Boolean lConverted;           /* True if conversions were required */

  if (1 NEQ fread(lVersion, 2, 1, pInFile)) {
    return False;
  }

  gVersionMajor = lVersion[0];
  gVersionMinor = lVersion[1];

  if (lVersion[0] NEQ PHOST_VERSION_MAJOR) {
    Error("Game data files are incompatible -- they are version %u.%u.\n",
          (Uns16) lVersion[0], (Uns16) lVersion[1]);
    return False;
  }

#ifdef __MSDOS__
  if (1 NEQ fread(&lTurntime, sizeof(lTurntime), 1, pInFile))
    goto bad_read;
#else
  if (!DOSReadStruct(TurntimeStruct_Convert, NumTurntimeStruct_Convert,
              &lTurntime, pInFile))
    goto bad_read;
#endif

  /* Check to make sure timestamp matches NEXTTURN.HST. If not, this file is
     stale. */
  if (memcmp(&lTurntime, RawTurnTime(), sizeof(lTurntime))) {
    Error
          ("AUXDATA.HST file is from turn %u. It is stale and must be deleted.\n",
          lTurntime.TurnNumber);
    return False;
  }

  /* 16 bytes for first-battle and auxilliary info */
  if (1 NEQ fread(gAuxilliaryInfo, 16, 1, pInFile))
    goto bad_read;

  return True;

bad_read:
  return False;
}

static Boolean
WriteAuxVersionInfo(FILE * pOutFile)
{
  Uns8 lVersion[2];             /* Element 0: major, element 1: minor */
  const Turntime_Struct *lTurnPtr;

  lVersion[0] = gVersionMajor;
  lVersion[1] = gVersionMinor;

  if (1 NEQ fwrite(lVersion, 2, 1, pOutFile)) {
    return False;
  }

  lTurnPtr = RawTurnTime();
#ifdef __MSDOS__
  if (1 NEQ fwrite(lTurnPtr, sizeof(*lTurnPtr), 1, pOutFile))
    goto bad_write;
#else
  if (!DOSWriteStruct(TurntimeStruct_Convert, NumTurntimeStruct_Convert,
              lTurnPtr, pOutFile))
    goto bad_write;
#endif

  if (1 NEQ fwrite(gAuxilliaryInfo, 16, 1, pOutFile))
    goto bad_write;

  return True;

bad_write:
  return False;
}

Boolean
GameFilesVersion(Uns16 * pMajor, Uns16 * pMinor)
{
  passert(pMajor NEQ 0);
  passert(pMinor NEQ 0);

  InitAuxData();

  if (gNoAuxData)
    return False;

  *pMajor = gVersionMajor;
  *pMinor = gVersionMinor;
  return True;
}

/*
 *    B A S E    N A T I V E S I N F O
 */

/* The gBaseNatives variable is an array of dimension PLANET_NR which stores
   the type of natives that were present on a planet at the time a starbase
   was built. The value in each element is actually of type NativeType_Def.
   The only purpose behind this rigamarole is to not unfairly downgrade
   the tech of a shareware Cyborg player's starbases. Ugh!
*/

static unsigned char *gBaseNatives = 0;

static void
InitBaseNativeInfo(void)
{
  gBaseNatives = (unsigned char *) MemCalloc(PLANET_NR + 1, 1);
}

static void
FreeBaseNativeInfo(void)
{
  MemFree(gBaseNatives);
  gBaseNatives = 0;
}

/* File I/O */

static Boolean
ReadBaseNativeInfo(FILE * pFile)
{
  passert(gBaseNatives NEQ 0);
  if (1 NEQ fread(gBaseNatives, PLANET_NR + 1, 1, pFile))
    return False;

  return True;
}

static Boolean
WriteBaseNativeInfo(FILE * pFile)
{
  passert(gBaseNatives NEQ 0);
  if (1 NEQ fwrite(gBaseNatives, PLANET_NR + 1, 1, pFile))
    return False;

  return True;
}

/*
 *       A L L I A N C E S
 */

#define ALLY_SHIPS_BIT  (1 << ALLY_SHIPS)
#define ALLY_PLANETS_BIT (1 << ALLY_PLANETS)
#define ALLY_MINES_BIT  (1 << ALLY_MINES)
#define ALLY_COMBAT_BIT  (1 << ALLY_COMBAT)
#define ALLY_VISION_BIT  (1 << ALLY_VISION)
#define ALLY_OFFER_BIT  (1 << (ALLY_VISION + 1))

#define ALLY_SHIPS_COND_BIT   (1 << (8 + ALLY_SHIPS))
#define ALLY_PLANETS_COND_BIT (1 << (8 + ALLY_PLANETS))
#define ALLY_MINES_COND_BIT   (1 << (8 + ALLY_MINES))
#define ALLY_COMBAT_COND_BIT  (1 << (8 + ALLY_COMBAT))
#define ALLY_VISION_COND_BIT  (1 << (8 + ALLY_VISION))

/* This two-dimensional array is to be interpreted as follows:
 gAlliances[Race1][Race2] means Race1 offers alliance to Race2
         or   Race1 allows the alliance level to Race2
*/

static Uns16 gAlliances[RACE_NR + 1][RACE_NR + 1];

static void
InitAlliances(void)
{
  memset(gAlliances, 0, sizeof(gAlliances));
}

void
AllyAddRequest(RaceType_Def pPlayer, RaceType_Def pRace)
{
  InitAuxData();

  gAlliances[pPlayer][pRace] = ALLY_OFFER_BIT; /* set all other bits to 0 */
}

void
AllyDropRequest(RaceType_Def pPlayer, RaceType_Def pRace)
{
  InitAuxData();

  gAlliances[pPlayer][pRace] = 0;
}

Boolean
PlayerAllowsAlly(RaceType_Def pPlayer, RaceType_Def pRace,
      AllianceLevel_Def pLevel)
{
  /* A player allows his ally the given level IF: * The level has been
     configured to be allowed AND * Either the conditional bit for this level 
     has not been set OR * The ally offers the same level to the player */
  InitAuxData();

  return (gAlliances[pPlayer][pRace] & (1 << pLevel))
        AND((!(gAlliances[pPlayer][pRace] & (1 << (pLevel + 8))))
        OR(gAlliances[pRace][pPlayer] & (1 << pLevel))
        );
}

Boolean
IsAllyOfferConditional(RaceType_Def pPlayer, RaceType_Def pRace,
      AllianceLevel_Def pLevel)
{
  InitAuxData();

  return (gAlliances[pPlayer][pRace] & (1 << pLevel))
        AND(gAlliances[pPlayer][pRace] & (1 << (pLevel + 8)));
}

Boolean
IsAllyOfferAny(RaceType_Def pPlayer, RaceType_Def pRace,
      AllianceLevel_Def pLevel)
{
  InitAuxData();

  return (gAlliances[pPlayer][pRace] & (1 << pLevel));
}

void
PutPlayerAllowsAlly(RaceType_Def pPlayer, RaceType_Def pRace,
      AllianceLevel_Def pLevel, AllianceState_Def pState)
{
  InitAuxData();

  switch (pState) {
  case ALLY_STATE_OFF:
    gAlliances[pPlayer][pRace] &= ~((1 << pLevel) | (1 << (pLevel + 8)));
    break;

  case ALLY_STATE_CONDITIONAL:
    gAlliances[pPlayer][pRace] |= (1 << pLevel) | (1 << (pLevel + 8));
    break;

  case ALLY_STATE_ON:
    gAlliances[pPlayer][pRace] &= ~(1 << (pLevel + 8));
    gAlliances[pPlayer][pRace] |= (1 << pLevel);
    break;

  default:
    passert(0);
  }
}

Boolean
PlayerOffersAlliance(RaceType_Def pPlayer, RaceType_Def pRace)
{
  InitAuxData();

  return gAlliances[pPlayer][pRace] & ALLY_OFFER_BIT;
}

Boolean
PlayersAreAllies(RaceType_Def pPlayer, RaceType_Def pRace)
{
  InitAuxData();

  if (!PlayerIsActive(pPlayer) OR ! PlayerIsActive(pRace))
    return False;

  return (gAlliances[pPlayer][pRace] & ALLY_OFFER_BIT)
        AND(gAlliances[pRace][pPlayer] & ALLY_OFFER_BIT);
}

static Boolean
ReadAllianceInfo(FILE * pFile)
{
  if (sizeof(Uns16) NEQ fread(gAlliances, (RACE_NR + 1) * (RACE_NR + 1),
              sizeof(Uns16), pFile))
    return False;
  EndianSwap16(gAlliances, (RACE_NR + 1) * (RACE_NR + 1));

  return True;
}

static Boolean
WriteAllianceInfo(FILE * pFile)
{
  EndianSwap16(gAlliances, (RACE_NR + 1) * (RACE_NR + 1));
  if (sizeof(Uns16) NEQ fwrite(gAlliances, (RACE_NR + 1) * (RACE_NR + 1),
              sizeof(Uns16), pFile))
    return False;
  EndianSwap16(gAlliances, (RACE_NR + 1) * (RACE_NR + 1));

  return True;
}

/*
 *                 P L A Y E R    A C T I V I T Y    L E V E L
 */

static Uns32 gActivityLevel[RACE_NR + 1];

static void
InitActivityLevel(void)
{
  memset(gActivityLevel, 0, sizeof(gActivityLevel));
}

static Boolean
ReadActivityLevel(FILE * pFile)
{
  if ((RACE_NR + 1) NEQ fread(gActivityLevel, sizeof(gActivityLevel[0]),
              RACE_NR + 1, pFile))
    return False;
  EndianSwap32(gActivityLevel, RACE_NR + 1);

  return True;
}

static Boolean
WriteActivityLevel(FILE * pFile)
{
  EndianSwap32(gActivityLevel, RACE_NR + 1);
  if ((RACE_NR + 1) NEQ fwrite(gActivityLevel, sizeof(gActivityLevel[0]),
              RACE_NR + 1, pFile))
    return False;
  EndianSwap32(gActivityLevel, RACE_NR + 1);

  return True;
}

Uns32
GetActivityLevel(Uns16 pRace)
{
  assert((pRace >= 1) && (pRace <= RACE_NR));
  return gActivityLevel[pRace];
}

void
SetActivityLevel(Uns16 pRace, Uns32 pNewLevel)
{
  assert((pRace >= 1) && (pRace <= RACE_NR));
  gActivityLevel[pRace] = pNewLevel;
}

/*
 *                  R E M O T E    C O N T R O L
 */

/* This is an array indexed by ship number. It simply indicates which
   race controls the given ship, or 0 for the default owner. We must
   remember to set this value to 0 whenever a ship is deleted, and for
   safety, whenever it is created.

   The MSB bit is used as a 'forbid' bit. If it is cleared (default)
   then the ship can be remotely controlled. If it is set, the ship
   cannot be controlled.
   */

static Uns16 *gRemoteOwner = 0;

/* This array stores a ship's original owner, or it may be 0. */

static Uns16 *gOriginalOwner = 0;

static void
InitRemoteControl(void)
{
  passert(gRemoteOwner EQ 0);
  passert(gOriginalOwner EQ 0);
  gRemoteOwner = (Uns16 *) MemCalloc(SHIP_NR + 1, sizeof(gRemoteOwner[0]));
  gOriginalOwner =
        (Uns16 *) MemCalloc(SHIP_NR + 1, sizeof(gOriginalOwner[0]));
}

static void
FreeRemoteControl(void)
{
  passert(gRemoteOwner NEQ 0);
  passert(gOriginalOwner NEQ 0);
  MemFree(gRemoteOwner);
  gRemoteOwner = 0;
  MemFree(gOriginalOwner);
  gOriginalOwner = 0;
}

static Boolean
ReadRemoteControl(FILE * pFile)
{
  passert(gRemoteOwner NEQ 0);
  if ((SHIP_NR + 1) NEQ fread(gRemoteOwner, sizeof(gRemoteOwner[0]),
              SHIP_NR + 1, pFile))
    return False;
  EndianSwap16(gRemoteOwner, SHIP_NR + 1);
  if ((SHIP_NR + 1) NEQ fread(gOriginalOwner, sizeof(gOriginalOwner[0]),
              SHIP_NR + 1, pFile))
    return False;
  EndianSwap16(gOriginalOwner, SHIP_NR + 1);

  return True;
}

static Boolean
WriteRemoteControl(FILE * pFile)
{
  EndianSwap16(gRemoteOwner, SHIP_NR + 1);
  if ((SHIP_NR + 1) NEQ fwrite(gRemoteOwner, sizeof(gRemoteOwner[0]),
              SHIP_NR + 1, pFile))
    return False;
  EndianSwap16(gRemoteOwner, SHIP_NR + 1);

  EndianSwap16(gOriginalOwner, SHIP_NR + 1);
  if ((SHIP_NR + 1) NEQ fwrite(gOriginalOwner, sizeof(gOriginalOwner[0]),
              SHIP_NR + 1, pFile))
    return False;
  EndianSwap16(gOriginalOwner, SHIP_NR + 1);

  return True;
}

/************************************************
 *  These are the remote control public accessors
 ************************************************/

Boolean
BeginShipRemoteControl(Uns16 pShipID, Uns16 pRemoteOwner)
{
  Uns16 lOwner1;

  InitAuxData();

  passert(IsShipExist(pShipID));
  passert(gRemoteOwner NEQ 0);
  passert(gOriginalOwner NEQ 0);

  if (gRemoteOwner[pShipID] & 0x8000U) {
    /* Forbidden */
    return False;

  }
  else {
    /* Ship may already be remote controlled. In this case, do not override
       original owner */
    lOwner1 = gOriginalOwner[pShipID];
    if (lOwner1 EQ 0) {
      lOwner1 = ShipOwner(pShipID);
    }

    /* Make sure we're not remote-ing ourselves */
    if (pRemoteOwner EQ lOwner1) {
      return False;
    }

    /* Make sure it is allowed */
    if (PlayerAllowsAlly(lOwner1, pRemoteOwner, ALLY_SHIPS)
          AND PlayersAreAllies(lOwner1, pRemoteOwner)
          ) {
      ;                         /* OK */
    }
    else
      return False;

    gRemoteOwner[pShipID] = pRemoteOwner;
    gOriginalOwner[pShipID] = lOwner1;

    return True;
  }
}

Boolean
EndShipRemoteControl(Uns16 pShipID, Uns16 pRemoteOwner)
{
  Uns16 lOwner2;

  InitAuxData();

  passert(gRemoteOwner NEQ 0);

  /* We only terminate the remote control if the pRemoteOwner parameter
     matches the current remote controller. */

  if (!IsShipExist(pShipID))
    return False;

  lOwner2 = gRemoteOwner[pShipID] & 0x7FFFU;
  if (lOwner2 EQ pRemoteOwner) {
    gRemoteOwner[pShipID] = 0;
    return True;
  }
  else {
    return False;
  }
  /* Do NOT set original owner to 0 since we will need to swap out */
}

/* This routine checks to see if a ship is being remote controlled. The
   caller should consider the status of remote control (?) */
Boolean
IsShipRemoteControlled(Uns16 pShip)
{
  InitAuxData();

  passert(gRemoteOwner NEQ 0);

  return (Boolean) ((gRemoteOwner[pShip] & 0x7FFFU) NEQ 0);
}

/* This routine checks to see if a ship is forbidden from being remotely
   controlled */
Boolean
IsShipRemoteForbidden(Uns16 pShip)
{
  InitAuxData();

  passert(gRemoteOwner NEQ 0);

  return (Boolean) ((gRemoteOwner[pShip] & 0x8000U) NEQ 0);
}

/* Calling this routine is suitable for when a ship is destroyed, created, etc. */

void
ResetShipRemoteControl(Uns16 pShip)
{
  InitAuxData();

  passert(gRemoteOwner NEQ 0);
  passert(gOriginalOwner NEQ 0);

  gRemoteOwner[pShip] = 0;
  gOriginalOwner[pShip] = 0;
}

Boolean
AllowShipRemoteControl(Uns16 pShipID, Uns16 pOrigOwner)
{
  Uns16 lOwner1;

  InitAuxData();

  passert(gRemoteOwner NEQ 0);
  passert(gOriginalOwner NEQ 0);

  if (!IsShipExist(pShipID))
    return False;

  lOwner1 = gOriginalOwner[pShipID];
  if (lOwner1 EQ 0) {
    lOwner1 = ShipOwner(pShipID);
  }

  /* Only allow this kind of control if original owner matches pOrigOwner */
  if (lOwner1 EQ pOrigOwner) {
    if (gRemoteOwner[pShipID] & 0x8000U) {
      gRemoteOwner[pShipID] &= 0x7FFFU;
    }
    return True;
  }
  else {
    return False;
  }
}

Boolean
ForbidShipRemoteControl(Uns16 pShipID, Uns16 pOrigOwner)
{
  Uns16 lOwner1;

  InitAuxData();

  passert(gRemoteOwner NEQ 0);
  passert(gOriginalOwner NEQ 0);

  lOwner1 = gOriginalOwner[pShipID];
  if (lOwner1 EQ 0) {
    lOwner1 = ShipOwner(pShipID);
  }

  if (pOrigOwner EQ lOwner1) {
    gRemoteOwner[pShipID] = 0x8000U; /* And remove remote owner */
    /* Leave original owner the same, so we can swap out */
  }
  else
    return False;

  return True;
}

Uns16
ShipRemoteController(Uns16 pShipID)
{
  InitAuxData();

  passert(gRemoteOwner NEQ 0);
  return gRemoteOwner[pShipID] & 0x7FFFU;
}

Uns16
ShipRemoteOwner(Uns16 pShipID)
{
  InitAuxData();

  passert(gOriginalOwner NEQ 0);
  return gOriginalOwner[pShipID];
}

/*
 *    F I L E    I N P U T A N D  O U T P U T
 *
 *       A N D I N I T I A L I Z A T I O N
 */

/* In this routine, we handle the case of a missing auxdata.hst file, as
   may happen if PHOST is taking over a HOST game. We need to set our
   data structures to be non-intrusive. Specifically, we must:
  1. Assume that all ships can see each other. This is so that the
     cheat-checking code will not complain for ships intercepting
     other ships that "cannot be seen".

  2. Flush the build queue. Nothing has to be done here since the
     build queue is initialized to all 0's which means there's
     nothing in it.

  3. Fake entries in the original base info array. Here we take our
     best guess and say that the natives present when a base was
     originally built are the natives that are on the planet now.

  4. Clear all alliance information (not needed since array is
     initalized as all 0's)
*/

static void
HandleMissingAuxdata(void)
{
  InitAuxData();

  /* Flush build queue */
  gNumBaseOrders = 0;
  gNoAuxData = True;
}

static Boolean
ReadAuxData(FILE * pAuxDataFile)
{
  InitAuxData();

  if (!ReadAuxVersionInfo(pAuxDataFile))
    goto bad_read;
  if (!ReadBaseNativeInfo(pAuxDataFile))
    goto bad_read;
  if (!ReadAllianceInfo(pAuxDataFile))
    goto bad_read;
  if (!ReadScanInfo(pAuxDataFile))
    goto bad_read;
  if (!ReadBuildQueue(pAuxDataFile))
    goto bad_read;
  if (!ReadActivityLevel(pAuxDataFile))
    goto bad_read;
  if (!ReadRemoteControl(pAuxDataFile))
    goto bad_read;

  gAuxdataLoaded = True;

  return True;
bad_read:
  Error("Unable to read from file 'auxdata.hst'");
  return False;
}

static Boolean
WriteAuxData(FILE * pAuxDataFile)
{
  InitAuxData();

  if (gNoAuxData)
    return True;

  if (!gAuxdataLoaded) {
    Error("Cannot write auxdata file without first reading it.\n");
    return False;
  }

  if (!WriteAuxVersionInfo(pAuxDataFile))
    goto bad_write;
  if (!WriteBaseNativeInfo(pAuxDataFile))
    goto bad_write;
  if (!WriteAllianceInfo(pAuxDataFile))
    goto bad_write;
  if (!WriteScanInfo(pAuxDataFile))
    goto bad_write;
  if (!WriteBuildQueue(pAuxDataFile))
    goto bad_write;
  if (!WriteActivityLevel(pAuxDataFile))
    goto bad_write;
  if (!WriteRemoteControl(pAuxDataFile))
    goto bad_write;

  return True;
bad_write:
  Error("Unable to write to file 'auxdata.hst'");
  return False;
}

static const char *AUXDATA_FILE = "auxdata.hst"; /* PHOST-specific */

IO_Def
Read_AuxData_File(void)
{
  FILE *lAuxDataFile;
  IO_Def lError = IO_SUCCESS;

  if ((lAuxDataFile =
              OpenInputFile(AUXDATA_FILE,
            GAME_DIR_ONLY | NO_MISSING_ERROR)) NEQ NULL) {
    if (!ReadAuxData(lAuxDataFile)) {
      lError = IO_FAILURE;
    }
    else
      gReadAuxData = True;
    fclose(lAuxDataFile);
  }
  else {
    if (gNonPHOSTWarnings)
      Warning("Can't find file '%s'", AUXDATA_FILE);
    HandleMissingAuxdata();
    gReadAuxData = False;
  }
  return (lError);
}

IO_Def
Write_AuxData_File(void)
{
  FILE *lAuxDataFile;
  IO_Def lError = IO_SUCCESS;

  if (!gReadAuxData)
    return IO_SUCCESS;

  if ((lAuxDataFile = OpenOutputFile(AUXDATA_FILE, GAME_DIR_ONLY)) NEQ NULL) {
    if (!WriteAuxData(lAuxDataFile)) {
      lError = IO_FAILURE;
    }
    fclose(lAuxDataFile);
  }
  else
    lError = IO_FAILURE;
  return (lError);
}

/* Write out the CLOAKC.HST file introduced in HOST 3.22.017. From the
   docs:

   Host writes a file named CLOAKC.HST before running auxbc.ini (500 ints
   of cloak flags and 500 ints of shield levels) (The cloak flags are read
   back in after auxbc.ini is done.)

   NOTE: HOST doesn't write this file unless AUXBC.INI is present. We do
         it anyway.
*/

Boolean
WriteCLOAKCFile(void)
{
  Uns16 lShip;
  FILE *lFile = OpenOutputFile("cloakc.hst", GAME_DIR_ONLY);
  Uns16 lStatus;

  /* Info(StringRetrieve(S__Writing_File, gConfigInfo->Language[0]),
     "CLOAKC.HST"); */

  /* Write cloak status from scaninfo */
  for (lShip = 1; lShip <= SHIP_NR; lShip++) {
    if (IsShipExist(lShip)) {
      lStatus = IsShipCloaked(lShip) ? 1 : 0;
    }
    else {
      lStatus = 0;
    }

    (void) DOSWrite16(&lStatus, 1, lFile);
  }

  /* Write shield levels */
  for (lShip = 1; lShip <= SHIP_NR; lShip++) {
    if (IsShipExist(lShip)) {
      lStatus = ShipCombatShieldLevel(lShip);
    }
    else {
      lStatus = 0;
    }

    (void) DOSWrite16(&lStatus, 1, lFile);
  }
  fclose(lFile);
  return True;
}

/* Read the CLOAKC.HST file. */

void
ReadCLOAKCFile(void)
{
  Uns16 lShip;
  FILE *lFile = OpenInputFile("cloakc.hst", GAME_DIR_ONLY | NO_MISSING_ERROR);
  Uns16 lStatus;

  if (lFile EQ 0) {
    Warning("CLOAKC.HST file does not exist!");
    gCloakcLoaded = False;
    return;
  }

  /* Info(StringRetrieve(S__Reading_File, gConfigInfo->Language[0]),
     "CLOAKC.HST"); */

  /* Read cloak status */
  for (lShip = 1; lShip <= SHIP_NR; lShip++) {
    if (!DOSRead16(&lStatus, 1, lFile)) {
      Error("Unable to read info for ship %u in CLOAKC.HST file", lShip);
      return;
    }

    if (IsShipExist(lShip)) {
      if (lStatus EQ 0) {
        /* Ship is NOT cloaked */
        gScanInfo[lShip] &= 0x7fffU;
      }
      else {
        gScanInfo[lShip] |= 0x8000U;
      }
    }
  }

  /* Read shield levels */
  passert(gCombatShields);

  for (lShip = 1; lShip <= SHIP_NR; lShip++) {
    if (!DOSRead16(&lStatus, 1, lFile)) {
      Error("Unable to read shield info for ship %u in CLOAKC.HST file",
            lShip);
      return;
    }

    if (IsShipExist(lShip)) {
      gCombatShields[lShip] = lStatus;
    }
  }

  gCloakcLoaded = True;
  fclose(lFile);

  return;
}

void
MarkShipCloaking(Uns16 pID, Boolean pCloaked)
{
  if (IsShipExist(pID)) {
    if (pCloaked AND ShipCanCloak(pID))
      gScanInfo[pID] |= 0x8000U;
    else
      gScanInfo[pID] &= 0x7fffU;
  }
}

Uns16
ShipCombatShieldLevel(Uns16 pID)
{
  if (IsShipExist(pID))
    return gCombatShields[pID];
  else
    return 0;
}

void
SetShipCombatShieldLevel(Uns16 pID, Uns16 pShields)
{
  if (IsShipExist(pID))
    gCombatShields[pID] = pShields;
}

extern Boolean
IsCLOAKCFound(void)
{
  return gCloakcLoaded;
}

/*************************************************************
  $HISTORY:$
**************************************************************/

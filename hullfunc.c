/**
  *  \file hullfunc.c
  *  \brief Hull Function Handling
  *
  *  \author Andrew Sterian (original)
  *  \author Maurits van Rees (HullXXX functions)
  *  \author Stefan Reuther (updates for PHost 4.0i)
  *
  *  This file defines the functions to access hull functions. It
  *  reads hullfunc.txt resp. shiplist.txt, and the relevant
  *  auxdata.hst parts.
  *
  *  All files in this distribution are Copyright (C) 1995-2000 by the program
  *  authors: Andrew Sterian, Thomas Voigt, and Steffen Pietsch.
  *  You can reach the PHOST team via email (support@phost.de).
  *
  *  This program is free software; you can redistribute it and/or
  *  modify it under the terms of the GNU General Public License
  *  as published by the Free Software Foundation; either version 2
  *  of the License, or (at your option) any later version.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.
  *
  *  You should have received a copy of the GNU General Public License
  *  along with this program; if not, write to the Free Software
  *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
  */

#include <ctype.h>
#include "phostpdk.h"
#include "private.h"
#include "listmat.h"

static const char HULLFUNC_FILE[] = "hullfunc.txt";
const char SHIPLIST_FILE[] = "shiplist.txt";

/* Here we define the specials that we currently recognize */
typedef enum {
  SPC_Alchemy = 0,
  SPC_Refinery,
  SPC_AdvancedRefinery,
  SPC_HeatsTo50,
  SPC_CoolsTo50,
  SPC_HeatsTo100,
  SPC_Hyperdrive,
  SPC_Gravitonic,
  SPC_ScansAllWormholes,
  SPC_Gambling,
  SPC_AntiCloak,
  SPC_ImperialAssault,
  SPC_Chunneling,
  SPC_Ramscoop,
  SPC_FullBioscan,
  SPC_AdvancedCloak,
  SPC_Cloak,
  SPC_Bioscan,
  SPC_GloryDevice,              /* Low damage glory device */
  SPC_HEGloryDevice,            /* High damage glory device */
  SPC_Unclonable,
  SPC_CloneOnce,
  SPC_Ungiveable,
  SPC_GiveOnce,
  SPC_Level2Tow,
  SPC_Tow,                    /* 25 */
  SPC_ChunnelSelf,
  SPC_ChunnelOthers,
  SPC_ChunnelTarget,
  SPC_PlanetImmunity,
  SPC_OreCondenser,           /* 30 */
  SPC_Boarding,
  SPC_AntiCloakImmunity,
  SPC_Academy,
  SPC_Repairs,
  SPC_FullWeaponry,           /* 35 */
  SPC_HardenedEngines,
  SPC_Commander,
  SPC_IonShield,
  SPC_HardenedCloak,
  SPC_AdvancedAntiCloak,      /* 40 */

  NumSpecials,

  SPC_NO_SPECIAL  /* Marker */
} Special_Def;

/* Must match order of Special_Def above */
static const char *gSpecialNames[] = {
  "Alchemy",
  "Refinery",
  "AdvancedRefinery",
  "HeatsTo50",
  "CoolsTo50",
  "HeatsTo100",
  "Hyperdrive",
  "Gravitonic",
  "ScansAllWormholes",
  "Gambling",
  "AntiCloak",
  "ImperialAssault",
  "Chunneling",
  "Ramscoop",
  "FullBioscan",
  "AdvancedCloak",
  "Cloak",
  "Bioscan",
  "GloryDeviceLowDamage",
  "GloryDeviceHighDamage",
  "Unclonable",
  "CloneOnce",
  "Ungiveable",
  "GiveOnce",
  "Level2Tow",
  "Tow",
  "ChunnelSelf",
  "ChunnelOthers",
  "ChunnelTarget",
  "PlanetImmunity",
  "OreCondenser",
  "Boarding",
  "AntiCloakImmunity",
  "Academy",
  "Repairs",
  "FullWeaponry",
  "HardenedEngines",
  "Commander",
  "IonShield",
  "HardenedCloak",
  "AdvancedAntiCloak",
};

#define NumSpecialNames (sizeof(gSpecialNames)/sizeof(gSpecialNames[0]))

/** Definition of a synthetic (level-modified) hull function. */
typedef struct {
  Int16 mFunction;              /**< Basic function number. */
  Int16 mHostBit;               /**< Number of this function in host data. -1 if none. */
  Uns16 mLevelMask;             /**< Level mask. */
} Special_Struct;

/* This is where we implement HULLFUNC lookup using an array of arrays. The
   structure is as follows:

    * gHullfunc is an array of HULL_NR+1 elements, indexed by hull number
      (0th entry unused)

    * each entry of gHullfunc is NULL if this hull has no specials, for any
      race

    * otherwise, each entry of gHullfunc points to dynamic memory which stores
      an array indexed by special number (from 0 to NUM_SPECIALS-1).

    * each entry of the specials array contains a 16-bit value which is a mask
      describing which players have access to this special, in the usual
      (1 << player) fashion. Note that this is NOT mapped through EffRace()
      since we want to have the freedom to differentiate by players, not
      races.

   This scheme (index by hull-special-race) takes up less space per hull than
   indexing by hull-race-special as long as the number of specials is less
   than 23. With 23 or more specials, indexing by race-then-special will take
   less space (up to 32 specials, using 32-bit words).
*/

static Uns16 **gHullfunc = 0;
static Uns16 **gSynthHullfunc = 0;

/* This is where we store the default special assignments if no HULLFUNC.TXT
   file is found.
*/
typedef struct {
  Uns16 mHull;
  Special_Def mSpecial;
} SpecialInfo_Struct;

static SpecialInfo_Struct gDefaultSpecials[] = {
  {3, SPC_HeatsTo50},           /* Bohemian */
  {3, SPC_ScansAllWormholes},   /* Bohemian */
  {7, SPC_AntiCloak},           /* Loki */
  {8, SPC_CoolsTo50},           /* Eros */
  {9, SPC_Bioscan},             /* Brynhild */
  {21, SPC_Cloak},              /* Reptile  */
  {22, SPC_Cloak},              /* LCC */
  {25, SPC_Cloak},              /* Saurian */
  {26, SPC_Cloak},              /* White Falcon */
  {27, SPC_Cloak},              /* Swift Heart */
  {28, SPC_Cloak},              /* Fearless Wing */
  {29, SPC_AdvancedCloak},      /* Darkwing */
  {31, SPC_AdvancedCloak},      /* Resolute */
  {32, SPC_Cloak},              /* Bright Heart */
  {33, SPC_Cloak},              /* Deth Specula */
  {36, SPC_Cloak},              /* D7 Coldpain */
  {38, SPC_Cloak},              /* D3 Thorn */
  {39, SPC_HEGloryDevice},      /* D19b Nefarious */
  {41, SPC_GloryDevice},        /* Saber */
  {42, SPC_Gambling},           /* Lady Royale */
  {43, SPC_Cloak},              /* Dwarfstar */
  {44, SPC_Gravitonic},         /* BR4 */
  {44, SPC_Cloak},              /* BR4 */
  {45, SPC_Gravitonic},         /* BR5 */
  {45, SPC_Cloak},              /* BR5 */
  {46, SPC_Gravitonic},         /* MBR */
  {46, SPC_Cloak},              /* MBR */
  {47, SPC_Cloak},              /* Red Wind */
  {51, SPC_Hyperdrive},         /* B200 */
  {56, SPC_Chunneling},         /* Firecloud */
  {64, SPC_HeatsTo100},         /* Onyx */
  {69, SPC_ImperialAssault},    /* SSD */
  {77, SPC_Hyperdrive},         /* PL21 */
  {84, SPC_FullBioscan},        /* Pawn */
  {87, SPC_Hyperdrive},         /* Falcon */
  {96, SPC_Ramscoop},           /* Cobol */
  {96, SPC_Bioscan},            /* Cobol */
  {97, SPC_AdvancedRefinery},   /* Aries */
  {104, SPC_Refinery},          /* NR ship */
  {105, SPC_Alchemy},           /* Merlin */
  {69, SPC_PlanetImmunity},
};

#define NumDefaultSpecials (sizeof(gDefaultSpecials)/sizeof(gDefaultSpecials[0]))

/* Definition of synthetic hull functions.
   We support a maximum of 256 (=NumSynthSpecials) functions, like PHost.
   We may have to distinguish between standard and synthesized functions.
   Therefore, we identify synthetic functions by adding FirstSynthSpecial
   to their number. This does never appear in the user interface. */
#define NumSynthSpecials 256
#define FirstSynthSpecial 128
static Special_Struct sSynthSpecials[NumSynthSpecials];
static Uns16 sNumSynthSpecials;
static Uns16 sAllLevelsMask = (1 << 11) - 1;

static void clearAllSpecials(void);

static void
ShutdownHullfunc(void)
{
  clearAllSpecials();
  if (gHullfunc) {
    MemFree(gHullfunc);
    gHullfunc = 0;
  }
  if (gSynthHullfunc) {
    MemFree(gSynthHullfunc);
    gSynthHullfunc = 0;
  }
  sNumSynthSpecials = 0;
}

static void
InitHullfunc(void)
{
  passert(NumSpecialNames EQ NumSpecials);
  if (gHullfunc) {
    passert(gSynthHullfunc);
    return;                     /* Already initialized */
  }

  gHullfunc = (Uns16 **) MemCalloc(HULL_NR + 1, sizeof(Uns16 *));
  gSynthHullfunc = (Uns16 **) MemCalloc(HULL_NR + 1, sizeof(Uns16 *));

  RegisterCleanupFunction(ShutdownHullfunc);
}

/** Given a function, return an integer usable to identify it.
    Returns [0,NumSpecials) if it is a regular function.
    Returns [FirstSynthSpecial,FirstSynthSpecial+NumSynthSpecials]
    if it is a synthetic function. In that case, it also assimilates
    the information from pSpecial into the mapping table.
    Returns -1 if we don't know this function. */
static int
getFunctionFromDef(const Special_Struct* pSpecial)
{
  unsigned i;

  /* do we know the function? */
  if (pSpecial->mFunction == NumSpecials)
    return -1;
  passert(pSpecial->mFunction >= 0 AND pSpecial->mFunction < NumSpecials);

  /* is it a standard function? */
  if ((pSpecial->mLevelMask & sAllLevelsMask) == sAllLevelsMask)
    return pSpecial->mFunction;

  /* not a standard function, try to find one. */
  for (i = 0; i < sNumSynthSpecials; ++i) {
    if (sSynthSpecials[i].mFunction == pSpecial->mFunction
        && (sSynthSpecials[i].mLevelMask & sAllLevelsMask) == (pSpecial->mLevelMask & sAllLevelsMask))
    {
      /* found a function. If the new definition comes with a mHostBit
         and the previous one doesn't yet have one, remember it. */
      if (sSynthSpecials[i].mHostBit < 0)
        sSynthSpecials[i].mHostBit = pSpecial->mHostBit;
      return i + FirstSynthSpecial;
    }
  }

  /* allocate new function */
  if (sNumSynthSpecials >= NumSynthSpecials) {
    static char sBeenHere = 0;
    if (!sBeenHere) {
      Warning("Too many modified hull functions. Ignoring some.");
      sBeenHere = 1;
    }
    return -1;
  }

  i = sNumSynthSpecials++;
  sSynthSpecials[i] = *pSpecial;

  return i + FirstSynthSpecial;
}

/** Set races for a special function.
    \param pHull    hull to process
    \param pSpecial function to assign
    \param pAdd     add for these races
    \param pRemove  remove for these races */
static void
setSpecialRaces(Uns16 pHull, const Special_Struct* pSpecial, Uns16 pAdd, Uns16 pRemove)
{
  int lFunc = getFunctionFromDef(pSpecial);
  Uns16* lEntry;

  if (lFunc < 0)
    return;

  passert(pHull >= 1 AND pHull <= HULL_NR);
  passert(gHullfunc);
  passert(gSynthHullfunc);

  if (lFunc >= 0 && lFunc < NumSpecials) {
    /* standard function */
    if (gHullfunc[pHull] EQ 0) {
      gHullfunc[pHull] = (Uns16 *) MemCalloc(NumSpecials, sizeof(Uns16));
    }
    lEntry = &gHullfunc[pHull][lFunc];
  } else if (lFunc >= FirstSynthSpecial && lFunc < FirstSynthSpecial + NumSynthSpecials) {
    /* synthetic function */
    if (gSynthHullfunc[pHull] EQ 0) {
      gSynthHullfunc[pHull] = (Uns16*) MemCalloc(NumSynthSpecials, sizeof(Uns16));
    }
    lEntry = &gSynthHullfunc[pHull][lFunc - FirstSynthSpecial];
  } else {
    passert(0);
    return;
  }

  *lEntry = (*lEntry & ~pRemove) | pAdd;
}

/** Free all hull functions. */
static void
clearAllSpecials(void)
{
  Uns16 lHull;

  passert(gHullfunc);
  passert(gSynthHullfunc);

  for (lHull = 1; lHull <= HULL_NR; lHull++) {
    MemFree(gHullfunc[lHull]);
    MemFree(gSynthHullfunc[lHull]);
    gHullfunc[lHull] = 0;
    gSynthHullfunc[lHull] = 0;
  }
}

/** Give all ships a particular function.
    \param pFunc     race query function
    \param pRace     process races where pFunc returns pRace
    \param pSpecial  basic function to assign */
static void
giveAllShips(Uns16 (*pFunc)(Uns16), RaceType_Def pRace, Special_Def pSpecial)
{
  Uns16 lMask = 0;
  int i;
  Special_Struct lDev;

  for (i = 1; i <= RACE_NR; ++i)
    if (pFunc(i) == pRace)
      lMask |= 1 << i;

  if (lMask) {
    lDev.mFunction = pSpecial;
    lDev.mHostBit = -1;
    lDev.mLevelMask = sAllLevelsMask;
    for (i = 1; i <= HULL_NR; ++i)
      setSpecialRaces(i, &lDev, lMask, 0);
  }
}

/** Add all default specials. */
static void
addDefaultSpecials(void)
{
  int i;
  Uns16 lMask;
  SpecialInfo_Struct *lInfo;
  Special_Struct lDev;

  passert(gHullfunc);

  /* Set a mask that enables all players for any special */
  lMask = ~0;
  for (i = 0, lInfo = gDefaultSpecials; i < NumDefaultSpecials; i++, lInfo++) {
    lDev.mFunction = lInfo->mSpecial;
    lDev.mHostBit = -1;
    lDev.mLevelMask = sAllLevelsMask;
    setSpecialRaces(lInfo->mHull, &lDev, lMask, 0);
  }

  /* Now the PHost4 specific things */
  if (!gConfigInfo->PlanetKlingonAttack)
    giveAllShips(EffRace, Klingons, SPC_PlanetImmunity);
  if (!gConfigInfo->PlanetRebelAttack)
    giveAllShips(EffRace, Rebels, SPC_PlanetImmunity);
  if (gConfigInfo->AllowPrivTowCapture)
    giveAllShips(EffMission, Privateers, SPC_Boarding);
  if (gConfigInfo->AllowCrystalTowCapture)
    giveAllShips(EffMission, Crystals, SPC_Boarding);
  if (gConfigInfo->FedCrewBonus)
    giveAllShips(EffRace, Federation, SPC_FullWeaponry);
  for (i = 1; i <= HULL_NR; ++i) {
    lDev.mFunction = SPC_Tow;
    lDev.mHostBit = -1;
    lDev.mLevelMask = sAllLevelsMask;
    if (gConfigInfo->OneTow || HullEngineNumber(i) > 1)
      setSpecialRaces(i, &lDev, lMask, 0);
  }
}

/** Check whether ship has basic special function. Checks per-ship and regular specials.
    \param pShip    ship
    \param pSpecial special function */
static Boolean
shipHasBasicSpecial(Uns16 pShip, Special_Def pSpecial)
{
  struct Auxdata_Chunk* lChunk;
  Uns16 lOwner;
  Uns16* lPtr;

  /* may be per-ship special */
  lChunk = GetAuxdataChunkById(aux_ShipSpecial);
  if (pSpecial < 64 && AuxdataChunkSize(lChunk) >= 8 * pShip) {
      unsigned char* lEle = AuxdataChunkData(lChunk);
      if (lEle[8 * (pShip-1) + pSpecial / 8] & (1 << (pSpecial & 7)))
          return True;
  }

  /* check basic special */
  lOwner = ShipOwner(pShip);
  lPtr = gHullfunc[ShipHull(pShip)];

  if (lPtr) {
    return (lPtr[pSpecial] & (1 << lOwner)) ? True : False;
  }

  return False;
}

/** Check whether ship has synthetic function. Checks per-ship and regular specials.
    \param pShip ship
    \param pIndex index, WITHOUT FirstSynthSpecial added */
static Boolean
shipHasSynthSpecial(Uns16 pShip, int pIndex)
{
  struct Auxdata_Chunk* lChunk;
  Uns16 lOwner;
  Uns16* lPtr;
  Uns16 lBit;
  size_t lChunkSize;

  /* sanity check */
  if (pIndex < 0 || pIndex >= sNumSynthSpecials)
    return False;

  /* may be per-ship special. It can be one only if host knows about it. */
  if (sSynthSpecials[pIndex].mHostBit >= 0) {
    lBit = sSynthSpecials[pIndex].mHostBit;

    /* check old record (8 bytes per ship) */
    lChunk = GetAuxdataChunkById(aux_ShipExtraSpecialOld);
    if (lBit < 64 && AuxdataChunkSize(lChunk) >= 8 * pShip) {
      unsigned char* lEle = AuxdataChunkData(lChunk);
      if (lEle[8 * (pShip-1) + lBit / 8] & (1 << (lBit & 7)))
        return True;
    }

    /* check new record (variable size) */
    lChunk = GetAuxdataChunkById(aux_ShipExtraSpecialNew);
    lChunkSize = AuxdataChunkSize(lChunk);
    if (lChunkSize > 2) {
      Uns16 lRecordSize = ReadDOSUns16(AuxdataChunkData(lChunk));
      if (lBit < lRecordSize*8 && lChunkSize >= 2 + lRecordSize * pShip) {
        unsigned char* lEle = AuxdataChunkData(lChunk);
        lEle += 2;
        if (lEle[lRecordSize * (pShip-1) + lBit / 8] & (1 << (lBit & 7)))
          return True;
      }
    }
  }

  /* check hull special */
  lOwner = ShipOwner(pShip);
  lPtr = gSynthHullfunc[ShipHull(pShip)];

  if (lPtr) {
    return (lPtr[pIndex] & (1 << lOwner)) ? True : False;
  }

  return False;
}

/** Check whether ship has a special function. All-in-one function
    that checks all data sources.
    \param pShip the ship
    \param pSpecial special function */
static Boolean
shipHasSpecial(Uns16 pShip, Special_Def pSpecial)
{
  /* NOTE: Do NOT map through EffRace() */
  struct Auxdata_Chunk* lChunk;
  unsigned i;

  InitHullfunc();

  /* must not be inhibited */
  lChunk = GetAuxdataChunkById(aux_SpecialInhibited);
  if (pSpecial < 64 && AuxdataChunkSize(lChunk) >= 8 * pShip) {
    unsigned char* lEle = AuxdataChunkData(lChunk);
    if (lEle[8 * (pShip-1) + pSpecial / 8] & (1 << (pSpecial & 7)))
      return False;
  }

  /* does it have the basic device? */
  if (shipHasBasicSpecial(pShip, pSpecial))
    return True;

  /* now check modified devices */
  for (i = 0; i < sNumSynthSpecials; ++i) {
    if (sSynthSpecials[i].mFunction != pSpecial)
      continue;
    if ((sSynthSpecials[i].mLevelMask & (1 << ExperienceLevel(ShipExperience(pShip)))) == 0)
      continue;
    if (shipHasSynthSpecial(pShip, i))
      return True;
  }

  return False;
}

/** Check whether hull has a special function. Checks for regular
    functions only. */
static Boolean
hullHasSpecial(Uns16 pHull, Special_Def pSpecial, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  /* NOTE: Do NOT map through EffRace() */
  Uns16 *lPtr;

  passert(Owner >= 1 AND Owner <= 11);

  InitHullfunc();

  lPtr = gHullfunc[pHull];

  if (lPtr) {
    return (lPtr[pSpecial] & (1 << Owner)) ? True : False;
  }

  return False;
}

Boolean
ShipDoesAlchemy(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_Alchemy);
}

Boolean
HullDoesAlchemy(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_Alchemy, Owner);
}

Boolean
ShipDoesRefinery(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_Refinery);
}

Boolean
HullDoesRefinery(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_Refinery, Owner);
}

Boolean
ShipDoesAdvancedRefinery(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_AdvancedRefinery);
}

Boolean
HullDoesAdvancedRefinery(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_AdvancedRefinery, Owner);
}

Boolean
ShipHeatsTo50(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_HeatsTo50);
}

Boolean
HullHeatsTo50(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_HeatsTo50, Owner);
}

Boolean
ShipCoolsTo50(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_CoolsTo50);
}

Boolean
HullCoolsTo50(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_CoolsTo50, Owner);
}

Boolean
ShipCools(Uns16 pShip)
{
  /* NOTE: Added by Maurits 2004-08-22 */
  return ShipCoolsTo50(pShip);
}

Boolean
HullCools(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-08-22 */
  return HullCoolsTo50(pHull, Owner);
}

Boolean
ShipHeatsTo100(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_HeatsTo100);
}

Boolean
HullHeatsTo100(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_HeatsTo100, Owner);
}

Boolean
ShipHeats(Uns16 pShip)
{
  /* NOTE: Added by Maurits 2004-08-22 */
  return (ShipHeatsTo50(pShip) OR ShipHeatsTo100(pShip));
}

Boolean
HullHeats(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-08-22 */
  return (HullHeatsTo50(pHull, Owner) OR HullHeatsTo100(pHull, Owner));
}

Boolean
ShipCanHyperwarp(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_Hyperdrive);
}

Boolean
HullCanHyperwarp(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_Hyperdrive, Owner);
}

Boolean
ShipIsGravitonic(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_Gravitonic);
}

Boolean
HullIsGravitonic(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_Gravitonic, Owner);
}

Boolean
ShipScansAllWormholes(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_ScansAllWormholes);
}

Boolean
HullScansAllWormholes(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_ScansAllWormholes, Owner);
}

Boolean
ShipIsGamblingShip(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_Gambling);
}

Boolean
HullIsGamblingShip(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_Gambling, Owner);
}

Boolean
ShipIsAntiCloaking(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_AntiCloak)
      || ShipHasAdvancedAntiCloak(pShip);
}

Boolean
HullIsAntiCloaking(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_AntiCloak, Owner)
      || HullHasAdvancedAntiCloak(pHull, Owner);
}

Boolean
ShipDoesImperialAssault(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_ImperialAssault);
}

Boolean
HullDoesImperialAssault(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_ImperialAssault, Owner);
}

Boolean
ShipDoesChunneling(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_Chunneling);
}

Boolean
HullDoesChunneling(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_Chunneling, Owner);
}

Boolean
ShipHasRamScoop(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_Ramscoop);
}

Boolean
HullHasRamScoop(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_Ramscoop, Owner);
}

Boolean
ShipDoesFullBioscan(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_FullBioscan);
}

Boolean
HullDoesFullBioscan(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_FullBioscan, Owner);
}

Boolean
ShipHasAdvancedCloak(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_AdvancedCloak);
}

Boolean
HullHasAdvancedCloak(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_AdvancedCloak, Owner);
}

Boolean
ShipCanCloak(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_Cloak)
      || ShipHasAdvancedCloak(pShip)
      || ShipHasHardenedCloak(pShip);
}

Boolean
HullCanCloak(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_Cloak, Owner)
      || HullHasAdvancedCloak(pHull, Owner)
      || HullHasHardenedCloak(pHull, Owner);
}

Boolean
ShipDoesBioscan(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_Bioscan) OR ShipDoesFullBioscan(pShip);
}

Boolean
HullDoesBioscan(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_Bioscan, Owner) \
      OR HullDoesFullBioscan(pHull, Owner);
}

Boolean
ShipHasGloryDevice(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_GloryDevice);
}

Boolean
HullHasGloryDevice(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_GloryDevice, Owner);
}

Boolean
ShipHasHEGloryDevice(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_HEGloryDevice);
}

Boolean
HullHasHEGloryDevice(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_HEGloryDevice, Owner);
}

Boolean
ShipIsUnclonable(Uns16 pShip)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return shipHasSpecial(pShip, SPC_Unclonable);
}

Boolean
HullIsUnclonable(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_Unclonable, Owner);
}

Boolean
ShipIsCloneableOnce(Uns16 pShip)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return shipHasSpecial(pShip, SPC_CloneOnce);
}

Boolean
HullIsCloneableOnce(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_CloneOnce, Owner);
}

Boolean
ShipIsUngiveable(Uns16 pShip)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return shipHasSpecial(pShip, SPC_Ungiveable);
}

Boolean
HullIsUngiveable(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_Ungiveable, Owner);
}

Boolean
ShipIsGiveableOnce(Uns16 pShip)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return shipHasSpecial(pShip, SPC_GiveOnce);
}

Boolean
HullIsGiveableOnce(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_GiveOnce, Owner);
}

Boolean
ShipHasLevel2Tow(Uns16 pShip)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return shipHasSpecial(pShip, SPC_Level2Tow);
}

Boolean
HullHasLevel2Tow(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_Level2Tow, Owner);
}

/* NEW 21/May/2005: */
Boolean
ShipCanTow(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_Tow) || ShipHasLevel2Tow(pShip);
}

Boolean
HullCanTow(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_Tow, pOwner) || HullHasLevel2Tow(pHull, pOwner);
}

Boolean
ShipCanChunnelItself(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_ChunnelSelf) || ShipDoesChunneling(pShip);
}

Boolean
HullCanChunnelItself(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_ChunnelSelf, pOwner) || HullDoesChunneling(pHull, pOwner);
}

Boolean
ShipCanChunnelOthers(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_ChunnelOthers) || ShipDoesChunneling(pShip);
}

Boolean
HullCanChunnelOthers(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_ChunnelOthers, pOwner) || HullDoesChunneling(pHull, pOwner);
}

Boolean
ShipCanBeChunnelMate(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_ChunnelTarget) || ShipDoesChunneling(pShip);
}

Boolean
HullCanBeChunnelMate(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_ChunnelTarget, pOwner) || HullDoesChunneling(pHull, pOwner);
}

Boolean
ShipIsImmuneToPlanetAttacks(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_PlanetImmunity);
}

Boolean
HullIsImmuneToPlanetAttacks(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_PlanetImmunity, pOwner);
}

Boolean
ShipIsOreCondenser(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_OreCondenser);
}

Boolean
HullIsOreCondenser(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_OreCondenser, pOwner);
}

Boolean
ShipCanBoard(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_Boarding);
}

Boolean
HullCanBoard(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_Boarding, pOwner);
}

Boolean
ShipIsImmuneToAntiCloak(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_AntiCloakImmunity);
}

Boolean
HullIsImmuneToAntiCloak(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_AntiCloakImmunity, pOwner);
}

Boolean
ShipIsCrewAcademy(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_Academy);
}

Boolean
HullIsCrewAcademy(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_Academy, pOwner);
}

Boolean
ShipCanRepairOthers(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_Repairs);
}

Boolean
HullCanRepairOthers(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_Repairs, pOwner);
}

Boolean
ShipHasFullWeaponry(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_FullWeaponry);
}

Boolean
HullHasFullWeaponry(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_FullWeaponry, pOwner);
}

Boolean
ShipHasHardenedEngines(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_HardenedEngines);
}

Boolean
HullHasHardenedEngines(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_HardenedEngines, pOwner);
}

Boolean
ShipIsCommander(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_Commander);
}

Boolean
HullIsCommander(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_Commander, pOwner);
}

Boolean
ShipHasIonShield(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_IonShield);
}

Boolean
HullHasIonShield(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_IonShield, pOwner);
}

Boolean
ShipHasHardenedCloak(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_HardenedCloak);
}

Boolean
HullHasHardenedCloak(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_HardenedCloak, pOwner);
}

Boolean
ShipHasAdvancedAntiCloak(Uns16 pShip)
{
  return shipHasSpecial(pShip, SPC_AdvancedAntiCloak);
}

Boolean
HullHasAdvancedAntiCloak(Uns16 pHull, Uns16 pOwner)
{
  return hullHasSpecial(pHull, SPC_AdvancedAntiCloak, pOwner);
}

/* ----------------------------------------------------------------------- */

static Uns16 gHullSet[HULL_NR / 16 + 1];
static Boolean gHullSeen;
static Special_Struct gSpecial;
static Boolean gAssignToShip;

static Boolean doAssignment(const char *pName, char *pValue);

static Boolean
HullfuncAssign(const char* name, char* value, const char* line)
{
    (void) line;
    if (name && value)
        return doAssignment(name, value);
    else
        return True;
}

static void
initReader(void)
{
  struct Auxdata_Chunk* lChunk;

  sAllLevelsMask = (1 << (gConfigInfo->NumExperienceLevels+1)) - 1;

  gHullSeen = False;
  gSpecial.mFunction = SPC_NO_SPECIAL;
  gSpecial.mHostBit = -1;
  gSpecial.mLevelMask = sAllLevelsMask;
  gAssignToShip = 0;
  InitHullfunc();

  /* Process AUXDATA.HST numbers */
  if ((lChunk = GetAuxdataChunkById(aux_ShipExtraDef)) != 0) {
    Uns16 lData[2];
    char* lPtr = AuxdataChunkData(lChunk);
    unsigned lSize = AuxdataChunkSize(lChunk);
    unsigned lIndex = 0;
    while (lSize >= 4) {
      memcpy(lData, lPtr, 4);
      lPtr += 4;
      lSize -= 4;
      EndianSwap16(lData, 2);
      if (lData[0] < NumSpecials && lData[1] != 0) {
        Special_Struct lDev;
        lDev.mFunction  = lData[0];
        lDev.mHostBit   = lIndex;
        lDev.mLevelMask = lData[1] & sAllLevelsMask;
        getFunctionFromDef(&lDev);
      }
      ++lIndex;
    }
  }
}

void
SpecialReadHullfunc(const char *shiplistFile, const char *hullfuncFile)
{
    FILE* lFile;

    initReader();
    lFile = fopen(shiplistFile,"r");
    if (lFile) {
        ConfigFileReader(lFile, shiplistFile, "hullfunc", False, HullfuncAssign);
        fclose(lFile);
        return;
    }
    lFile = fopen(hullfuncFile,"r");
    if (lFile) {
        ConfigFileReader(lFile, hullfuncFile, "hullfunc", True, HullfuncAssign);
        fclose(lFile);
        return;
    }
    addDefaultSpecials();
}

void
ReadHullfunc(void)
{
    FILE* lFile;

    initReader();
    lFile = OpenInputFile(SHIPLIST_FILE, GAME_OR_ROOT_DIR | TEXT_MODE | NO_MISSING_ERROR);
    if (lFile) {
        ConfigFileReader(lFile, SHIPLIST_FILE, "hullfunc", False, HullfuncAssign);
        fclose(lFile);
        return;
    }
    lFile = OpenInputFile(HULLFUNC_FILE, GAME_OR_ROOT_DIR | TEXT_MODE | NO_MISSING_ERROR);
    if (lFile) {
        ConfigFileReader(lFile, HULLFUNC_FILE, "hullfunc", True, HullfuncAssign);
        fclose(lFile);
        return;
    }
    addDefaultSpecials();
}

static Boolean
doHullAssignment(char *pNames)
{
  Uns16 lLen;
  Uns16 lHull;
  char* lStr;

  memset(gHullSet, 0, sizeof(gHullSet));
  gHullSeen = True;
  while ((lStr = strtok(pNames, ",")) != 0) {
    pNames = 0; /* for next call to strtok */
    while (*lStr && isspace((unsigned char) *lStr))
      ++lStr;
    TrimTrailingWS(lStr);

    /* Might be wildcard? */
    if (strcmp(lStr, "*") == 0) {
      for (lHull = 0; lHull < sizeof(gHullSet) / sizeof(gHullSet[0]); ++lHull)
        gHullSet[lHull] = ~0;
      continue;
    }

    /* Might be hull number */
    if (isdigit((unsigned char) *lStr)) {
      int lStart, lEnd;
      switch (sscanf(lStr, "%d - %d", &lStart, &lEnd)) {
       case 1:
          lEnd = lStart;
          /* FALLTHROUGH */
       case 2:
          if (lStart <= 0 || lEnd > HULL_NR || lStart > lEnd)
            return False;
          for (lHull = lStart; lHull <= (unsigned) lEnd; ++lHull)
            gHullSet[lHull / 16] |= 1 << (lHull & 15);
          break;
       default:
          return False;
      }
    }

    /* It's a name. Look through hull names for a match */
    strupr(lStr);
    lLen = strlen(lStr);

    for (lHull = 1; lHull <= HULL_NR; lHull++) {
      if (memcmp(lStr, strupr((char *)HullName(lHull, 0)), lLen) EQ 0) {
        gHullSet[lHull / 16] |= 1 << (lHull & 15);
        break;
      }
    }
  }

  return True;
}

static Special_Def
ParseHullFunction(const char* pName)
{
  int lVal;
  size_t lLen;
  char lStr[64], lName[64];

  if (isdigit((unsigned char) *pName)) {
    lVal = atoi(pName);
    if (lVal < 0 OR lVal >= NumSpecials)
      return NumSpecials;
    return lVal;
  }

  strncpy(lName, pName, sizeof(lName));
  lName[sizeof(lName)-1] = 0;
  strupr(lName);

  lLen = strlen(lName);
  for (lVal = 0; lVal < NumSpecialNames; lVal++) {
    strcpy(lStr, gSpecialNames[lVal]);
    strupr(lStr);
    if (memcmp(lName, lStr, lLen) EQ 0)
      return lVal;
  }
  return NumSpecials;
}

static Boolean
doFuncAssignment(char *pName)
{
  gSpecial.mFunction = ParseHullFunction(pName);
  return True;
}

static Boolean
getPlayerMask(char *pValue, Boolean pMapRace,
              Uns16 *pRacesToRemove, Uns16* pRacesToAdd)
{
  /* Format is: expr expr expr expr

     where 'expr' is one of the following: +   all races enabled -   all
     races disabled +N  enable race N N   enable race N -N  disable race N

     Processing is sequential from left to right, so that + - + - + - + is
     simply equivalent to + */
  int lRace;
  Boolean lAdd;
  char *p;

  while ((p = strtok(pValue, " ,\t")) NEQ 0) {
    pValue = 0; /* For next call to strtok */

    lAdd = True;    /* So we can skip the leading '+' */

    if (strcmp(p, "*") == 0) {
      *pRacesToRemove = 0;
      *pRacesToAdd = ~0;
      continue;
    } else if (*p EQ '+') {
      /* Check for non-digit following '+' */
      if (! isdigit((unsigned char) *++p)) {
        *pRacesToRemove = 0;
        *pRacesToAdd = ~0;     /* Enable all */
        continue;
      }
    } else if (*p EQ '-') {
      lAdd = False;

      /* Check for non-digit following '-' */
      if (! isdigit((unsigned char) *++p)) {
        *pRacesToRemove = ~0;      /* Disable all */
        *pRacesToAdd = 0;
        continue;
      }
    }
    if (isdigit((unsigned char) *p)) {
      Uns16 lMaskHere = 0;

      lRace = atoi(p);

      /* Skip past race number */
      while (*p AND isdigit((unsigned char) *p)) p++;

      /* Map race through EffRace if so desired */
      if (pMapRace) {
        Uns16 lPlayer;

        /* Allow any race. If hosts want to shoot themselves
           in the feet, let them do it. */
        if (lRace < 1 OR lRace > 255)
          return False;
        for (lPlayer=1; lPlayer <= OLD_RACE_NR; lPlayer++)
          if (gConfigInfo->PlayerRace[lPlayer] EQ lRace)
            lMaskHere |= (1 << lPlayer);
      } else {
        if (lRace < 1 OR lRace > RACE_NR)
          return False;
        lMaskHere = 1 << lRace;
      }

      if (lAdd) {
        *pRacesToAdd |= lMaskHere;
        *pRacesToRemove &= ~lMaskHere;
      } else {
        *pRacesToRemove |= lMaskHere;
        *pRacesToAdd &= ~lMaskHere;
      }
    } else {
      return False;
    }
  }

  return True;
}

static Boolean
doAssignment(const char *pName, char *pValue)
{
  int ix;
  Uns16 lAdd, lRemove;
  Boolean lEffRaceMap;

  ix = ListMatch(pName, "Hull Function RacesAllowed PlayersAllowed Initialize AssignTo Level");
  switch (ix) {
  case 0:                      /* Hull */
    return doHullAssignment(pValue);

  case 1:                      /* Function */
    return doFuncAssignment(pValue);

  case 2:                      /* RacesAllowed */
  case 3:                      /* PlayersAllowed */
    if (!gHullSeen) {
      Error("No 'Hull' assignment seen yet");
      return False;
    }
    if (gSpecial.mFunction == SPC_NO_SPECIAL) {
      Error("No 'Function' assignment seen yet");
      return False;
    }
    if (gSpecial.mFunction == NumSpecials) {
      /* Forward-compatible parsing of an unknown function */
      return True;
    }
    lEffRaceMap = (ix EQ 2) ? True : False;
    lRemove = lAdd = 0;
    if (!getPlayerMask(pValue, lEffRaceMap, &lRemove, &lAdd)) {
      return False;
    }

    if (!gAssignToShip)
      for (ix = 1; ix <= HULL_NR; ++ix)
        if (gHullSet[ix / 16] & (1 << (ix & 15)))
          setSpecialRaces(ix, &gSpecial, lAdd, lRemove);
    gSpecial.mLevelMask = sAllLevelsMask;
    return True;

  case 4:                      /* Initialize */
    ix = ListMatch(pValue, "Clear Defaults");
    switch (ix) {
    case 0:                    /* Clear */
      clearAllSpecials();
      break;

    case 1:                    /* Defaults */
      clearAllSpecials();
      addDefaultSpecials();
      break;

    default:
      return False;
    }
    return True;
  case 5:                       /* AssignTo */
    gAssignToShip = (ListMatch(pValue, "Hull Ship") == 1);
    return True;

  case 6: { /* Level */
    int lMin, lMax, i;
    char lTmp;
    switch (sscanf(pValue, "%d - %d%c", &lMin, &lMax, &lTmp)) {
     case 1:
        lMax = 10;
        /* FALLTHROUGH */
     case 2:
        if (lMin < 0 || lMax > 10 || lMin > lMax)
          return False;
        gSpecial.mLevelMask = 0;
        for (i = lMin; i <= lMax; ++i)
          gSpecial.mLevelMask |= (1 << i);
        gSpecial.mLevelMask &= sAllLevelsMask;
        break;
     default:
        return False;
    }
    return True;
  }

  default:
    return False;
  }
}

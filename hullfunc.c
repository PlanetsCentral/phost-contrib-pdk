
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

#ifndef CBUILDER
#include <ctype.h>
#endif

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

  NumSpecials,

  SPC_NO_SPECIAL                /* Marker */
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
  "Level2Tow"
};

#define NumSpecialNames (sizeof(gSpecialNames)/sizeof(gSpecialNames[0]))

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
  {105, SPC_Alchemy}            /* Merlin */
};

#define NumDefaultSpecials (sizeof(gDefaultSpecials)/sizeof(gDefaultSpecials[0]))

static void
ShutdownHullfunc(void)
{
  MemFree(gHullfunc);
  gHullfunc = 0;
}

static void
InitHullfunc(void)
{
  if (gHullfunc)
    return;                     /* Already initialized */

  passert(NumSpecialNames EQ NumSpecials);
  gHullfunc = (Uns16 **) MemCalloc(HULL_NR + 1, sizeof(Uns16 *));

  RegisterCleanupFunction(ShutdownHullfunc);
}

static void
setSpecialRaces(Uns16 pHull, Special_Def pSpecial, Uns16 pRaceMask)
{
  /* pSpecial == NumSpecials means we got a special we don't know. 
     Simply ignore it. */
  if (pSpecial == NumSpecials)
      return;

  passert(pHull >= 1 AND pHull <= HULL_NR);
  passert(pSpecial >= 0 AND pSpecial < NumSpecials);
  passert(gHullfunc);

  if (gHullfunc[pHull] EQ 0) {
    gHullfunc[pHull] = (Uns16 *) MemCalloc(NumSpecials, sizeof(Uns16));
  }

  gHullfunc[pHull][pSpecial] = pRaceMask;
}

static void
clearAllSpecials(void)
{
  Uns16 lHull;
  Uns16 **lSpecials;

  passert(gHullfunc);

  for (lHull = 1, lSpecials = gHullfunc; lHull <= HULL_NR;
        lHull++, lSpecials++) {
    MemFree(*lSpecials);
    *lSpecials = 0;
  }
}

static void
addDefaultSpecials(void)
{
  int i;
  Uns16 lMask;
  SpecialInfo_Struct *lInfo;

  passert(gHullfunc);

  /* Set a mask that enables all players for any special */
  lMask = ~0;

  for (i = 0, lInfo = gDefaultSpecials; i < NumDefaultSpecials; i++, lInfo++) {
    setSpecialRaces(lInfo->mHull, lInfo->mSpecial, lMask);
  }
}

static Boolean
shipHasSpecial(Uns16 pShip, Special_Def pSpecial)
{
  /* NOTE: Do NOT map through EffRace() */
  Uns16 lOwner;
  Uns16 *lPtr;
  struct Auxdata_Chunk* lChunk;

  InitHullfunc();

  lChunk = GetAuxdataChunkById(aux_ShipSpecial);
  if (pSpecial < 64 && AuxdataChunkSize(lChunk) >= 8 * pShip) {
      unsigned char* lEle = AuxdataChunkData(lChunk);
      if (lEle[8 * (pShip-1) + pSpecial / 8] & (1 << (pSpecial & 7)))
          return True;
  }

  lOwner = ShipOwner(pShip);
  lPtr = gHullfunc[ShipHull(pShip)];

  if (lPtr) {
    return (lPtr[pSpecial] & (1 << lOwner)) ? True : False;
  }

  return False;
}

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
  return shipHasSpecial(pShip, SPC_AntiCloak);
}

Boolean
HullIsAntiCloaking(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_AntiCloak, Owner);
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
  return shipHasSpecial(pShip, SPC_Cloak) OR ShipHasAdvancedCloak(pShip);
}

Boolean
HullCanCloak(Uns16 pHull, Uns16 Owner)
{
  /* NOTE: Added by Maurits 2004-07-30 */
  return hullHasSpecial(pHull, SPC_Cloak, Owner) \
      OR HullHasAdvancedCloak(pHull, Owner);;
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

/* ----------------------------------------------------------------------- */

static Uns16 gHull = 0;
static Special_Def gSpecial = SPC_NO_SPECIAL;

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

void
SpecialReadHullfunc(const char *shiplistFile, const char *hullfuncFile)
{
    FILE* lFile;

    gHull = 0;
    gSpecial = SPC_NO_SPECIAL;

    InitHullfunc();
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

    gHull = 0;
    gSpecial = SPC_NO_SPECIAL;

    InitHullfunc();
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
doHullAssignment(char *pName)
{
  Uns16 lLen;

  /* pName can be either a hull number or hull name */
  if (isdigit(*pName)) {
    gHull = (Uns16) atoi(pName);
    if (gHull < 1 OR gHull > HULL_NR) {
      gHull = 0;
      return False;
    }

    return True;
  }

  /* It's a name. Look through hull names for a match */
  strupr(pName);
  lLen = strlen(pName);

  for (gHull = 1; gHull <= HULL_NR; gHull++) {
    if (memcmp(pName, strupr((char *) HullName(gHull, 0)), lLen) EQ 0)
      return True;
  }

  gHull = 0;
  return False;
}

static Boolean
doFuncAssignment(char *pName)
{
  int lVal;
  size_t lLen;
  char lStr[64];

  /* pName can be either a special number or special name */
  if (isdigit(*pName)) {
    lVal = atoi(pName);
    if (lVal < 0 OR lVal >= NumSpecials) {
        /* for forward compatibility, just ignore specials we can't
           handle. */
        gSpecial = NumSpecials;
        return True;
    }

    gSpecial = (Special_Def) lVal;
    return True;
  }

  strupr(pName);
  lLen = strlen(pName);
  for (lVal = 0; lVal < NumSpecialNames; lVal++) {
    strcpy(lStr, gSpecialNames[lVal]);
    strupr(lStr);
    if (memcmp(pName, lStr, lLen) EQ 0) {
      gSpecial = (Special_Def) lVal;
      return True;
    }
  }

  /* for forward compatibility, just ignore specials we can't handle. */
  gSpecial = NumSpecialNames;
  return True;
}

static Boolean
getPlayerMask(char *pValue, Boolean pMapRace, Uns16 * pRaceMaskPtr)
{
  /* Format is: expr expr expr expr

     where 'expr' is one of the following: +   all races enabled -   all
     races disabled +N  enable race N N   enable race N -N  disable race N

     Processing is sequential from left to right, so that + - + - + - + is
     simply equivalent to + */
  Uns16 lMask = 0;
  int lRace;
  Boolean lAdd;
  char *p;

  while ((p = strtok(pValue, " ,\t")) NEQ 0) {
    pValue = 0;                 /* For next call to strtok */

    lAdd = True;                /* So we can skip the leading '+' */

    if (*p EQ '+') {
      /* Check for non-digit following '+' */
      if (!isdigit(*++p)) {
        lMask = ~0;             /* Enable all */
        continue;
      }
    }
    else if (*p EQ '-') {
      lAdd = False;

      /* Check for non-digit following '-' */
      if (!isdigit(*++p)) {
        lMask = 0;              /* Disable all */
        continue;
      }
    }
    if (isdigit(*p)) {
      lRace = atoi(p);

      /* Skip past race number */
      while (*p AND isdigit(*p))
        p++;

      /* Allow RACE_NR to allow for aliens or non-race players */
      if (lRace < 1 OR lRace > RACE_NR)
        return False;

      /* Map race through EffRace if so desired */
      if (pMapRace) {
        Uns16 lPlayer;

        for (lPlayer = 1; lPlayer <= OLD_RACE_NR; lPlayer++) {
          if (gConfigInfo->PlayerRace[lPlayer] EQ lRace) {
            if (lAdd) {
              lMask |= (1 << lPlayer);
            }
            else {
              lMask &= ~(1 << lPlayer);
            }
          }
        }
      }
      else {
        if (lAdd) {
          lMask |= (1 << lRace);
        }
        else {
          lMask &= ~(1 << lRace);
        }
      }
    }
    else {
      return False;
    }
  }

  *pRaceMaskPtr = lMask;

  return True;
}

static Boolean
doAssignment(const char *pName, char *pValue)
{
  int ix;
  Uns16 lRaceMask;
  Boolean lEffRaceMap;

  ix =
        ListMatch(pName,
        "Hull Function RacesAllowed PlayersAllowed Initialize AssignTo");
  switch (ix) {
  case 0:                      /* Hull */
    return doHullAssignment(pValue);

  case 1:                      /* Function */
    return doFuncAssignment(pValue);

  case 2:                      /* RacesAllowed */
  case 3:                      /* PlayersAllowed */
    if (gHull == 0) {
        Error("No 'Hull' assignment seen yet");
        return False;
    }
    if (gSpecial == SPC_NO_SPECIAL) {
        Error("No 'Function' assignment seen yet");
        return False;
    }
    lEffRaceMap = (ix EQ 2) ? True : False;
    if (!getPlayerMask(pValue, lEffRaceMap, &lRaceMask))
      return False;

    setSpecialRaces(gHull, gSpecial, lRaceMask);
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
    /* FIXME? */
    return True;

  default:
    return False;
  }
}

/*************************************************************
  $HISTORY:$
**************************************************************/

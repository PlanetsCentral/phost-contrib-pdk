
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

/******************************************************************
 *                  *
 *  Module:   Central Data Structures        *
 *  Project:   Portable Host           *
 *  File Name:   vgapdata.c           *
 *  Programmer:  Thomas Voigt           *
 *  Create Date: October 18, 1994          *
 *  Description: This module provides accessors and update    *
 *      routines for the central data structures such   *
 *      as planets, ships, bases, mines, etc.     *
 *  Functions:               *
 *  Change Record:              *
 *                  *
 *                  *
 ******************************************************************/

#include "phostpdk.h"
#include "private.h"

static const char *PLANET_FILE = "pdata.hst";
static Planet_Struct *gAllPlanets = 0;

extern XYData_Struct *gXYPlanPtr;

static void
FreePlanets(void)
{
  MemFree(gAllPlanets);
  gAllPlanets = 0;
}

static void
InitPlanets(void)
{
  if (gAllPlanets EQ 0) {
    gAllPlanets =
          (Planet_Struct *) MemCalloc(PLANET_NR + 1, sizeof(Planet_Struct));

    RegisterCleanupFunction(FreePlanets);
  }
}

/*--------------------------------------------------------------------------
Name   : GetPlanet(), PutPlanet()
Purpose   : returns/stores the whole planetary structure
Arguments  : planetary ID, planet structure (PutPlanet())
Algorithm  : currently it returns/stores just the structure in gAllPlanets.
      Future DOS versions (where we'll have to deal
      with memory problems) will use EMS/XMS
Return value : If the argument is out of range then Planet_Struct returns
      a zero-filled structure.
--------------------------------------------------------------------------*/

Planet_Struct *
GetPlanet(Uns16 pID)
{
  InitPlanets();

/* _All_ planets should exist */
  passert((pID > 0) AND(pID <= PLANET_NR) AND(gAllPlanets[pID].Id NEQ 0));

  return gAllPlanets + pID;
}

void
PutPlanet(Uns16 pID, Planet_Struct * pPlanet)
{
  InitPlanets();

  passert((pID > 0) AND(pID <= PLANET_NR) AND(pPlanet NEQ NULL));

  memmove(gAllPlanets + pID, pPlanet, sizeof(Planet_Struct));
}

Boolean
IsPlanetExist(Uns16 pID)
{
  InitPlanets();

  if ((pID > 0) AND(pID <= PLANET_NR) AND(gAllPlanets[pID].Id NEQ 0)
        AND IsPointOnWrapMap(gXYPlanPtr[pID].X, gXYPlanPtr[pID].Y))
    return True;
  else
    return False;
}

RaceType_Def
PlanetOwner(Uns16 pID)
{
  return (GetPlanet(pID)->Owner);
}

Uns16
PlanetMines(Uns16 pID)
{
  return (GetPlanet(pID)->Mines);
}

Uns16
PlanetFactories(Uns16 pID)
{
  return (GetPlanet(pID)->Factories);
}

Uns16
PlanetDefense(Uns16 pID)
{
  return (GetPlanet(pID)->Defense);
}

Uns32
PlanetCargo(Uns16 pID, CargoType_Def pType)
{
  Uns32 lRetVal = GetPlanet(pID)->Cargo[pType];

  return (pType EQ COLONISTS) ? lRetVal * 100UL : lRetVal;
}

Uns32
PlanetCore(Uns16 pID, CargoType_Def pType)
{
  if (pType < 4)
    return (GetPlanet(pID)->Core[pType]);
  else
    return (0);
}

Uns16
PlanetDensity(Uns16 pID, CargoType_Def pType)
{
  if (pType < 4)
    return (GetPlanet(pID)->Density[pType]);
  else
    return (0);
}

Uns16
PlanetColTax(Uns16 pID)
{
  return (GetPlanet(pID)->ColTax);
}

Uns16
PlanetNatTax(Uns16 pID)
{
  return (GetPlanet(pID)->NatTax);
}

Int16
PlanetColHappy(Uns16 pID)
{
  return (GetPlanet(pID)->ColHappy);
}

Int16
PlanetNatHappy(Uns16 pID)
{
  return (GetPlanet(pID)->NatHappy);
}

NativeType_Def
PlanetNatType(Uns16 pID)
{
  return (GetPlanet(pID)->NativeType);
}

NativeGovm_Def
PlanetNatGovm(Uns16 pID)
{
  return (GetPlanet(pID)->Govm);
}

Uns32
PlanetNativePopulation(Uns16 pID)
{
  return (GetPlanet(pID)->NativePopulation * 100UL);
}

Uns16
PlanetTemp(Uns16 pID)
{
  return (100 - GetPlanet(pID)->Temp);
}

Boolean
PlanetBuildBase(Uns16 pID)
{
  return (GetPlanet(pID)->BuildBase);
}

Boolean
PlanetHasNatives(Uns16 pPlanet)
{
  return (Boolean) ((PlanetNatType(pPlanet) NEQ NoNatives)
        AND(PlanetNativePopulation(pPlanet) > 0));
}

void
PutPlanetOwner(Uns16 pID, RaceType_Def pOwner)
{
  GetPlanet(pID)->Owner = pOwner;
}

void
PutPlanetFC(Uns16 pID, char *pFCode)
{
  Planet_Struct *lPlanet = GetPlanet(pID);

  passert(pFCode NEQ NULL);
  passert(strlen(pFCode) <= 3);
  memset(lPlanet->FriendlyCode, ' ', 3);
  strncpy(lPlanet->FriendlyCode, pFCode, 3);
}

void
PutPlanetMines(Uns16 pID, Uns16 pMines)
{
  GetPlanet(pID)->Mines = pMines;
}

void
PutPlanetFactories(Uns16 pID, Uns16 pFactories)
{
  GetPlanet(pID)->Factories = pFactories;
}

void
PutPlanetDefense(Uns16 pID, Uns16 pDefense)
{
  GetPlanet(pID)->Defense = pDefense;
}

void
PutPlanetCargo(Uns16 pID, CargoType_Def pType, Uns32 pAmount)
{
  if (pType EQ COLONISTS)
    pAmount /= 100;
  GetPlanet(pID)->Cargo[pType] = pAmount;
}

void
PutPlanetCore(Uns16 pID, CargoType_Def pType, Uns32 pAmount)
{
  passert(pType < 4);
  GetPlanet(pID)->Core[pType] = pAmount;
}

void
PutPlanetDensity(Uns16 pID, CargoType_Def pType, Uns16 pDensity)
{
  passert((pType < 4) AND(pDensity > 0) AND(pDensity <= MAX_DENSITY));
  GetPlanet(pID)->Density[pType] = pDensity;
}

void
PutPlanetColTax(Uns16 pID, Uns16 pTax)
{
  passert(pTax <= 100);
  GetPlanet(pID)->ColTax = pTax;
}

void
PutPlanetNatTax(Uns16 pID, Uns16 pTax)
{
  passert(pTax <= 100);
  GetPlanet(pID)->NatTax = pTax;
}

void
PutPlanetColHappy(Uns16 pID, Int16 pHappy)
{
  Int16 lHappy;

  lHappy = min(pHappy, MAX_HAPPY);
  lHappy = max(lHappy, MIN_HAPPY);
  GetPlanet(pID)->ColHappy = lHappy;
}

void
PutPlanetNatHappy(Uns16 pID, Int16 pHappy)
{
  Int16 lHappy;

  lHappy = min(pHappy, MAX_HAPPY);
  lHappy = max(lHappy, MIN_HAPPY);
  GetPlanet(pID)->NatHappy = lHappy;
}

void
PutPlanetNatGovm(Uns16 pID, NativeGovm_Def pGovm)
{
  GetPlanet(pID)->Govm = pGovm;
}

void
PutPlanetNativePopulation(Uns16 pID, Uns32 pPopulation)
{
  GetPlanet(pID)->NativePopulation = pPopulation / 100;
}

void
PutPlanetNatType(Uns16 pID, NativeType_Def pType)
{
  GetPlanet(pID)->NativeType = pType;
}

void
PutPlanetTemp(Uns16 pID, Uns16 pTemp)
{
  GetPlanet(pID)->Temp = 100 - pTemp;
}

void
PutPlanetBuildBase(Uns16 pID, Boolean pBuild)
{
  GetPlanet(pID)->BuildBase = pBuild;
}

void
ChangePlanetOwner(Uns16 pID, RaceType_Def pNewOwner)
{
  PutPlanetOwner(pID, pNewOwner);
  if (IsBasePresent(pID)) {
    ClearBaseBuildOrder(pID);
    if (pNewOwner NEQ NoOwner)
      PutBaseOwner(pID, pNewOwner);
    ClearBaseHulls(pID);
  }
}

const char *
PlanetFCode(Uns16 pID, char *pFCode)
{
  static char lFC[4];
  char *lPtr = pFCode;

  if (lPtr EQ 0)
    lPtr = lFC;
  strncpy(lPtr, GetPlanet(pID)->FriendlyCode, 3);
  lPtr[3] = 0;

  return lPtr;
}

/* NEW */
const char *
PlanetTempString(Uns16 pPlanet)
{
  Uns16 lTemp = PlanetTemp(pPlanet);

  if (lTemp < 15)
    return "Arctic World";
  else if (lTemp < 40)
    return "Temperate - cool";
  else if (lTemp < 65)
    return "Temperate - warm";
  else if (lTemp < 85)
    return "Tropical World";
  else
    return "Desert World";
}

IO_Def
Read_Planets_File(Int16 * pControl)
{
  FILE *lPlanetFile;
  Int16 i;
  IO_Def lError = IO_SUCCESS;
  Planet_Struct lPlanet;

  passert(pControl NEQ NULL);

  if ((lPlanetFile =
              OpenInputFile(PLANET_FILE,
             GAME_DIR_ONLY | NO_MISSING_ERROR)) NEQ NULL) {
    if (1 NEQ fread(pControl, sizeof(Uns16), 1, lPlanetFile)) { /* first 2
                                                                   bytes in
                                                                   pdata.hst */
      Error("Can't read file '%s'", PLANET_FILE);
      lError = IO_FAILURE;
    }

    for (i = 1; i <= PLANET_NR; i++) {
#ifdef __MSDOS__
      if (fread(&lPlanet, sizeof(Planet_Struct), 1, lPlanetFile) NEQ 1) {
#else
      memset(&lPlanet, 0, sizeof(lPlanet));
      if (!DOSReadStruct(PlanetStruct_Convert, NumPlanetStruct_Convert,
                  &lPlanet, lPlanetFile)) {
#endif
        Error("Can't read file '%s'", PLANET_FILE);
        lError = IO_FAILURE;    /* i/o error  */
        break;
      }
      else {
        if (lPlanet.Id NEQ 0)   /* NEW */
          PutPlanet(i, &lPlanet);
      }
    }

    fclose(lPlanetFile);
  }
  else
    lError = IO_FAILURE;

  return (lError);
}

IO_Def
Write_Planets_File(Int16 pControl)
{
  FILE *lPlanetFile;
  Int16 i;
  IO_Def lError = IO_SUCCESS;
  Planet_Struct lPlanet;        /* NEW */

  if (gAllPlanets EQ 0) {
    Error("Cannot write planets file without first reading it.\n");
    return IO_FAILURE;
  }

  if ((lPlanetFile = OpenOutputFile(PLANET_FILE, GAME_DIR_ONLY)) NEQ NULL) {
    if (1 NEQ fwrite(&pControl, sizeof(Uns16), 1, lPlanetFile)) {
      Error("Can't write to file '%s'", PLANET_FILE);
      lError = IO_FAILURE;
    }

    memset(&lPlanet, 0, sizeof(lPlanet)); /* NEW */
    for (i = 1; i <= PLANET_NR; i++)
#ifdef __MSDOS__
      if (1 NEQ fwrite((IsPlanetExist(i) ? GetPlanet(i) : &lPlanet),
                  sizeof(Planet_Struct), 1, lPlanetFile)) {
#else
      if (!DOSWriteStruct(PlanetStruct_Convert, NumPlanetStruct_Convert,
                  IsPlanetExist(i) ? GetPlanet(i) : &lPlanet, lPlanetFile)) {
#endif
        Error("Can't write to file '%s'", PLANET_FILE);
        lError = IO_FAILURE;
        break;
      }

    fclose(lPlanetFile);
  }
  else
    lError = IO_FAILURE;
  return (lError);
}

void
InitializePlanets(void)
{
  Uns16 lPlanet;

  InitPlanets();

  memset(gAllPlanets, 0, (PLANET_NR + 1) * sizeof(Planet_Struct));

  /* Planets with positions at (0,0) in XYPLAN.DAT are deemed nonexistant */

  for (lPlanet = 1; lPlanet <= PLANET_NR; lPlanet++) {
    if (PlanetLocationX(lPlanet) NEQ 0 OR PlanetLocationY(lPlanet) NEQ 0) {
      gAllPlanets[lPlanet].Id = lPlanet;
    }
  }
}

/*************************************************************
  $HISTORY:$
**************************************************************/


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

static const char *SHIP_FILE = "ship.hst";
static Ship_Struct *gAllShips = 0;

static void
FreeShips(void)
{
  MemFree(gAllShips);
  gAllShips = 0;
}

static void
InitShips(void)
{
  if (gAllShips EQ 0) {
    gAllShips = (Ship_Struct *) MemCalloc(SHIP_NR + 1, sizeof(Ship_Struct));

    RegisterCleanupFunction(FreeShips);
  }
}

Ship_Struct *
GetShip(Uns16 pID)
{
  InitShips();

  passert((pID > 0) AND(pID <= SHIP_NR));
  passert(gAllShips[pID].Owner NEQ 0);

  return gAllShips + pID;
}

void
PutShip(Uns16 pID, Ship_Struct * pShip)
{
  InitShips();

  passert((pID > 0) AND(pID <= SHIP_NR) AND(pShip NEQ NULL));

  memmove(gAllShips + pID, pShip, sizeof(Ship_Struct));
}

void
DeleteShip(Uns16 pID)
{
  InitShips();

  passert((pID > 0) AND(pID <= SHIP_NR) AND(gAllShips[pID].Owner NEQ 0));

  gAllShips[pID].Owner = NoOwner;
}

Boolean
IsShipExist(Uns16 pID)
{
  InitShips();

  if ((pID > 0) AND(pID <= SHIP_NR) AND(gAllShips[pID].Owner NEQ 0))
    return (True);
  else
    return (False);
}

RaceType_Def
ShipOwner(Uns16 pID)
{
  return (GetShip(pID)->Owner);
}

Uns16
ShipSpeed(Uns16 pID)
{
  return (GetShip(pID)->Speed);
}

static Uns16
ShipLocation(Uns16 pID, XYComponent_Def pIndex)
{
  passert((pIndex EQ 0) OR(pIndex EQ 1));
  return (GetShip(pID)->Location[pIndex]);
}

Uns16
ShipLocationX(Uns16 pID)
{
  return (GetShip(pID)->Location[0]);
}

Uns16
ShipLocationY(Uns16 pID)
{
  return (GetShip(pID)->Location[1]);
}

static Uns16
ShipWaypoint(Uns16 pID, XYComponent_Def pIndex)
{
  passert((pIndex EQ 0) OR(pIndex EQ 1));

  /* NEW */
  return (GetShip(pID)->Waypoint[pIndex] + (Int16) ShipLocation(pID, pIndex));
}

Uns16
ShipWaypointX(Uns16 pID)
{
  /* NEW */
  return (GetShip(pID)->Waypoint[0] + (Int16) ShipLocationX(pID));
}

Uns16
ShipWaypointY(Uns16 pID)
{
  /* NEW */
  return (GetShip(pID)->Waypoint[1] + (Int16) ShipLocationY(pID));
}

Int16
ShipRelWaypointX(Uns16 pID)
{
  return GetShip(pID)->Waypoint[0];
}

Int16
ShipRelWaypointY(Uns16 pID)
{
  return GetShip(pID)->Waypoint[1];
}

Uns16
ShipHull(Uns16 pID)
{
  return (GetShip(pID)->Hull);
}

Uns16
ShipEngine(Uns16 pID)
{
  return (GetShip(pID)->Engine);
}

Uns16
ShipBeamType(Uns16 pID)
{
  return (GetShip(pID)->BeamType);
}

Uns16
ShipBeamNumber(Uns16 pID)
{
  Ship_Struct *lPtr = GetShip(pID);

  return (lPtr->BeamType EQ 0 ? 0 : lPtr->BeamNumber);
}

Uns16
ShipTorpType(Uns16 pID)
{
  return (GetShip(pID)->TorpType);
}

Uns16
ShipTubeNumber(Uns16 pID)
{
  Ship_Struct *lPtr = GetShip(pID);

  return (lPtr->TorpType EQ 0 ? 0 : lPtr->TubeNumber);
}

Uns16
ShipBays(Uns16 pID)
{
  return (GetShip(pID)->Bays);
}

Uns16
ShipAmmunition(Uns16 pID)
{
  return (GetShip(pID)->Ammunition);
}

ShipMission_Def
ShipMission(Uns16 pID)
{
  return (GetShip(pID)->Mission);
}

RaceType_Def
ShipEnemy(Uns16 pID)
{
  return (GetShip(pID)->Enemy);
}

Uns16
ShipTowTarget(Uns16 pID)
{
  return (GetShip(pID)->TowTarget);
}

Uns16
ShipInterceptTarget(Uns16 pID)
{
  return (GetShip(pID)->InterceptTarget);
}

Uns16
ShipDamage(Uns16 pID)
{
  return (GetShip(pID)->Damage);
}

Uns16
ShipCrew(Uns16 pID)
{
  return (GetShip(pID)->Crew);
}

Uns16
ShipDump(Uns16 pID, CargoType_Def pType)
{
  if (pType EQ CREDITS)
    return 0;
  return (GetShip(pID)->Dump[pType]);
}

Uns16
ShipDumpPlanet(Uns16 pID)
{
  return (GetShip(pID)->DumpPlanet);
}

Uns16
ShipTransfer(Uns16 pID, CargoType_Def pType)
{
  if (pType EQ CREDITS)
    return 0;
  return (GetShip(pID)->Transfer[pType]);
}

Uns16
ShipTransferShip(Uns16 pID)
{
  return (GetShip(pID)->TransferShip);
}

void
PutShipOwner(Uns16 pID, RaceType_Def pOwner)
{
  GetShip(pID)->Owner = pOwner;
}

void
PutShipFC(Uns16 pID, char *pFCode)
{
  Ship_Struct *lShip = GetShip(pID);

  passert(pFCode NEQ NULL);
  passert(strlen(pFCode) <= 3);

  memset(lShip->FriendlyCode, ' ', 3);
  strncpy(lShip->FriendlyCode, pFCode, 3);
}

void
PutShipSpeed(Uns16 pID, Uns16 pSpeed)
{
  passert(pSpeed <= MAX_SPEED);
  GetShip(pID)->Speed = pSpeed;
}

static void
PutShipWaypoint(Uns16 pID, XYComponent_Def pIndex, Uns16 pWaypoint)
{
  passert((pIndex EQ 0) OR(pIndex EQ 1));

  /* NEW */
  GetShip(pID)->Waypoint[pIndex] =
        (Int16) pWaypoint - (Int16) ShipLocation(pID, pIndex);
}

void
PutShipWaypointX(Uns16 pID, Uns16 pWaypoint)
{
  /* NEW */
  GetShip(pID)->Waypoint[0] = (Int16) pWaypoint - (Int16) ShipLocationX(pID);
}

void
PutShipWaypointY(Uns16 pID, Uns16 pWaypoint)
{
  /* NEW */
  GetShip(pID)->Waypoint[1] = (Int16) pWaypoint - (Int16) ShipLocationY(pID);
}

void
PutShipRelWaypointX(Uns16 pID, Int16 pRel)
{
  GetShip(pID)->Waypoint[0] = WrapDistX(pRel);
}

void
PutShipRelWaypointY(Uns16 pID, Int16 pRel)
{
  GetShip(pID)->Waypoint[1] = WrapDistY(pRel);
}

void
PutShipLocationX(Uns16 pID, Int16 pLocation)
{
  Int16 lWaypoint;

  if (pLocation > MAX_COORDINATE)
    pLocation = MAX_COORDINATE;

  lWaypoint = ShipWaypointX(pID);

  GetShip(pID)->Location[0] = pLocation = WrapMapX(pLocation);
  PutShipRelWaypointX(pID, lWaypoint - pLocation);
}

void
PutShipLocationY(Uns16 pID, Int16 pLocation)
{
  Int16 lWaypoint;

  if (pLocation > MAX_COORDINATE)
    pLocation = MAX_COORDINATE;

  lWaypoint = ShipWaypointY(pID);

  GetShip(pID)->Location[1] = pLocation = WrapMapY(pLocation);
  PutShipRelWaypointY(pID, lWaypoint - pLocation);
}

void
PutShipHull(Uns16 pID, Uns16 pHull)
{
  passert((pHull > 0) AND(pHull <= HULL_NR));
  GetShip(pID)->Hull = pHull;
}

void
PutShipEngine(Uns16 pID, Uns16 pEngine)
{
  passert((pEngine > 0) AND(pEngine <= ENGINE_NR));
  GetShip(pID)->Engine = pEngine;
}

void
PutShipBeamType(Uns16 pID, Uns16 pBeamType)
{
  passert((pBeamType > 0) AND(pBeamType <= BEAM_NR));
  GetShip(pID)->BeamType = pBeamType;
}

void
PutShipBeamNumber(Uns16 pID, Uns16 pBeamNumber)
{
  passert(pBeamNumber <= MAX_BEAMS);
  GetShip(pID)->BeamNumber = pBeamNumber;
}

void
PutShipTorpType(Uns16 pID, Uns16 pTorpType)
{
  passert(pTorpType <= TORP_NR);
  GetShip(pID)->TorpType = pTorpType;
}

void
PutShipTubeNumber(Uns16 pID, Uns16 pTubeNumber)
{
  passert(pTubeNumber <= MAX_TUBES);
  GetShip(pID)->TubeNumber = pTubeNumber;
}

void
PutShipBays(Uns16 pID, Uns16 pBays)
{
  passert(pBays <= MAX_BAYS);
  GetShip(pID)->Bays = pBays;
}

void
PutShipAmmunition(Uns16 pID, Uns16 pAmmunition)
{
  GetShip(pID)->Ammunition = pAmmunition;
}

void
PutShipMission(Uns16 pID, ShipMission_Def pMission)
{
  GetShip(pID)->Mission = pMission;
}

void
PutShipEnemy(Uns16 pID, RaceType_Def pEnemy)
{
  GetShip(pID)->Enemy = pEnemy;
}

void
PutShipTowTarget(Uns16 pID, Uns16 pTarget)
{
  passert(pTarget <= SHIP_NR);
  GetShip(pID)->TowTarget = pTarget;
}

void
PutShipInterceptTarget(Uns16 pID, Uns16 pTarget)
{
  passert(pTarget <= SHIP_NR);
  GetShip(pID)->InterceptTarget = pTarget;
}

void
PutShipDamage(Uns16 pID, Uns16 pDamage)
{
  passert(pDamage <= 150);
  GetShip(pID)->Damage = pDamage;
}

void
PutShipCrew(Uns16 pID, Uns16 pCrew)
{
  GetShip(pID)->Crew = pCrew;
}

void
PutShipDump(Uns16 pID, CargoType_Def pType, Uns16 pCargo)
{
  if (pType NEQ CREDITS)
    GetShip(pID)->Dump[pType] = pCargo;
}

void
PutShipDumpPlanet(Uns16 pID, Uns16 pPlanet)
{
  passert(pPlanet <= PLANET_NR);
  GetShip(pID)->DumpPlanet = pPlanet;
}

void
PutShipTransfer(Uns16 pID, CargoType_Def pType, Uns16 pCargo)
{
  if (pType NEQ CREDITS)
    GetShip(pID)->Transfer[pType] = pCargo;
}

void
PutShipTransferShip(Uns16 pID, Uns16 pShip)
{
  passert(pShip <= SHIP_NR);
  GetShip(pID)->TransferShip = pShip;
}

/* the pName parameter must point to a storage area at least 21 bytes in length */
char *
ShipName(Uns16 pID, char *pName)
{
  static char lName[21];
  char *lPtr = pName;

  if (lPtr EQ NULL)
    lPtr = lName;               /* NEW */
  memcpy(lPtr, GetShip(pID)->Name, 20);
  lPtr[20] = 0;

  return (lPtr);
}

const char *
ShipFC(Uns16 pID, char *pFCode)
{
  static char lFC[4];
  char *lPtr = pFCode;

  if (lPtr EQ 0)
    lPtr = lFC;
  strncpy(lPtr, GetShip(pID)->FriendlyCode, 3);

  lPtr[3] = 0;
  return lPtr;
}

Uns16
ShipCargo(Uns16 pID, CargoType_Def pType)
{
  switch (pType) {
  case NEUTRONIUM:
  case TRITANIUM:
  case DURANIUM:
  case MOLYBDENUM:
    return (GetShip(pID)->Cargo[pType]);
  case SUPPLIES:
    return (GetShip(pID)->Cargo[4]);
  case CREDITS:
    return (GetShip(pID)->Credits);
  case COLONISTS:
    return (GetShip(pID)->Colonists); /* CLANS!!! */
  default:
    passert(0);
  }
  return (0);
}

void
PutShipCargo(Uns16 pID, CargoType_Def pType, Uns16 pCargo)
{
  switch (pType) {
  case NEUTRONIUM:
  case TRITANIUM:
  case DURANIUM:
  case MOLYBDENUM:
    GetShip(pID)->Cargo[pType] = pCargo;
    break;
  case SUPPLIES:
    GetShip(pID)->Cargo[4] = pCargo;
    break;
  case CREDITS:
    GetShip(pID)->Credits = pCargo;
    break;
  case COLONISTS:
    GetShip(pID)->Colonists = pCargo; /* CLANS!!! */
    break;
  }
}

void
PutShipName(Uns16 pID, const char *pName)
{
  char ShipName[21];
  int i;

  passert(pName NEQ NULL);
  memcpy(ShipName, pName, 20);
  ShipName[20] = 0;
  for (i = strlen(ShipName); i < 20; i++)
    ShipName[i] = ' ';

  memcpy(GetShip(pID)->Name, ShipName, 20); /* NEW */
}

/* This is used to find a new ship slot in which to build a ship. The routine
   returns 0 if all ship slots are full. We don't fill in any ship details
   other than the ID and owner. The owner indicates that the ship exists. */

/* NEW */
Uns16
CreateShip(RaceType_Def pOwner)
{
  Uns16 lShip;
  Ship_Struct *lShipPtr;

  for (lShip = 1; lShip <= SHIP_NR; lShip++) {
    if (IsShipExist(lShip))
      continue;

    lShipPtr = gAllShips + lShip;
    memset(lShipPtr, 0, sizeof(Ship_Struct));

    lShipPtr->Id = lShip;
    lShipPtr->Owner = pOwner;
    return lShip;
  }
  return 0;
}

IO_Def
Read_Ships_File(Int16 * pControl)
{
  FILE *lShipFile;
  Int16 i;
  IO_Def lError = IO_SUCCESS;
  Ship_Struct lShip;

  passert(pControl NEQ NULL);

  if ((lShipFile =
              OpenInputFile(SHIP_FILE,
               GAME_DIR_ONLY | NO_MISSING_ERROR)) NEQ NULL) {
    if (1 NEQ fread(pControl, sizeof(Uns16), 1, lShipFile)) {
      Error("Can't read file '%s'", SHIP_FILE);
      lError = IO_FAILURE;
    }

    for (i = 1; i <= SHIP_NR; i++) {
#ifdef __MSDOS__
      if (fread(&lShip, sizeof(Ship_Struct), 1, lShipFile) NEQ 1) {
#else
      memset(&lShip, 0, sizeof(lShip));
      if (!DOSReadStruct(ShipStruct_Convert, NumShipStruct_Convert, &lShip,
                  lShipFile)) {
#endif
        Error("Can't read file '%s'", SHIP_FILE);
        lError = IO_FAILURE;    /* i/o error */
        break;
      }
      else {
        if (lShip.Owner NEQ 0)
          PutShip(i, &lShip);
      }
    }

    fclose(lShipFile);
  }
  else
    lError = IO_FAILURE;
  return (lError);
}

IO_Def
Write_Ships_File(Int16 pControl)
{
  FILE *lShipFile;
  Int16 i;
  IO_Def lError = IO_SUCCESS;
  Ship_Struct lShip;

  if (gAllShips EQ 0) {
    Error("Cannot write ships file without first reading it.\n");
    return IO_FAILURE;
  }

  if ((lShipFile = OpenOutputFile(SHIP_FILE, GAME_DIR_ONLY)) NEQ NULL) {
    if (1 NEQ fwrite(&pControl, sizeof(Uns16), 1, lShipFile)) {
      Error("Can't write to file '%s'", SHIP_FILE);
      lError = IO_FAILURE;
    }

    memset(&lShip, 0, sizeof(lShip));

    for (i = 1; i <= SHIP_NR; i++) {
      lShip.Id = i;
#ifdef __MSDOS__
      if (1 NEQ fwrite((IsShipExist(i) ? GetShip(i) : &lShip),
                  sizeof(Ship_Struct), 1, lShipFile)) {
#else
      if (!DOSWriteStruct(ShipStruct_Convert, NumShipStruct_Convert,
                  IsShipExist(i) ? GetShip(i) : &lShip, lShipFile)) {
#endif
        Error("Can't write to file '%s'", SHIP_FILE);
        lError = IO_FAILURE;
        break;
      }
    }

    fclose(lShipFile);
  }
  else
    lError = IO_FAILURE;
  return (lError);
}

void
InitializeShips(void)
{
  InitShips();

  memset(gAllShips, 0, (SHIP_NR + 1) * sizeof(Ship_Struct));
}

void
WrapShipLocation(Uns16 pID)
{
  Int16 lLocation,
    lWrap,
    lWaypoint;
  Ship_Struct *lShip = GetShip(pID);

  lLocation = lShip->Location[0];
  lWrap = WrapMapX(lLocation);
  if (lWrap NEQ lLocation) {
    lWaypoint = ShipWaypointX(pID);
    lShip->Location[0] = lWrap;
    PutShipRelWaypointX(pID, lWaypoint - lWrap);
  }

  lLocation = lShip->Location[1];
  lWrap = WrapMapY(lLocation);
  if (lWrap NEQ lLocation) {
    lWaypoint = ShipWaypointY(pID);
    lShip->Location[1] = lWrap;
    PutShipRelWaypointY(pID, lWaypoint - lWrap);
  }
}

/*************************************************************
  $HISTORY:$
**************************************************************/

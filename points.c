/****************************************************************************
All files in this distribution are Copyright (C) 1995-2001 by the program
authors: Andrew Sterian, Thomas Voigt and Steffen Pietsch.

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
 *                                                                *
 *  Module:  Evaluate PTScore-like points                         *
 *  Project:   Portable Host Development Kit                      *
 *  File Name:   points.c                                         *
 *  Programmer:  Piotr Winiarczyk                                 *
 *  Create Date: December 7, 2001                                 *
 *  Description: This module provides routines for ptscore-like   *
 *      points evaluation planets, ships, bases, mines.           *
 *      Basic count functions implemented.                        *
 *  Functions:                                                    *
 *  Change Record:                                                *
 *     22.12.2001 - THost point added.                            *
 *                  Race statistic functions                      *
 *                                                                *
 ******************************************************************/

#include "phostpdk.h"
#include "private.h"

static Uns32                     /* Default points values */
PointsForCargo[NumCargoTypes] = { 0, 3, 3, 3, 0, 1, 1 };

Uns32
PointsFor(CargoType_Def lCargo)
{
  passert((lCargo >= 0) AND(lCargo < NumCargoTypes));
  return PointsForCargo[lCargo];
}

void
PutPointsFor(CargoType_Def lCargo, Uns16 lPoints)
{
  passert((lCargo >= 0) AND(lCargo < NumCargoTypes));
  PointsForCargo[lCargo]=lPoints;
  return;
}

Uns32
PointsForHull(Uns16 lHullNr)
{
  passert((lHullNr >= 1) AND(lHullNr <= HULL_NR));
  return HullTritCost(lHullNr) *PointsFor(TRITANIUM)+
         HullDurCost(lHullNr)  *PointsFor(DURANIUM)+
         HullMolyCost(lHullNr) *PointsFor(MOLYBDENUM)+
         HullMoneyCost(lHullNr)*PointsFor(CREDITS);
}

Uns32
PointsForEngine(Uns16 lEngNr)
{
  passert((lEngNr >= 1) AND(lEngNr <= ENGINE_NR));
  return EngTritCost(lEngNr) *PointsFor(TRITANIUM)+
         EngDurCost(lEngNr)  *PointsFor(DURANIUM)+
         EngMolyCost(lEngNr) *PointsFor(MOLYBDENUM)+
         EngMoneyCost(lEngNr)*PointsFor(CREDITS);
}

Uns32
PointsForBeam(Uns16 lBeamNr)
{
  passert((lBeamNr >= 1) AND(lBeamNr <= BEAM_NR));
  return BeamTritCost(lBeamNr) *PointsFor(TRITANIUM)+
         BeamDurCost(lBeamNr)  *PointsFor(DURANIUM)+
         BeamMolyCost(lBeamNr) *PointsFor(MOLYBDENUM)+
         BeamMoneyCost(lBeamNr)*PointsFor(CREDITS);
}

Uns32
PointsForTube(Uns16 lTorpNr)
{
  passert((lTorpNr >= 1) AND(lTorpNr <= TORP_NR));
  return TorpTritCost(lTorpNr) *PointsFor(TRITANIUM)+
         TorpDurCost(lTorpNr)  *PointsFor(DURANIUM)+
         TorpMolyCost(lTorpNr) *PointsFor(MOLYBDENUM)+
         TorpTubeCost(lTorpNr) *PointsFor(CREDITS);
}

Uns32
PointsForFighter(void)
{
  return 3 *PointsFor(TRITANIUM)+
         2 *PointsFor(MOLYBDENUM)+
         5 *PointsFor(SUPPLIES);
}

Uns32
PointsForTorpedo(Uns16 lTorpNr)
{
  passert((lTorpNr >= 1) AND(lTorpNr <= TORP_NR));
  return 1 *PointsFor(TRITANIUM)+
         1 *PointsFor(DURANIUM)+
         1 *PointsFor(MOLYBDENUM)+
         TorpTorpCost(lTorpNr) *PointsFor(CREDITS);
}

Uns32
PointsForShipParts(Uns16 sID)
{
  Uns32 lPoints=0;

  passert(IsShipExist(sID));

  lPoints += PointsForHull(ShipHull(sID));
  lPoints += PointsForEngine(ShipEngine(sID))*HullEngineNumber(ShipHull(sID));
  if (ShipBeamNumber(sID)>0)
    lPoints += PointsForBeam(ShipBeamType(sID))*ShipBeamNumber(sID);
  if (ShipTubeNumber(sID)>0)
    lPoints += PointsForTube(ShipTorpType(sID))*ShipTubeNumber(sID);

  return lPoints;
}

Uns32
PointsForShipCargo(Uns16 sID)
{
  Uns32 lPoints=0;
  CargoType_Def i;

  passert(IsShipExist(sID));

  /* Cargo */
  for(i=NEUTRONIUM;i<=CREDITS;i++)
   lPoints += PointsFor(i)*ShipCargo(sID,i);

  /* Transfer cargo */
  for(i=NEUTRONIUM;i<=CREDITS;i++)
   lPoints += PointsFor(i)*ShipTransfer(sID,i);

  /* Dump cargo */
  for(i=NEUTRONIUM;i<=CREDITS;i++)
   lPoints += PointsFor(i)*ShipDump(sID,i);

  return lPoints;
}

Uns32
PointsForShipAmmo(Uns16 sID)
{
  Uns32 lPoints=0;

  passert(IsShipExist(sID));

  if (ShipBays(sID)>0)
    lPoints +=  ShipAmmunition(sID) * PointsForFighter();

  if (ShipTubeNumber(sID)>0)
    lPoints +=  ShipAmmunition(sID) * PointsForTorpedo(ShipTorpType(sID));

  return lPoints;
}

Uns32
PointsForShip(Uns16 sID, PointsType_Def CountType)
{
  passert(IsShipExist(sID));
  
  return ((CountType!=UNUSED_POINTS)?1:0)*PointsForShipParts(sID)+
         ((CountType!=  USED_POINTS)?1:0)*PointsForShipCargo(sID)+
         ((CountType!=UNUSED_POINTS)?1:0)*PointsForShipAmmo(sID);
}

Uns32
PointsForMineralMine(void)
{
  return 4 *PointsFor(CREDITS)+
         1 *PointsFor(SUPPLIES);
}

Uns32
PointsForFactory(void)
{
  return 3 *PointsFor(CREDITS)+
         1 *PointsFor(SUPPLIES);
}

Uns32
PointsForDefencePost(void)
{
  return 10 *PointsFor(CREDITS)+
         1  *PointsFor(SUPPLIES);
}

Uns32
PointsForBaseOnPlanet(void)
{
  return 402 *PointsFor(TRITANIUM)+
         120 *PointsFor(DURANIUM)+
         340 *PointsFor(MOLYBDENUM)+
         900 *PointsFor(CREDITS);
}

Uns32
PointsForPlanetStructures(Uns16 pID)
{
  passert(IsPlanetExist(pID));

  return PointsForMineralMine()*PlanetMines(pID)+
         PointsForFactory()    *PlanetFactories(pID)+
         PointsForDefencePost()*PlanetDefense(pID)+
         (PlanetBuildBase(pID)? PointsForBaseOnPlanet(): 0);
}

Uns32
PointsForPlanetMinerals(Uns16 pID)
{
  Uns32 lPoints=0;
  CargoType_Def i;

  passert(IsPlanetExist(pID));

  /* Cargo */
  for(i=NEUTRONIUM;i<=CREDITS;i++)
   lPoints += PointsFor(i)*PlanetCargo(pID,i);

 return lPoints;
}

Uns32
PointsForPlanet(Uns16 pID, PointsType_Def CountType)
{
  passert(IsPlanetExist(pID));

  return ((CountType!=UNUSED_POINTS)?1:0)*PointsForPlanetStructures(pID)+
         ((CountType!=  USED_POINTS)?1:0)*PointsForPlanetMinerals(pID);
}

Uns32
PointsForBaseTechLevels(Uns16 bID)
{
  Uns32 lPoints=0;
  BaseTech_Def i;

  passert(IsBaseExist(bID));

  for (i = ENGINE_TECH; i <= TORP_TECH; i++)
    lPoints += 50 * BaseTech(bID,i) * (BaseTech(bID,i) - 1)
                  * PointsFor(CREDITS);

  return lPoints;
}

Uns32
PointsForBaseDefensePost(void)
{
  return 1  *PointsFor(DURANIUM)+
         10 *PointsFor(CREDITS);
}

Uns32
PointsForBaseDefense(Uns16 bID)
{
  passert(IsBaseExist(bID));

  return PointsForBaseDefensePost()*BaseDefense(bID)+
         PointsForFighter()*BaseFighters(bID);
}

Uns32
PointsForBaseStorage(Uns16 bID)
{
  Uns32 lPoints=0;
  Uns16 i;

  passert(IsBaseExist(bID));

  for (i = 1; i <= RACEHULLS; i++)
  if (BaseHulls(bID,i)!=0)
    lPoints += BaseHulls(bID,i) * PointsForHull(TrueHull(BaseOwner(bID),i));

  for (i = 1; i <= ENGINE_NR; i++)
    lPoints += BaseEngines(bID,i) * PointsForEngine(i);

  for (i = 1; i <= BEAM_NR; i++)
    lPoints += BaseBeams(bID,i)   * PointsForBeam(i);

  for (i = 1; i <= TORP_NR; i++)
    lPoints += BaseTubes(bID,i)   * PointsForTube(i);

  return lPoints;
}

Uns32
PointsForBaseTorpedos(Uns16 bID)
{
  Uns32 lPoints=0;
  Uns16 i;

  passert(IsBaseExist(bID));

  for (i = 1; i <= TORP_NR; i++)
    lPoints += BaseTorps(bID,i)   * PointsForTorpedo(i);

  return lPoints;
}

Uns32
PointsForBase(Uns16 bID, PointsType_Def CountType)
{
  passert(IsBaseExist(bID));

  return ((CountType!=UNUSED_POINTS)?1:0)*PointsForBaseOnPlanet()+
         ((CountType!=  USED_POINTS)?1:0)*PointsForBaseStorage(bID)+
         ((CountType!=UNUSED_POINTS)?1:0)*PointsForBaseTorpedos(bID)+
         ((CountType!=UNUSED_POINTS)?1:0)*PointsForBaseDefense(bID)+
         ((CountType!=UNUSED_POINTS)?1:0)*PointsForBaseTechLevels(bID);
}

/* MineFieldMultiplier procedure is taken from ptscore.c
   It is private function for points.c */

/* calculates how many 10000 mines are worth :
   The score for a certain torp is torp_score*10000/mines_per_torp */
Uns32
MineFieldMultiplier(void)
{
  Uns16 lCount;
  Uns32 lMinimum;

  lMinimum = (TorpTorpCost(1) * PointsFor(CREDITS) +
             PointsFor(TRITANIUM)+PointsFor(DURANIUM)+PointsFor(MOLYBDENUM))
             * 10000;
  for (lCount = 2; lCount <= TORP_NR; lCount++)
    lMinimum =
          min(lMinimum,
          (((Uns32) TorpTorpCost(lCount)) * PointsFor(CREDITS) +
          PointsFor(TRITANIUM)+PointsFor(DURANIUM)+PointsFor(MOLYBDENUM))
          * 10000 / (lCount * lCount));

  return lMinimum;
}

Uns32
PointsForMinefield(Uns16 mID, PointsType_Def CountType)
{
   return ((CountType!=UNUSED_POINTS)?1:0)*
          MinefieldUnits(mID)* MineFieldMultiplier() / 10000;
}

Boolean
IsShipType(Uns16 sID,ShipsType_Def ShipsType)
{
   passert(IsShipExist(sID));

   if (ShipsType==ALL_SHIPS) return True;

   if ((ShipBays(sID)>0)||(ShipTubeNumber(sID)>0)||
         (ShipBeamNumber(sID)>0))
   {
    if (ShipsType==WAR_SHIPS)
          return True;
    else
          return False;
   }
   else
   {
    if (ShipsType==WAR_SHIPS)
          return False;
    else
          return True;
   }
  return False;
}


Uns32
RaceScoreForShips( RaceType_Def Race,
                 PointsType_Def CountType,
                  ShipsType_Def ShipsType)
{
  Uns16 lCount,lCount_2;
  Uns32 lScore=0;

  if (CountType==THOST_POINTS)
  {
      lCount   = RaceShipsNumber(Race,WAR_SHIPS);
      lCount_2 = RaceShipsNumber(Race,FREIGHTER_SHIPS);

      if ((ShipsType==WAR_SHIPS)||(ShipsType==ALL_SHIPS))
         lScore += lCount*10;   // 10 points for each armed ship in THost

      if ((ShipsType==FREIGHTER_SHIPS)||(ShipsType==ALL_SHIPS))
         lScore += lCount*1;    // 1 point for each armed ship in THost
         
  }
  else
  for (lCount=1; lCount <= SHIP_NR ; lCount++)
      if (IsShipExist(lCount))
       if ((ShipOwner(lCount)==Race)&&(IsShipType(lCount,ShipsType)))
         lScore += PointsForShip(lCount,CountType);

 return lScore;
}

Uns32
RaceScoreForPlanets( RaceType_Def Race,
                   PointsType_Def CountType)
{
  Uns16 lCount;
  Uns32 lScore=0;
  
  if (CountType==THOST_POINTS) lScore = RacePlanetsNumber(Race) * 10;
                              // 10 points for each planet in THost
  else
  for (lCount=1; lCount <= PLANET_NR ; lCount++)
      if (IsPlanetExist(lCount))
       if (PlanetOwner(lCount)==Race)
         lScore += PointsForPlanet(lCount,CountType);

 return lScore;
}

Uns32
RaceScoreForBases( RaceType_Def Race,
                   PointsType_Def CountType)
{
  Uns16 lCount;
  Uns32 lScore=0;
  
  if (CountType==THOST_POINTS) lScore = RaceBasesNumber(Race) * 120;
                               // 120 points for each base in THost
  else
  for (lCount=1; lCount <= BASE_NR ; lCount++)
      if (IsBaseExist(lCount))
       if (BaseOwner(lCount)==Race)
         lScore += PointsForBase(lCount,CountType);

 return lScore;
}


Uns32
RaceScoreForMinefields( RaceType_Def Race,
                      PointsType_Def CountType,
                      Boolean RaceModify)
{
  Uns16  lCount;
  Uns32  lScore=0;
  double lModifiedScore=0;
  
  if (CountType==THOST_POINTS) lScore = 0; // no points for minefields in THost
  else
  for (lCount=1; lCount <= MINE_NR ; lCount++)
      if (IsMinefieldExist(lCount))
       if (MinefieldOwner(lCount)==Race)
         lScore += PointsForMinefield(lCount,CountType);

 lModifiedScore = (double)lScore / gPconfigInfo->UnitsPerTorpRate[Race] * 100.0;

 if (RaceModify) return lModifiedScore;
 else            return lScore;
}

Uns32
RaceShipsNumber( RaceType_Def Race, ShipsType_Def ShipsType)
{
  Uns16 lCount;
  Uns32 lScore=0;
  
  for (lCount=1; lCount <= SHIP_NR ; lCount++)
      if (IsShipExist(lCount))
       if ((ShipOwner(lCount)==Race)&&(IsShipType(lCount,ShipsType)))
         lScore++;

 return lScore;
}

Uns32
RacePlanetsNumber(RaceType_Def Race)
{
  Uns16  lCount;
  Uns32  lScore=0;

  for (lCount=1; lCount <= PLANET_NR ; lCount++)
      if (IsPlanetExist(lCount))
       if (PlanetOwner(lCount)==Race)
         lScore++;

  return lScore;
}

Uns32
RaceBasesNumber( RaceType_Def Race)
{
  Uns16 lCount;
  Uns32 lScore=0;
  
  for (lCount=1; lCount <= BASE_NR ; lCount++)
      if (IsBaseExist(lCount))
       if (BaseOwner(lCount)==Race)
         lScore++;

 return lScore;
}

Uns32
RaceFightersNumber(RaceType_Def Race)
{
  Uns16 lCount;
  Uns32 lScore=0;

  /* Check bases first */

  for (lCount=1; lCount <= BASE_NR ; lCount++)
      if (IsBaseExist(lCount))
       if (BaseOwner(lCount)==Race)
         lScore+=BaseFighters(lCount);

  /* Check ships */

  for (lCount=1; lCount <= SHIP_NR ; lCount++)
      if (IsShipExist(lCount))
       if ((ShipOwner(lCount)==Race)&&(ShipBays(lCount)) )
         lScore+=ShipAmmunition(lCount);

 return lScore;
}

Uns32
RaceTorpedosNumber(RaceType_Def Race, Uns16 TorpType)
{
  Uns16 lCount;
  Uns32 lScore=0;

  /* Check bases first */

  for (lCount=1; lCount <= BASE_NR ; lCount++)
      if (IsBaseExist(lCount))
       if (BaseOwner(lCount)==Race)
         lScore+=BaseTorps(lCount,TorpType);

  /* Check ships */

  for (lCount=1; lCount <= SHIP_NR ; lCount++)
      if (IsShipExist(lCount))
       if ((ShipOwner(lCount)==Race)&&(ShipTubeNumber(lCount)))
         if (ShipTorpType(lCount)==TorpType)
           lScore+=ShipAmmunition(lCount);

 return lScore;
}

Uns32
RaceFactoriesNumber(RaceType_Def Race)
{
  Uns16 lCount;
  Uns32 lScore=0;

  for (lCount=1; lCount <= PLANET_NR ; lCount++)
      if (IsPlanetExist(lCount))
       if (PlanetOwner(lCount)==Race)
          lScore += PlanetFactories(lCount);

  return lScore;
}

Uns32
RaceMineralMinesNumber(RaceType_Def Race)
{
  Uns16 lCount;
  Uns32 lScore=0;

  for (lCount=1; lCount <= PLANET_NR ; lCount++)
      if (IsPlanetExist(lCount))
       if (PlanetOwner(lCount)==Race)
          lScore+=PlanetMines(lCount);

  return lScore;
}

Uns32
RaceDefencePostsNumber(RaceType_Def Race)
{
  Uns16 lCount;
  Uns32 lScore=0;

  for (lCount=1; lCount <= PLANET_NR ; lCount++)
      if (IsPlanetExist(lCount))
       if (PlanetOwner(lCount)==Race)
          lScore+=PlanetDefense(lCount);

  return lScore;
}

Uns32
RaceBaseDefenceNumber(RaceType_Def Race)
{
  Uns16 lCount;
  Uns32 lScore=0;

  for (lCount=1; lCount <= BASE_NR ; lCount++)
      if (IsBaseExist(lCount))
       if (BaseOwner(lCount)==Race)
          lScore+=BaseDefense(lCount);

  return lScore;
}

Uns32
RaceCargoNumber(RaceType_Def Race,CargoType_Def CargoType)
{
  Uns16 lCount;
  Uns32 lScore=0;

  /* Check planets first */

  for (lCount=1; lCount <= PLANET_NR ; lCount++)
      if (IsPlanetExist(lCount))
       if (PlanetOwner(lCount)==Race)
         lScore+=PlanetCargo(lCount,CargoType);

  /* Check ships */

  for (lCount=1; lCount <= SHIP_NR ; lCount++)
      if (IsShipExist(lCount))
       if (ShipOwner(lCount)==Race)
       {
           lScore+=ShipCargo(lCount,CargoType);
           lScore+=ShipTransfer(lCount,CargoType);
           lScore+=ShipDump(lCount,CargoType);
       }

 return lScore;
}

Uns32
RaceMinefieldUnitsNumber(RaceType_Def Race)
{
  Uns16 lCount;
  Uns32 lScore=0;

  /* Check planets first */

  for (lCount=1; lCount <= MINE_NR ; lCount++)
      if (IsMinefieldExist(lCount))
       if (MinefieldOwner(lCount)==Race)
         lScore+=MinefieldUnits(lCount);
         
 return lScore;
}


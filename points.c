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

#define TRITANIUM_FOR_FIGHTER     3
#define DURANIUM_FOR_FIGHTER      0
#define MOLYBDENUM_FOR_FIGHTER    2
#define CREDITS_FOR_FIGHTER       0
#define SUPPLIES_FOR_FIGHTER      5

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
RaceBeamsNumber(RaceType_Def Race, Uns16 BeamType)
{
  Uns16 lCount;
  Uns32 lScore=0;

  /* Check bases first */

  for (lCount=1; lCount <= BASE_NR ; lCount++)
      if (IsBaseExist(lCount))
       if (BaseOwner(lCount)==Race)
         lScore+=BaseBeams(lCount,BeamType);

  /* Check ships */

  for (lCount=1; lCount <= SHIP_NR ; lCount++)
      if (IsShipExist(lCount))
       if ((ShipOwner(lCount)==Race)&&(ShipBeamNumber(lCount)))
         if (ShipBeamType(lCount)==BeamType)
           lScore+=ShipBeamNumber(lCount);

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

/* Count minerals */
Uns32
MineralsForHull(Uns16 lHullNr, CargoType_Def CargoType)
{
  Uns32 Value;

  passert((lHullNr >= 1) AND(lHullNr <= HULL_NR));

  switch (CargoType)
  {
    case TRITANIUM:  Value =HullTritCost (lHullNr); break;
    case DURANIUM:   Value =HullDurCost  (lHullNr); break;
    case MOLYBDENUM: Value =HullMolyCost (lHullNr); break;
    case CREDITS:    Value =HullMoneyCost(lHullNr); break;
    default:
      Value =0;
  }
  return Value;
}

Uns32
MineralsForEngine(Uns16 lEngNr, CargoType_Def CargoType)
{
  Uns32 Value;

  passert((lEngNr >= 1) AND(lEngNr <= ENGINE_NR));

  switch (CargoType)
  {
    case TRITANIUM:  Value =EngTritCost (lEngNr); break;
    case DURANIUM:   Value =EngDurCost  (lEngNr); break;
    case MOLYBDENUM: Value =EngMolyCost (lEngNr); break;
    case CREDITS:    Value =EngMoneyCost(lEngNr); break;
    default:
      Value =0;
  }
  return Value;
}

Uns32
MineralsForBeam(Uns16 lBeamNr, CargoType_Def CargoType)
{
  Uns32 Value;

  passert((lBeamNr >= 1) AND(lBeamNr <= BEAM_NR));

  switch (CargoType)
  {
    case TRITANIUM:  Value =BeamTritCost (lBeamNr); break;
    case DURANIUM:   Value =BeamDurCost  (lBeamNr); break;
    case MOLYBDENUM: Value =BeamMolyCost (lBeamNr); break;
    case CREDITS:    Value =BeamMoneyCost(lBeamNr); break;
    default:
      Value =0;
  }
  return Value;
}

Uns32
MineralsForTube(Uns16 lTorpNr, CargoType_Def CargoType)
{
  Uns32 Value;

  passert((lTorpNr >= 1) AND(lTorpNr <= TORP_NR));

  switch (CargoType)
  {
    case TRITANIUM:  Value =TorpTritCost (lTorpNr); break;
    case DURANIUM:   Value =TorpDurCost  (lTorpNr); break;
    case MOLYBDENUM: Value =TorpMolyCost (lTorpNr); break;
    case CREDITS:    Value =TorpTubeCost(lTorpNr); break;
    default:
      Value =0;
  }
  return Value;
}

Uns32
MineralsForFighter(CargoType_Def CargoType)
{
  Uns32 Value;

  switch (CargoType)
  {
    case TRITANIUM:  Value =TRITANIUM_FOR_FIGHTER; break;
    case DURANIUM:   Value =DURANIUM_FOR_FIGHTER; break;
    case MOLYBDENUM: Value =MOLYBDENUM_FOR_FIGHTER; break;
    case CREDITS:    Value =CREDITS_FOR_FIGHTER; break;
    case SUPPLIES:   Value =SUPPLIES_FOR_FIGHTER; break;
    default:
      Value =0;
  }
  return Value;
}

Uns32
MineralsForTorpedo(Uns16 lTorpNr, CargoType_Def CargoType)
{
  Uns32 Value;

  passert((lTorpNr >= 1) AND(lTorpNr <= TORP_NR));

  switch (CargoType)
  {
    case TRITANIUM:  Value =1; break;
    case DURANIUM:   Value =1; break;
    case MOLYBDENUM: Value =1; break;
    case CREDITS:    Value =TorpTorpCost(lTorpNr); break;
    default:
      Value =0;
  }
  return Value;
}

Uns32
MineralsForShipParts(Uns16 sID, CargoType_Def CargoType)
{
  Uns32 lPoints=0;

  passert(IsShipExist(sID));

  lPoints += MineralsForHull(ShipHull(sID),CargoType);
  lPoints += MineralsForEngine(ShipEngine(sID),CargoType)*HullEngineNumber(ShipHull(sID));
  if (ShipBeamNumber(sID)>0)
    lPoints += MineralsForBeam(ShipBeamType(sID),CargoType)*ShipBeamNumber(sID);
  if (ShipTubeNumber(sID)>0)
    lPoints += MineralsForTube(ShipTorpType(sID),CargoType)*ShipTubeNumber(sID);

  return lPoints;
}

Uns32
MineralsForShipCargo(Uns16 sID, CargoType_Def CargoType)
{
  Uns32 lPoints=0;

  passert(IsShipExist(sID));

  /* Cargo */
   lPoints += ShipCargo(sID,CargoType);

  /* Transfer cargo */
   lPoints += ShipTransfer(sID,CargoType);

  /* Dump cargo */
   lPoints += ShipDump(sID,CargoType);

  return lPoints;
}

Uns32
MineralsForShipAmmo(Uns16 sID, CargoType_Def CargoType)
{
  Uns32 lPoints=0;

  passert(IsShipExist(sID));

  if (ShipBays(sID)>0)
    lPoints +=  ShipAmmunition(sID) * MineralsForFighter(CargoType);

  if (ShipTubeNumber(sID)>0)
    lPoints +=  ShipAmmunition(sID) * MineralsForTorpedo(ShipTorpType(sID),CargoType);

  return lPoints;
}

Uns32
MineralsForShip(Uns16 sID, PointsType_Def CountType, CargoType_Def CargoType)
{
  passert(IsShipExist(sID));
  
  return ((CountType!=UNUSED_POINTS)?1:0)*MineralsForShipParts(sID,CargoType)+
         ((CountType!=  USED_POINTS)?1:0)*MineralsForShipCargo(sID,CargoType)+
         ((CountType!=UNUSED_POINTS)?1:0)*MineralsForShipAmmo(sID,CargoType);
}

Uns32
MineralsForMineralMine(CargoType_Def CargoType)
{
  Uns32 Value;

  switch (CargoType)
  {
    case CREDITS:    Value =4; break;
    case SUPPLIES:   Value =1; break;
    default:
      Value =0;
  }
  return Value;
}

Uns32
MineralsForFactory(CargoType_Def CargoType)
{
  Uns32 Value;

  switch (CargoType)
  {
    case CREDITS:    Value =3; break;
    case SUPPLIES:   Value =1; break;
    default:
      Value =0;
  }
  return Value;
}

Uns32
MineralsForDefencePost(CargoType_Def CargoType)
{
  Uns32 Value;

  switch (CargoType)
  {
    case CREDITS:    Value =10; break;
    case SUPPLIES:   Value =1; break;
    default:
      Value =0;
  }
  return Value;
}

Uns32
MineralsForBaseOnPlanet(CargoType_Def CargoType)
{
  Uns32 Value;

  switch (CargoType)
  {
    case TRITANIUM:   Value =402; break;
    case DURANIUM:    Value =120; break;
    case MOLYBDENUM:  Value =340; break;
    case CREDITS:     Value =900; break;
    default:
      Value =0;
  }
  return Value;
}

Uns32
MineralsForPlanetStructures(Uns16 pID,CargoType_Def CargoType)
{
  passert(IsPlanetExist(pID));

  return MineralsForMineralMine(CargoType)*PlanetMines(pID)+
         MineralsForFactory(CargoType)    *PlanetFactories(pID)+
         MineralsForDefencePost(CargoType)*PlanetDefense(pID)+
         (PlanetBuildBase(pID)? MineralsForBaseOnPlanet(CargoType): 0);
}

Uns32
MineralsForPlanetMinerals(Uns16 pID,CargoType_Def CargoType)
{
  Uns32 lPoints=0;

  passert(IsPlanetExist(pID));

  /* Cargo */
   lPoints += PlanetCargo(pID,CargoType);

 return lPoints;
}

Uns32
MineralsForPlanet(Uns16 pID, PointsType_Def CountType,CargoType_Def CargoType)
{
  passert(IsPlanetExist(pID));

  return ((CountType!=UNUSED_POINTS)?1:0)*MineralsForPlanetStructures(pID,CargoType)+
         ((CountType!=  USED_POINTS)?1:0)*MineralsForPlanetMinerals(pID,CargoType);
}

Uns32
MineralsForBaseTechLevels(Uns16 bID,CargoType_Def CargoType)
{
  Uns32 lPoints=0;
  BaseTech_Def i;

  passert(IsBaseExist(bID));

  if (CargoType == CREDITS)
  for (i = ENGINE_TECH; i <= TORP_TECH; i++)
    lPoints += 50 * BaseTech(bID,i) * (BaseTech(bID,i) - 1);

  return lPoints;
}

Uns32
MineralsForBaseDefensePost(CargoType_Def CargoType)
{
  Uns32 Value;

  switch (CargoType)
  {
    case DURANIUM:   Value =1; break;
    case CREDITS:    Value =10; break;
    default:
      Value =0;
  }
  return Value;
}

Uns32
MineralsForBaseDefense(Uns16 bID,CargoType_Def CargoType)
{
  passert(IsBaseExist(bID));

  return MineralsForBaseDefensePost(CargoType)*BaseDefense(bID)+
         MineralsForFighter(CargoType)*BaseFighters(bID);
}

Uns32
MineralsForBaseStorage(Uns16 bID,CargoType_Def CargoType)
{
  Uns32 lPoints=0;
  Uns16 i;

  passert(IsBaseExist(bID));

  for (i = 1; i <= RACEHULLS; i++)
  if (BaseHulls(bID,i)!=0)
    lPoints += BaseHulls(bID,i) * MineralsForHull(TrueHull(BaseOwner(bID),i),CargoType);

  for (i = 1; i <= ENGINE_NR; i++)
    lPoints += BaseEngines(bID,i) * MineralsForEngine(i,CargoType);

  for (i = 1; i <= BEAM_NR; i++)
    lPoints += BaseBeams(bID,i)   * MineralsForBeam(i,CargoType);

  for (i = 1; i <= TORP_NR; i++)
    lPoints += BaseTubes(bID,i)   * MineralsForTube(i,CargoType);

  return lPoints;
}

Uns32
MineralsForBaseTorpedos(Uns16 bID,CargoType_Def CargoType)
{
  Uns32 lPoints=0;
  Uns16 i;

  passert(IsBaseExist(bID));

  for (i = 1; i <= TORP_NR; i++)
    lPoints += BaseTorps(bID,i)   * MineralsForTorpedo(i,CargoType);

  return lPoints;
}

Uns32
MineralsForBase(Uns16 bID, PointsType_Def CountType,CargoType_Def CargoType)
{
  passert(IsBaseExist(bID));

  return ((CountType!=UNUSED_POINTS)?1:0)*MineralsForBaseOnPlanet(CargoType)+
         ((CountType!=  USED_POINTS)?1:0)*MineralsForBaseStorage(bID,CargoType)+
         ((CountType!=UNUSED_POINTS)?1:0)*MineralsForBaseTorpedos(bID,CargoType)+
         ((CountType!=UNUSED_POINTS)?1:0)*MineralsForBaseDefense(bID,CargoType)+
         ((CountType!=UNUSED_POINTS)?1:0)*MineralsForBaseTechLevels(bID,CargoType);
}

/* Private function for points.c */

Uns32
MineralsForMinefieldUnit(CargoType_Def CargoType)
{
  Uns32 Value;
  Uns16 lCount;
  Uns32 lMinimum;

  lMinimum = TorpTorpCost(1) * 10000;
  for (lCount = 2; lCount <= TORP_NR; lCount++)
    lMinimum = min(lMinimum, ((Uns32) TorpTorpCost(lCount))
                * 10000 / (lCount * lCount));

  switch (CargoType)
  {
    case TRITANIUM:   Value =100; break;
    case DURANIUM:    Value =100; break;
    case MOLYBDENUM:  Value =100; break;
    case CREDITS:     Value =lMinimum; break;
    default:
      Value =0;
  }
  return Value;

}

Uns32
MineralsForMinefield(Uns16 mID, PointsType_Def CountType,CargoType_Def CargoType)
{
   return ((CountType!=UNUSED_POINTS)?1:0)*
          (double)MinefieldUnits(mID)*
          (double)MineralsForMinefieldUnit(CargoType)/ 10000.0;
}

Uns32
RaceMineralsForShips( RaceType_Def Race,
                 PointsType_Def CountType,
                  ShipsType_Def ShipsType,
                  CargoType_Def CargoType)
{
  Uns16 lCount,lCount_2;
  Uns32 lScore=0;

  if (CountType==THOST_POINTS)
  {
      lCount   = RaceShipsNumber(Race,WAR_SHIPS);
      lCount_2 = RaceShipsNumber(Race,FREIGHTER_SHIPS);

      if ((ShipsType==WAR_SHIPS)||(ShipsType==ALL_SHIPS))
         lScore += lCount*0;   //  0 points for each armed ship in THost

      if ((ShipsType==FREIGHTER_SHIPS)||(ShipsType==ALL_SHIPS))
         lScore += lCount*0;    // 0 point for each armed ship in THost
         
  }
  else
  for (lCount=1; lCount <= SHIP_NR ; lCount++)
      if (IsShipExist(lCount))
       if ((ShipOwner(lCount)==Race)&&(IsShipType(lCount,ShipsType)))
         lScore += MineralsForShip(lCount,CountType,CargoType);

 return lScore;
}

Uns32
RaceMineralsForPlanets( RaceType_Def Race,
                   PointsType_Def CountType,
                   CargoType_Def CargoType)
{
  Uns16 lCount;
  Uns32 lScore=0;
  
  if (CountType==THOST_POINTS) lScore = 0;
                              // 0 points for each planet in THost
  else
  for (lCount=1; lCount <= PLANET_NR ; lCount++)
      if (IsPlanetExist(lCount))
       if (PlanetOwner(lCount)==Race)
         lScore += MineralsForPlanet(lCount,CountType,CargoType);

 return lScore;
}

Uns32
RaceMineralsForBases( RaceType_Def Race,
                   PointsType_Def CountType,
                   CargoType_Def CargoType)
{
  Uns16 lCount;
  Uns32 lScore=0;
  
  if (CountType==THOST_POINTS) lScore = 0;

  else
  for (lCount=1; lCount <= BASE_NR ; lCount++)
      if (IsBaseExist(lCount))
       if (BaseOwner(lCount)==Race)
         lScore += MineralsForBase(lCount,CountType,CargoType);

 return lScore;
}



Uns32
RaceMineralsForMinefields( RaceType_Def Race,
                      PointsType_Def CountType,
                      Boolean RaceModify,
                      CargoType_Def CargoType)
{
  Uns16  lCount;
  Uns32  lScore=0;
  double lModifiedScore=0;
  
  if (CountType==THOST_POINTS) lScore = 0; // no points for minefields in THost
  else
  for (lCount=1; lCount <= MINE_NR ; lCount++)
      if (IsMinefieldExist(lCount))
       if (MinefieldOwner(lCount)==Race)
         lScore += MineralsForMinefield(lCount,CountType,CargoType);

 lModifiedScore = (double)lScore / gPconfigInfo->UnitsPerTorpRate[Race] * 100.0;

 if (RaceModify) return lModifiedScore;
 else            return lScore;
}


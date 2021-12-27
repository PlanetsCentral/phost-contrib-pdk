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

/*
 *    E N U M E R A T I O N    F U N C T I O N S
 */

#include "phostpdk.h"
#include "private.h"

/** Pointer to memory for enumeration functions. This one is shared
    between all the functions. This used to be statically sized to
    500 objects, but that failed with CPNumMinefields>500 on things
    like EnumerateMinesWithinRadius(2000,2000,5000). */
static Uns16 *newEnumPointer = 0;

/** Number of Uns16's at newEnumPointer */
static Uns16 newEnumSize = 0;

/** Current index. */
static Uns16 newEnumIndex = 0;

static void
FreeEnumerations(void)
{
  MemFree(newEnumPointer);
  newEnumPointer = 0;
  newEnumSize = newEnumIndex = 0;
}

/** Initialize Enumerations. Call this before doing an enumeration. */
static void
InitEnumerations(void)
{
  if (! newEnumPointer) {
    /* later on, we assume that newEnumPointer is non-NULL */
    newEnumSize = SHIP_NR + 1;
    newEnumPointer = (Uns16 *) MemCalloc(newEnumSize, sizeof(Uns16));
    RegisterCleanupFunction(FreeEnumerations);
  }
  newEnumIndex = 0;
}

/** Push an item on the enumeration list. Resizes the list when needed. */
static void
PushEnum(Uns16 id)
{
  if (newEnumIndex >= newEnumSize) {
    Uns16 lNewSize = newEnumSize + 500;
    newEnumPointer = MemRealloc (newEnumPointer, lNewSize * sizeof(Uns16));
    newEnumSize = lNewSize;
  }
  newEnumPointer[newEnumIndex++] = id;
}

/* Right now, all enumeration functions use drop-dead stupid algorithms
   that do nothing but an exhaustive search. Fortunately, they do so very
   quickly and for 500 ships it really doesn't take too long. A better
   way of doing things is left for future work */

/** Enumerate ships within radius.
    \param pX,pY    coordinates
    \param pRadius  radius (circle)
    \returns pointer to an array of Uns16 containing ship Ids,
    zero-terminated. This pointer is shared between the EnumerateXXX
    functions.

    Optimizations:
      We use lSimplRadius to weed out far ships early. This routine no
      longer rounds the radius; it now works exactly like PHost. */
#ifndef MICROSQUISH
Uns16 *
EnumerateShipsWithinRadius(Int16 pX, Int16 pY, double pRadius)
{
  Uns16 lShip;
  Uns16 lSimplRadius = (Uns16)(pRadius + 1);
  Int32 lRadiusSqr = (Int32)(pRadius * pRadius);
  Int16 lDistX, lDistY;

  InitEnumerations();

  for (lShip = 1; lShip <= SHIP_NR; lShip++) {
    if (!IsShipExist(lShip))
      continue;

    lDistX = abs(WrapDistX(ShipLocationX(lShip) - pX));
    if (lDistX > lSimplRadius)
      continue;

    lDistY = abs(WrapDistY(ShipLocationY(lShip) - pY));
    if (lDistY > lSimplRadius)
      continue;

    if (((Int32) lDistX * lDistX + (Int32) lDistY * lDistY) <= lRadiusSqr)
      PushEnum(lShip);
  }
  PushEnum(0);
  return newEnumPointer;
}

#endif /* MICROSQUISH */

/** Enumerate ships at a particular place.
    \param pX,pY    coordinates
    \param pRadius  radius (circle)
    \returns pointer to an array of Uns16 containing ship Ids,
    zero-terminated. This pointer is shared between the EnumerateXXX
    functions.

   Optimizations:
       none

   Real Time Burden:
       Using 500 random ship locations, running EnumerateShipsAt() for all
       500 ships uses about 0.16 seconds on a 486/66 machine (with or
       without a coprocessor). Each ship was randomly placed within the
       region [1500,1500]-[2500,2500]. */
Uns16 *
EnumerateShipsAt(Uns16 pX, Uns16 pY)
{
  Uns16 lShip;

  InitEnumerations();

  for (lShip = 1; lShip <= SHIP_NR; lShip++) {
    if (!IsShipExist(lShip))
      continue;

    if ((ShipLocationX(lShip) EQ pX)
          AND(ShipLocationY(lShip) EQ pY)) {
      PushEnum(lShip);
    }
  }
  PushEnum(0);
  return newEnumPointer;
}

/** Enumerate minefields within radius. A minefield is considered
    to be within radius if at least one point of it is within radius.
    \param pX,pY    coordinates
    \param pRadius  distance
    \returns pointer to an array of Uns16 containing minefield Ids,
    zero-terminated. This pointer is shared between the EnumerateXXX
    functions.

    Note that this function used to take an Uns16 pRadius. I have
    modified it to take a double for better precision.
    
   Optimizations:
        The absolute distance in X and Y directions is checked first to
        quickly weed out most of the candidates.

   Real Time Burden:
        Using 500 random ship locations randomly distributed over the region
        [1500,1500] - [2500,2500] and 100 minefields with centers in the
        same region and the number of minefield units randomly distributed
        between 900 and 90000 (radii between 30 and 300), running
        EnumerateMinesWithinRadius() for all 500 ships uses about 0.33
        seconds on a 486/66 with coprocessor and about 5.8 seconds without
        the coprocessor. */

#ifndef MICROSQUISH
Uns16 *
EnumerateMinesWithinRadius(Int16 pX, Int16 pY, double pRadius)
{
  Uns16 lMine;
  double lRadius;
  Int16 lDistX, lDistY;
  Uns16 lNum = GetNumMinefields();

  InitEnumerations();

  for (lMine = 1; lMine <= lNum; lMine++) {
    if (MinefieldUnits(lMine) EQ 0)
      continue;

    lRadius = MinefieldRadius(lMine) + pRadius;

    lDistX = abs(WrapDistX(MinefieldPositionX(lMine) - pX));
    if (lDistX > lRadius)
      continue;

    lDistY = abs(WrapDistY(MinefieldPositionY(lMine) - pY));
    if (lDistY > lRadius)
      continue;

    if (((Int32) lDistX * lDistX + (Int32) lDistY * lDistY)
          <= ((Uns32) lRadius * lRadius)) {
      PushEnum(lMine);
    }
  }
  PushEnum(0);
  return newEnumPointer;
}

#endif /* ! MICROSQUISH */

/** Find planet at ship.
    \returns Id of the planet where the specified ship is, or
    zero if the ship is in free space.
   
   Optimizations:
        none

   Real Time Burden:
        With 500 randomly placed ships on the region [1500,1500 - [2500,2500]
        and 500 randomly placed planets in the same region, calling
        FindPlanetAtShip() for all 500 ships uses about 0.05 seconds on
        a 486/66, with or without a coprocessor. */
Uns16
FindPlanetAtShip(Uns16 pShip)
{
  Uns16 lPlanet;
  Uns16 pX = ShipLocationX(pShip);
  Uns16 pY = ShipLocationY(pShip);

  for (lPlanet = 1; lPlanet <= PLANET_NR; lPlanet++) {
    if (!IsPlanetExist(lPlanet))
      continue;

    if ((PlanetLocationX(lPlanet) EQ pX)
          AND(PlanetLocationY(lPlanet) EQ pY)
          )
      return lPlanet;
  }
  return 0;
}

/** Find planet that does gravity wells at the specified place.
    That is the highest-id planet in range for gravity wells.
    PConfig settings are taken into account: size of gravity wells,
    round vs. square, wrap.
    \returns planet Id, or zero

   Optimizations:
        IsDistanceLTRadius is called to avoid unnecesary square roots. The
        X and Y dimensions are checked first in absolute value to weed out
        most candidates.

   Real Time Burden:
        With 500 ships placed randomly over the region [1500,1500] - [2500,2500]
        and 500 randomly placed planets over the same region, calling
        FindGravityPlanet() for all 500 ships uses about 0.11 seconds on
        a 486/66 with or without a coprocessor. */
Uns16
FindGravityPlanet(Int16 pX, Int16 pY)
{
  Uns16 lPlanet;
  Int16 lDistX, lDistY;
  Int16 lRange = gConfigInfo->GravityWellRange;

  for (lPlanet = PLANET_NR; lPlanet >= 1; lPlanet--) {
    if (!IsPlanetExist(lPlanet))
      continue;

    lDistX = abs(WrapDistX(PlanetLocationX(lPlanet) - pX));
    if (lDistX > lRange)
      continue;

    lDistY = abs(WrapDistY(PlanetLocationY(lPlanet) - pY));
    if (lDistY > lRange)
      continue;

    if ((!gConfigInfo->RoundWarpWells)
          OR(((Int32) lDistX * lDistX + (Int32) lDistY * lDistY)
                <= ((Int32) lRange * lRange))
          ) {
      return lPlanet;
    }
  }

  return 0;
}

/** Enumerate planets within radius.
    \param pX,pY    coordinates
    \param pRadius  radius (circle)
    \returns pointer to an array of Uns16 containing planet Ids,
    zero-terminated. This pointer is shared between the EnumerateXXX
    functions.

   Optimizations:
        Absolute X and Y dimensions are checked in absolute value to
        quickly weed out candidates. IsDistanceLTRadius is called to
        eliminate unnecessary square roots

   Real Time Burden:
        With 500 randomly spaced ships over the region [1500,1500]-[2500,2500]
        and 500 randomly placed planets over the same region, calling
        EnumeratePlanetsWithin() for all 500 ships uses about 0.27 seconds
        on a 486/66 with or without a coprocessor. */
Uns16 *
EnumeratePlanetsWithin(Int16 pX, Int16 pY, Uns16 pRadius)
{
  Uns16 lPlanet;
  Int16 lDistX,
    lDistY;

  InitEnumerations();

  for (lPlanet = 1; lPlanet <= PLANET_NR; lPlanet++) {
    if (!IsPlanetExist(lPlanet))
      continue;

    lDistX = abs(WrapDistX(PlanetLocationX(lPlanet) - pX));
    if (lDistX > pRadius)
      continue;

    lDistY = abs(WrapDistY(PlanetLocationY(lPlanet) - pY));
    if (lDistY > pRadius)
      continue;

    if (((Int32) lDistX * lDistX + (Int32) lDistY * lDistY)
          <= ((Int32) pRadius * pRadius)) {
      PushEnum(lPlanet);
    }
  }
  PushEnum(0);
  return newEnumPointer;
}

/** Enumerate ships at planet.
    \see EnumerateShipsAt */
Uns16 *
EnumerateShipsAtPlanet(Uns16 pPlanet)
{
  return EnumerateShipsAt(PlanetLocationX(pPlanet), PlanetLocationY(pPlanet));
}

/** Enumerate minefields covering a particular point.
    \param pX,pY  coordinates
    \returns pointer to an array of Uns16 containing minefield Ids,
    zero-terminated. This pointer is shared between the EnumerateXXX
    functions. */
#ifndef MICROSQUISH
Uns16 *
EnumerateMinesCovering(Int16 pX, Int16 pY)
{
  return EnumerateMinesWithinRadius(pX, pY, 0);
}
#endif

/** Enumerate minefields overlapping the specified minefield.
    \returns pointer to an array of Uns16 containing minefield Ids,
    zero-terminated. This pointer is shared between the EnumerateXXX
    functions. */    
#ifndef MICROSQUISH
Uns16 *
EnumerateOverlappingMines(Uns16 pMinefield)
{
  return EnumerateMinesWithinRadius(MinefieldPositionX(pMinefield),
        MinefieldPositionY(pMinefield), MinefieldRadius(pMinefield));
}
#endif

/*
 *              W R A P A R O U N D    S U P P O R T
 */

 /* LEAVE THESE as Int16's! */
static Int16 gWrapMaxX;         /* Maximum allowable x-value */
static Int16 gWrapMinX;         /* Minimum allowable x-value */
static Int16 gWrapMaxY;         /* Maximum allowable y-value */
static Int16 gWrapMinY;         /* Minimum allowable y-value */
static Int16 gWrapDimX;         /* X-Dimension wraparound size */
static Int16 gWrapDimY;         /* Y-Dimension wraparound size */
static Int16 gWrapDimX_2;       /* X-size divided by 2 */
static Int16 gWrapDimY_2;       /* Y-size divided by 2 */
static Boolean gWrap;           /* Just a mirror of

                                   gConfigInfo->AllowWraparoundMap */

static Boolean gWraparoundInitialized = False;

static void
ShutdownWraparound(void)
{
  /* Just so we re-init from a possibly-changed config file */
  gWraparoundInitialized = False;
}

static void
InitWraparound(void)
{
  if (gWraparoundInitialized)
    return;

  gWraparoundInitialized = True;
  RegisterCleanupFunction(ShutdownWraparound);

  ReinitWraparound();
}

/** Reload cached values from configuration. */
void
ReinitWraparound(void)
{
  Uns16 lVert[4];
  Uns16 lTemp;
  int i;

  passert(gConfigInfo);

  gWrap = gConfigInfo->AllowWraparoundMap;

  memcpy(lVert, gConfigInfo->WraparoundRectangle, sizeof(lVert));

  /* Ensure that left is to the left of right, and bottom is below top */
  for (i = 0; i < 2; i++) {
    if (lVert[i] > lVert[i + 2]) {
      lTemp = lVert[i];
      lVert[i] = lVert[i + 2];
      lVert[i + 2] = lTemp;
    }
  }

  gWrapMinX = lVert[0];
  gWrapMinY = lVert[1];
  gWrapMaxX = lVert[2];
  gWrapMaxY = lVert[3];

  gWrapDimX = gWrapMaxX - gWrapMinX;
  gWrapDimY = gWrapMaxY - gWrapMinY;

  gWrapDimX_2 = gWrapDimX / 2;
  gWrapDimY_2 = gWrapDimY / 2;
}

void
RewrapShipsAndMines(void)
{
  Uns16 lShip;
  Uns16 lMine;

  InitWraparound();

  /* Now go through and ensure that each ship position, minefield position,
     etc. is within the wrap boundaries. We need to do this, not only to
     check ourselves, but to enforce wraps in case external add-ons screw
     around with positions. */
  if (gWrap) {
    for (lShip = 1; lShip <= SHIP_NR; lShip++) {
      if (!IsShipExist(lShip))
        continue;

      WrapShipLocation(lShip);
    }

    for (lMine = 1; lMine <= GetNumMinefields(); lMine++) {
      if (!IsMinefieldExist(lMine))
        continue;

      WrapMineLocation(lMine);
    }
  }
}

Int16
WrapMapX(Int16 pX)
{
  InitWraparound();

  if (gWrap) {
    while (pX >= gWrapMaxX)
      pX -= gWrapDimX;
    while (pX < gWrapMinX)
      pX += gWrapDimX;
  }

  return pX;
}

Int16
WrapMapY(Int16 pY)
{
  InitWraparound();

  if (gWrap) {
    while (pY >= gWrapMaxY)
      pY -= gWrapDimY;
    while (pY < gWrapMinY)
      pY += gWrapDimY;
  }

  return pY;
}

Int16
WrapDistX(Int16 pX)
{
  InitWraparound();

  if (gWrap) {
    while (pX > gWrapDimX_2)
      pX -= gWrapDimX;
    while (pX < -gWrapDimX_2)
      pX += gWrapDimX;
  }

  return pX;
}

Int16
WrapDistY(Int16 pY)
{
  InitWraparound();

  if (gWrap) {
    while (pY > gWrapDimY_2)
      pY -= gWrapDimY;
    while (pY < -gWrapDimY_2)
      pY += gWrapDimY;
  }

  return pY;
}

Boolean
IsPointOnWrapMap(Int16 pX, Int16 pY)
{
  InitWraparound();

  if (gWrap) {
    return  (pX >= gWrapMinX)
        AND (pX <  gWrapMaxX)
        AND (pY >= gWrapMinY)
        AND (pY <  gWrapMaxY);
  }
  else
    return True;
}

Int32
DistanceSquared(Int16 pX1, Int16 pY1, Int16 pX2, Int16 pY2)
{
  Int16 lDX;
  Int16 lDY;

  InitWraparound();

  /* Ensure we are within wraparound region and that we find the closest
     distance in both X and Y directions. */
  if (gWrap) {
    lDX = WrapDistX(pX1 - pX2);
    lDY = WrapDistY(pY1 - pY2);
  }
  else {
    lDX = pX1 - pX2;
    lDY = pY1 - pY2;
  }

  return (Int32) lDX *lDX + (Int32) lDY *lDY;
}

#ifndef MICROSQUISH
double
Distance(Int16 pX1, Int16 pY1, Int16 pX2, Int16 pY2)
{
  return sqrt((double) DistanceSquared(pX1, pY1, pX2, pY2));
}
#endif

/* This routine checks whether the distance between two points is less than
   or equal to a reference radius. No floating point math is used. */

Boolean
IsDistanceLTRadius(Int16 pX1, Int16 pY1, Int16 pX2, Int16 pY2, Uns32 pRadius)
{
  return (Boolean)
        ((Uns32)DistanceSquared(pX1, pY1, pX2, pY2) <= (pRadius * pRadius));
}

/*************************************************************
  $HISTORY:$
**************************************************************/

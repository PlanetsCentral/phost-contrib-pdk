#ifdef MICROSQUISH
#include <stdlib.h>
#include <math.h>
#include <phostpdk.h>

extern Uns16 *enumPointer;

static Boolean IsDistanceLTRadius(Int16 pX1, Int16 pY1, Int16 pX2, Int16 pY2,
                                                                Uns32 pRadius)
{
    Int32 lDX = pX1 - pX2;
    Int32 lDY = pY1 - pY2;

    return (Boolean) ((lDX*lDX + lDY*lDY) <= (pRadius*pRadius));
}

Uns16 *EnumerateShipsWithinRadius(Int16 pX, Int16 pY, double pRadius)
{
    Uns16 lShip;
    Uns16 lRadius = (Uns16)(pRadius + 0.5);
    Uns16 lIndex = 0;       /* Index into enumeration list */
    Int16 lShipX, lShipY;

    for (lShip = 1; lShip <= SHIP_NR; lShip++) {
        if (!IsShipExist(lShip)) continue;

        if (      (abs((lShipX = ShipLocationX(lShip)) - pX) <= lRadius)
              && (abs((lShipY = ShipLocationY(lShip)) - pY) <= lRadius)
              && IsDistanceLTRadius(lShipX, lShipY, pX, pY, lRadius)
           ) {
            enumPointer[lIndex++] = lShip;
        }
    }
    enumPointer[lIndex] = 0;
    return enumPointer;
}

Uns16 *EnumerateMinesWithinRadius(Int16 pX, Int16 pY, Uns16 pRadius)
{
    Uns16 lMine;
    Uns16 lIndex = 0;
    Uns16 lRadius;
    Int16 lMineX, lMineY;

    for (lMine = 1; lMine <= GetNumMinefields(); lMine++) {
        if (MinefieldUnits(lMine) == 0) continue;

        lRadius = MinefieldRadius(lMine) + pRadius;
        if (    (abs((lMineX = MinefieldPositionX(lMine)) - pX) <= lRadius)
            && (abs((lMineY = MinefieldPositionY(lMine)) - pY) <= lRadius)
            && IsDistanceLTRadius(pX, pY, lMineX, lMineY, lRadius)
           ) {
            enumPointer[lIndex++] = lMine;
        }
    }
    enumPointer[lIndex] = 0;
    return enumPointer;
}

Uns16 *EnumerateMinesCovering(Int16 pX, Int16 pY)
{
    return EnumerateMinesWithinRadius(pX, pY, 0);
}

Uns16 *EnumerateOverlappingMines(Uns16 pMinefield)
{
    return EnumerateMinesWithinRadius(MinefieldPositionX(pMinefield),
                                      MinefieldPositionY(pMinefield),
                                      MinefieldRadius(pMinefield));
}

double MinefieldRadius(Uns16 pID)
{
    if (IsMinefieldExist(pID)) return sqrt((double)MinefieldUnits(pID));

    return 0;
}
#endif /* MICROSQUISH */

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
 *																  *
 *   Module:      Wormhole Module                                 *
 *	 Project:	  Portable Host 								  *
 *   File Name:   worm.c                                          *
 *	 Programmer:  Andrew Sterian								  *
 *	 Create Date:												  *
 *   Description: This module handles wormhole processing. All    *
 *                functions, from reading the wormhole file to    *
 *                doing the actual scanning and movement are      *
 *                handled here.                                   *
 *	 Functions: 												  *
 *	 Change Record: 											  *
 *																  *
 *																  *
 ******************************************************************/

#include "phostpdk.h"
#include "private.h"

static const char *WORMHOLE_FILE = "wormhole.txt";

#ifndef MICROSQUISH

/* A WormholeExt_Struct is used for reading in wormhole descriptions from
   the WORMHOLE.TXT file. */

typedef struct {
    Int16   mStartX, mStartY;
    Int16   mEndX, mEndY;
    Int16   mMass;             /* May be negative */
    float   mInstability;
    Int16 mStartWX, mStartWY;     /* These are WAYPOINTS, not displacements */
    Int16 mEndWX, mEndWY;
} Wormhole_Struct;

/* This is an array of all wormhole entries. */

static Wormhole_Struct *gWormholes = 0;
static Uns16 gNumWormholes = 0;

static void FreeWormholes(void)
{
    MemFree(gWormholes); gWormholes = 0; gNumWormholes = 0;
}

static void InitWormholes(void)
{
    if (gWormholes EQ 0) {
        gWormholes = (Wormhole_Struct *) MemCalloc(MAX_WORMHOLES, sizeof(Wormhole_Struct));
        gNumWormholes = 0;

        RegisterCleanupFunction(FreeWormholes);
    }
}

/* This routine checks a wormhole structure's parameters for conformance
   to requirements. */

static Boolean validateWormhole(Wormhole_Struct *lPtr)
{
    return
    (lPtr->mStartX >= MIN_COORDINATE AND lPtr->mStartX <= MAX_COORDINATE)
AND (lPtr->mStartY >= MIN_COORDINATE AND lPtr->mStartY <= MAX_COORDINATE)
AND (lPtr->mEndX >= MIN_COORDINATE AND lPtr->mEndX <= MAX_COORDINATE)
AND (lPtr->mEndY >= MIN_COORDINATE AND lPtr->mEndY <= MAX_COORDINATE)
AND (lPtr->mMass NEQ 0)
AND (lPtr->mInstability >= 0 AND lPtr->mInstability <= 100)
AND (lPtr->mStartWX >= MIN_COORDINATE AND lPtr->mStartWX <= MAX_COORDINATE)
AND (lPtr->mStartWY >= MIN_COORDINATE AND lPtr->mStartWY <= MAX_COORDINATE)
AND (lPtr->mEndWX >= MIN_COORDINATE AND lPtr->mEndWX <= MAX_COORDINATE)
AND (lPtr->mEndWY >= MIN_COORDINATE AND lPtr->mEndWY <= MAX_COORDINATE)
;
}

/* This is the main wormhole input function. The WORMHOLE.TXT file is read
   to obtain the wormhole descriptions. As they are read, they are entered
   into the gWormholes array. */

Boolean ReadWormholeFile(void)
{
    char lBuffer[512];
    FILE *lInfile;
    Uns16 lLinenum=0;
    Boolean lRetval = True;

    if (! gConfigInfo->AllowWormholes) return True;

    /* If the wormhole.txt file doesn't exist, then forget it */
    lInfile = OpenInputFile(WORMHOLE_FILE, GAME_DIR_ONLY | NO_MISSING_ERROR);
    if (lInfile EQ NULL) {
        if (gNonPHOSTWarnings) {
            Warning("Wormholes are enabled but no wormhole input file found");
        }
        return True;
    }
    fclose(lInfile);

    InitWormholes();

    lInfile = OpenInputFile(WORMHOLE_FILE, GAME_DIR_ONLY | TEXT_MODE);

    memset(lBuffer, 0, sizeof(lBuffer));
    while (fgets(lBuffer, 511, lInfile)) {
        int firstChar;
        Wormhole_Struct lStruct;
        int numConversions;

        lLinenum++;

        firstChar = lBuffer[strspn(lBuffer, " \t")];
        if (firstChar EQ 0 OR firstChar EQ 0x0D OR firstChar EQ 0x0A
                           OR firstChar EQ '#'  OR firstChar EQ ';') {
            continue;
        }

        /* The line is not blank and it's not a comment, hence it must
           be a wormhole spec. */

        memset(&lStruct, 0, sizeof(lStruct));

        numConversions = sscanf(lBuffer,
             "%hd %hd %hd %hd %hd %f %hd %hd %hd %hd",
             &lStruct.mStartX, &lStruct.mStartY,
             &lStruct.mEndX, &lStruct.mEndY,
             &lStruct.mMass, &lStruct.mInstability,
             &lStruct.mStartWX, &lStruct.mStartWY,
             &lStruct.mEndWX, &lStruct.mEndWY);

        switch (numConversions) {
         case 6:
            lStruct.mStartWX = lStruct.mStartX;
            /* FALL THROUGH */
         case 7:
            lStruct.mStartWY = lStruct.mStartY;
            /* FALL THROUGH */
         case 8:
            lStruct.mEndWX = lStruct.mEndX;
            /* FALL THROUGH */
         case 9:
            lStruct.mEndWY = lStruct.mEndY;
            /* FALL THROUGH */
         case 10:
            break;

         default:
            Error("Format error in wormhole file at line %u -- wormhole ignored", lLinenum);
            lRetval = False;
            continue;
        }

        if (!validateWormhole(&lStruct)) {
            Error("Wormhole parameter error at line %u -- wormhole ignored", lLinenum);
            lRetval = False;
            continue;
        }

        if (gNumWormholes >= MAX_WORMHOLES) {
            Error("Too many wormholes at line %u -- wormhole ignored", lLinenum);
            lRetval = False;
            continue;
        }

        memcpy(gWormholes + gNumWormholes, &lStruct, sizeof(Wormhole_Struct));
        gNumWormholes++;
    }

    if (!feof(lInfile)) {
        ErrorExit("Error in reading '%s'\n", WORMHOLE_FILE);
    }

    fclose(lInfile);

    return lRetval;
}

Boolean WriteWormholeFile(void)
{
    FILE *lOutfile;
    Uns16 lCount;
    Wormhole_Struct *lStruct;

    if (! gConfigInfo->AllowWormholes) return True;
    if (gNumWormholes EQ 0) return True;

    lOutfile = OpenOutputFile(WORMHOLE_FILE, GAME_DIR_ONLY | TEXT_MODE);
    for (lCount=0; lCount < gNumWormholes; lCount++) {
        lStruct = gWormholes + lCount;
        if (lStruct->mMass EQ 0) continue;

        fprintf(lOutfile,"%4hd %4hd %4hd %4hd %5hd %5.1f %4hd %4hd %4hd %4hd\n",
                 lStruct->mStartX, lStruct->mStartY,
                 lStruct->mEndX, lStruct->mEndY,
                 lStruct->mMass, lStruct->mInstability,
                 lStruct->mStartWX, lStruct->mStartWY,
                 lStruct->mEndWX, lStruct->mEndWY);
    }
    fclose(lOutfile);
    return True;
}

/*********** accessors ************/
Uns16 NumWormholes(void)
{
    return gNumWormholes;
}

Uns16 WormholeStabilityCode(Uns16 pID)
{
    float lInstability;

    InitWormholes();

    passert(pID < gNumWormholes AND gWormholes[pID].mMass NEQ 0);
    lInstability = gWormholes[pID].mInstability;

    if (lInstability <= 5) return 0;
    if (lInstability <= 15) return 1;
    if (lInstability <= 30) return 2;
    if (lInstability <= 50) return 3;
    if (lInstability <= 80) return 4;
    return 5;
}

Uns16 WormholeStartX(Uns16 pID)
{
    InitWormholes();

    passert(pID < gNumWormholes AND gWormholes[pID].mMass NEQ 0);
    return gWormholes[pID].mStartX;
}

Uns16 WormholeStartY(Uns16 pID)
{
    InitWormholes();

    passert(pID < gNumWormholes AND gWormholes[pID].mMass NEQ 0);
    return gWormholes[pID].mStartY;
}

Uns16 WormholeEndX(Uns16 pID)
{
    InitWormholes();

    passert(pID < gNumWormholes AND gWormholes[pID].mMass NEQ 0);
    return gWormholes[pID].mEndX;
}

Uns16 WormholeEndY(Uns16 pID)
{
    InitWormholes();

    passert(pID < gNumWormholes AND gWormholes[pID].mMass NEQ 0);
    return gWormholes[pID].mEndY;
}

Int16 WormholeMass(Uns16 pID)
{
    InitWormholes();

    passert(pID < gNumWormholes AND gWormholes[pID].mMass NEQ 0);
    return gWormholes[pID].mMass;
}

float WormholeInstability(Uns16 pID)
{
    InitWormholes();

    passert(pID < gNumWormholes AND gWormholes[pID].mMass NEQ 0);
    return gWormholes[pID].mInstability;
}

Uns16 WormholeWaypointStartX(Uns16 pID)
{
    InitWormholes();

    passert(pID < gNumWormholes AND gWormholes[pID].mMass NEQ 0);
    return gWormholes[pID].mStartWX;
}

Uns16 WormholeWaypointStartY(Uns16 pID)
{
    InitWormholes();

    passert(pID < gNumWormholes AND gWormholes[pID].mMass NEQ 0);
    return gWormholes[pID].mStartWY;
}

Uns16 WormholeWaypointEndX(Uns16 pID)
{
    InitWormholes();

    passert(pID < gNumWormholes AND gWormholes[pID].mMass NEQ 0);
    return gWormholes[pID].mEndWX;
}

Uns16 WormholeWaypointEndY(Uns16 pID)
{
    InitWormholes();

    passert(pID < gNumWormholes AND gWormholes[pID].mMass NEQ 0);
    return gWormholes[pID].mEndWY;
}

void PutWormholeStartX(Uns16 pID, Uns16 pX)
{
    InitWormholes();

    passert(pID < gNumWormholes);
    passert(pX >= MIN_COORDINATE AND pX <= MAX_COORDINATE);
    gWormholes[pID].mStartX = pX;
}

void PutWormholeStartY(Uns16 pID, Uns16 pY)
{
    InitWormholes();

    passert(pID < gNumWormholes);
    passert(pY >= MIN_COORDINATE AND pY <= MAX_COORDINATE);
    gWormholes[pID].mStartY = pY;
}

void PutWormholeEndX(Uns16 pID, Uns16 pX)
{
    InitWormholes();

    passert(pID < gNumWormholes);
    passert(pX >= MIN_COORDINATE AND pX <= MAX_COORDINATE);
    gWormholes[pID].mEndX = pX;
}

void PutWormholeEndY(Uns16 pID, Uns16 pY)
{
    InitWormholes();

    passert(pID < gNumWormholes);
    passert(pY >= MIN_COORDINATE AND pY <= MAX_COORDINATE);
    gWormholes[pID].mEndY = pY;
}

void PutWormholeMass(Uns16 pID, Int16 pMass)
{
    InitWormholes();

    passert(pID < gNumWormholes);
    passert(pMass NEQ 0);
    gWormholes[pID].mMass = pMass;
}

void PutWormholeInstability(Uns16 pID, float pInstability)
{
    InitWormholes();

    passert(pID < gNumWormholes);
    passert(pInstability >= 0 AND pInstability <= 100);
    gWormholes[pID].mInstability = pInstability;
}

void PutWormholeWaypointStartX(Uns16 pID, Uns16 pX)
{
    InitWormholes();

    passert(pID < gNumWormholes);
    passert(pX >= MIN_COORDINATE AND pX <= MAX_COORDINATE);
    gWormholes[pID].mStartWX = pX;
}

void PutWormholeWaypointStartY(Uns16 pID, Uns16 pY)
{
    InitWormholes();

    passert(pID < gNumWormholes);
    passert(pY >= MIN_COORDINATE AND pY <= MAX_COORDINATE);
    gWormholes[pID].mStartWY = pY;
}

void PutWormholeWaypointEndX(Uns16 pID, Uns16 pX)
{
    InitWormholes();

    passert(pID < gNumWormholes);
    passert(pX >= MIN_COORDINATE AND pX <= MAX_COORDINATE);
    gWormholes[pID].mEndWX = pX;
}

void PutWormholeWaypointEndY(Uns16 pID, Uns16 pY)
{
    InitWormholes();

    passert(pID < gNumWormholes);
    passert(pY >= MIN_COORDINATE AND pY <= MAX_COORDINATE);
    gWormholes[pID].mEndWY = pY;
}

Uns16 CreateWormhole(void)
{
    Uns16 lID;

    InitWormholes();

    for (lID = 0; lID < gNumWormholes; lID++) {
        if (gWormholes[lID].mMass EQ 0) return lID;
    }

    if (gNumWormholes >= MAX_WORMHOLES) return 0;

    memset(gWormholes + gNumWormholes, 0, sizeof(gWormholes[0]));
    gNumWormholes++;

    return gNumWormholes-1;
}

void DeleteWormhole(Uns16 pID)
{
    InitWormholes();

    passert(pID < gNumWormholes);

    memset(gWormholes + pID, 0, sizeof(gWormholes[0]));
}

#else /* MICROSQUISH */

Boolean ReadWormholeFile(void) { SquishError("ReadWormholeFile"); }
Boolean WriteWormholeFile(void) { SquishError("WriteWormholeFile"); }
void FreeWormholes(void) { SquishError("FreeWormholes"); }
Uns16 NumWormholes(void) { SquishError("NumWormholes"); }
Uns16 WormholeStabilityCode(Uns16 pID) { SquishError("WormholeStabilityCode"); }
Uns16 WormholeStartX(Uns16 pID) { SquishError("WormholeStartX"); }
Uns16 WormholeStartY(Uns16 pID) { SquishError("WormholeStartY"); }
Uns16 WormholeEndX(Uns16 pID) { SquishError("WormholeEndX"); }
Uns16 WormholeEndY(Uns16 pID) { SquishError("WormholeEndY"); }
Int16 WormholeMass(Uns16 pID) { SquishError("WormholeMass"); }
float WormholeInstability(Uns16 pID) { SquishError("WormholeInstability"); }
Uns16 WormholeWaypointStartX(Uns16 pID) { SquishError("WormholeWaypointStartX"); }
Uns16 WormholeWaypointStartY(Uns16 pID) { SquishError("WormholeWaypointStartY"); }
Uns16 WormholeWaypointEndX(Uns16 pID) { SquishError("WormholeWaypointEndX"); }
Uns16 WormholeWaypointEndY(Uns16 pID) { SquishError("WormholeWaypointEndY"); }
void PutWormholeStartX(Uns16 pID, Uns16 pX) { SquishError("PutWormholeStartX"); }
void PutWormholeStartY(Uns16 pID, Uns16 pY) { SquishError("PutWormholeStartY"); }
void PutWormholeEndX(Uns16 pID, Uns16 pX) { SquishError("PutWormholeEndX"); }
void PutWormholeEndY(Uns16 pID, Uns16 pY) { SquishError("PutWormholeEndY"); }
void PutWormholeMass(Uns16 pID, Int16 pMass) { SquishError("PutWormholeMass"); }
void PutWormholeInstability(Uns16 pID, float pInstability) { SquishError("PutWormholeInstability"); }
void PutWormholeWaypointStartX(Uns16 pID, Uns16 pX) { SquishError("PutWormholeWaypointStartX"); }
void PutWormholeWaypointStartY(Uns16 pID, Uns16 pY) { SquishError("PutWormholeWaypointStartY"); }
void PutWormholeWaypointEndX(Uns16 pID, Uns16 pX) { SquishError("PutWormholeWaypointEndX"); }
void PutWormholeWaypointEndY(Uns16 pID, Uns16 pY) { SquishError("PutWormholeWaypointEndY"); }
Uns16 CreateWormhole(void) { SquishError("CreateWormhole"); }
void DeleteWormhole(Uns16 pID) { SquishError("DeleteWormhole"); }

#endif /* MICROSQUISH */

/*************************************************************
  $HISTORY:$
**************************************************************/

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

static const char *HULLSPEC_FILE     = "hullspec.dat";
static const char *TRUEHULL_FILE     = "truehull.dat";
static Hullspec_Struct   *gHullspecPtr=0;
static Truehull_Def       gTruehull;

static void FreeHull(void)
{
    MemFree(gHullspecPtr); gHullspecPtr = 0;
}

static void InitHull(void)
{
    if (gHullspecPtr EQ 0) {
        gHullspecPtr = (Hullspec_Struct *)MemCalloc(HULL_NR+1, sizeof(Hullspec_Struct));
        memset(gTruehull, 0, sizeof(gTruehull));

        RegisterCleanupFunction(FreeHull);
    }
}

 Uns16 HullPicnumber(Uns16 pHullNr)
{
    InitHull();

	passert((pHullNr>=1) AND (pHullNr<=HULL_NR));
	return(gHullspecPtr[pHullNr].Picnumber);
}

 Uns16 HullTritCost(Uns16 pHullNr)
{
    InitHull();

    passert((pHullNr>=1) AND (pHullNr<=HULL_NR));
	return(gHullspecPtr[pHullNr].TritCost);
}

 Uns16 HullDurCost(Uns16 pHullNr)
{
    InitHull();

    passert((pHullNr>=1) AND (pHullNr<=HULL_NR));
	return(gHullspecPtr[pHullNr].DurCost);
}

 Uns16 HullMolyCost(Uns16 pHullNr)
{
    InitHull();

    passert((pHullNr>=1) AND (pHullNr<=HULL_NR));
	return(gHullspecPtr[pHullNr].MolyCost);
}

 Uns16 HullFuelCapacity(Uns16 pHullNr)
{
    InitHull();

    passert((pHullNr>=1) AND (pHullNr<=HULL_NR));
	return(gHullspecPtr[pHullNr].FuelCapacity);
}

 Uns16 HullCrewComplement(Uns16 pHullNr)
{
    InitHull();

    passert((pHullNr>=1) AND (pHullNr<=HULL_NR));
	return(gHullspecPtr[pHullNr].CrewComplement);
}

 Uns16 HullEngineNumber(Uns16 pHullNr)
{
    InitHull();

    passert((pHullNr>=1) AND (pHullNr<=HULL_NR));
	return(gHullspecPtr[pHullNr].EngineNumber);
}

 Uns16 HullMass(Uns16 pHullNr)
{
    InitHull();

    passert((pHullNr>=1) AND (pHullNr<=HULL_NR));
	return(gHullspecPtr[pHullNr].Mass);
}

 Uns16 HullTechLevel(Uns16 pHullNr)
{
    InitHull();

    passert((pHullNr>=1) AND (pHullNr<=HULL_NR));
	return(gHullspecPtr[pHullNr].TechLevel);
}

 Uns16 HullCargoCapacity(Uns16 pHullNr)
{
    InitHull();

    passert((pHullNr>=1) AND (pHullNr<=HULL_NR));
	return(gHullspecPtr[pHullNr].CargoCapacity);
}

 Uns16 HullBays(Uns16 pHullNr)
{
    InitHull();

    passert((pHullNr>=1) AND (pHullNr<=HULL_NR));
	return(gHullspecPtr[pHullNr].Bays);
}

 Uns16 HullTubeNumber(Uns16 pHullNr)
{
    InitHull();

    passert((pHullNr>=1) AND (pHullNr<=HULL_NR));
	return(gHullspecPtr[pHullNr].TubeNumber);
}

 Uns16 HullBeamNumber(Uns16 pHullNr)
{
    InitHull();

    passert((pHullNr>=1) AND (pHullNr<=HULL_NR));
	return(gHullspecPtr[pHullNr].BeamNumber);
}

 Uns16 HullMoneyCost(Uns16 pHullNr)
{
    InitHull();

    passert((pHullNr>=1) AND (pHullNr<=HULL_NR));
	return(gHullspecPtr[pHullNr].MoneyCost);
}

 Uns16 TrueHull(RaceType_Def pRace, Uns16 pHullIndex)
{
    InitHull();

    passert((pHullIndex>=1) AND (pHullIndex<=RACEHULLS));
	return(gTruehull[pRace][pHullIndex]);
}

const char* HullName(Uns16 pHullNr, char* pBuffer)
{
    static char lName[31];
	char *lPtr = pBuffer;

    InitHull();

    passert( (pHullNr>=1) AND (pHullNr<=HULL_NR) );

	if (lPtr EQ NULL) lPtr = lName; 				/* NEW */
	memcpy(lPtr, gHullspecPtr[pHullNr].Name, 30);
	lPtr[30]=0;

	return lPtr;
}

IO_Def Read_Hullspec_File(void)
{
    FILE    *lHullFile;
    IO_Def  lError = IO_SUCCESS;

    InitHull();

    if ((lHullFile = OpenInputFile(HULLSPEC_FILE, GAME_OR_ROOT_DIR | NO_MISSING_ERROR)) NEQ NULL) {
        /* NEW */
#ifdef __MSDOS__
		if (HULL_NR NEQ fread(gHullspecPtr+1, sizeof(Hullspec_Struct), HULL_NR, lHullFile)) {
            Error("Can't read HULLSPEC.DAT file");
			lError=IO_FAILURE;
		}
#else
        Uns16 lHull;
		for (lHull=1; lHull <= HULL_NR; lHull++) {
			if (! DOSReadStruct(HullspecStruct_Convert,
								NumHullspecStruct_Convert,
								gHullspecPtr+lHull,
								lHullFile)) {
                Error("Can't read HULLSPEC.DAT file");
				lError=IO_FAILURE;
				break;
			}
		}
#endif

        fclose(lHullFile);
    } else lError = IO_FAILURE;
    return(lError);
}

IO_Def Read_Truehull_File(void)
{
    FILE     *lTH_File;
    IO_Def   lError = IO_SUCCESS;
    Uns16    i;

    InitHull();

    if ((lTH_File = OpenInputFile(TRUEHULL_FILE, GAME_OR_ROOT_DIR | NO_MISSING_ERROR)) NEQ NULL) {
        for (i=1; i<=11; i++) {
            if (1 NEQ fread(&gTruehull[i][1], RACEHULLS*sizeof(gTruehull[0][0]), 1, lTH_File)) {
                Error("Can't read file '%s'", TRUEHULL_FILE);
                lError=IO_FAILURE;
                break;
            }
            EndianSwap16(&gTruehull[i][1], RACEHULLS);
        }

        fclose(lTH_File);
    } else lError = IO_FAILURE;
    return(lError);
}

/*************************************************************
  $HISTORY:$
**************************************************************/

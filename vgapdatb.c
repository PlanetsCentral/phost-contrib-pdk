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

static const char *XYPLAN_FILE       = "xyplan.dat";
XYData_Struct     *gXYPlanPtr=0;

static void FreeXY(void)
{
    MemFree(gXYPlanPtr); gXYPlanPtr = 0;
}

static void InitXY(void)
{
    if (gXYPlanPtr EQ 0) {
        gXYPlanPtr = (XYData_Struct *)MemCalloc(PLANET_NR+1, sizeof(XYData_Struct));

        RegisterCleanupFunction(FreeXY);
    }
}

 Uns16 PlanetLocationX(Uns16 pID)
{
    InitXY();

    passert ( (pID >= 1) AND (pID <= PLANET_NR) );
    return gXYPlanPtr[pID].X;
}

 Uns16 PlanetLocationY(Uns16 pID)
{
    InitXY();

    passert ( (pID >= 1) AND (pID <= PLANET_NR) );
    return gXYPlanPtr[pID].Y;
}

IO_Def Read_Xyplan_File(void)
{
    FILE    *XYPlanFile;
    IO_Def  lError = IO_SUCCESS;

    InitXY();

    if ((XYPlanFile = OpenInputFile(XYPLAN_FILE, GAME_OR_ROOT_DIR | NO_MISSING_ERROR)) NEQ NULL) {
#ifdef __MSDOS__
		if (PLANET_NR NEQ fread(gXYPlanPtr+1, sizeof(XYData_Struct), PLANET_NR, XYPlanFile)) {
            Error("Can't read file '%s'", XYPLAN_FILE);
			lError=IO_FAILURE;
		}
#else
        Uns16 lPlanet;
		for (lPlanet=1; lPlanet <= PLANET_NR; lPlanet++) {
			if (! DOSReadStruct(XyplanStruct_Convert,
								NumXyplanStruct_Convert,
								gXYPlanPtr+lPlanet,
								XYPlanFile)) {
                Error("Can't read file '%s'", XYPLAN_FILE);
				lError=IO_FAILURE;
				break;
			}
		}
#endif
        fclose(XYPlanFile);
    } else lError = IO_FAILURE;
    return(lError);
}

/*************************************************************
  $HISTORY:$
**************************************************************/

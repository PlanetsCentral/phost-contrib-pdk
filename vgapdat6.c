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

static const char *ENGSPEC_FILE      = "engspec.dat";
static Engspec_Struct    *gEngspecPtr=0;

static void FreeEng(void)
{
    MemFree(gEngspecPtr); gEngspecPtr = 0;
}

static void InitEng(void)
{
    if (gEngspecPtr EQ 0) {
        gEngspecPtr = (Engspec_Struct *)MemCalloc(ENGINE_NR+1, sizeof(Engspec_Struct));

        RegisterCleanupFunction(FreeEng);
    }
}

 char* EngineName(Uns16 pEngNr, char* pBuffer)
{
    InitEng();

    passert((pEngNr>=1) AND (pEngNr<=ENGINE_NR) AND (pBuffer NEQ NULL));

    memcpy(pBuffer,gEngspecPtr[pEngNr].Name,20);
    pBuffer[20]=0;
    return(pBuffer);
}

 Uns16 EngMoneyCost(Uns16 pEngNr)
{
    InitEng();

    passert((pEngNr>=1) AND (pEngNr<=ENGINE_NR));
    return(gEngspecPtr[pEngNr].MoneyCost);
}

 Uns16 EngTritCost(Uns16 pEngNr)
{
    InitEng();

    passert((pEngNr>=1) AND (pEngNr<=ENGINE_NR));
    return(gEngspecPtr[pEngNr].TritCost);
}

 Uns16 EngDurCost(Uns16 pEngNr)
{
    InitEng();

    passert((pEngNr>=1) AND (pEngNr<=ENGINE_NR));
    return(gEngspecPtr[pEngNr].DurCost);
}

 Uns16 EngMolyCost(Uns16 pEngNr)
{
    InitEng();

    passert((pEngNr>=1) AND (pEngNr<=ENGINE_NR));
    return(gEngspecPtr[pEngNr].MolyCost);
}

 Uns16 EngTechLevel(Uns16 pEngNr)
{
    InitEng();

    passert((pEngNr>=1) AND (pEngNr<=ENGINE_NR));
    return(gEngspecPtr[pEngNr].TechLevel);
}

 Uns32 EngFuelConsumption(Uns16 pEngNr, Uns16 pSpeed)
{
    InitEng();

    passert((pEngNr>=1) AND (pEngNr<=ENGINE_NR) AND (pSpeed<=MAX_SPEED));

    /* NEW */
    /* NOTE: Speed is 0 to 9 but the fuel consumption array only has
             9 elements. We must handle speed==0 specially and then
             subtract 1 from the given speed */

    if (pSpeed EQ 0) return 0;
    return(gEngspecPtr[pEngNr].FuelConsumption[pSpeed-1]);
}

IO_Def Read_Engspec_File(void)
{
    FILE    *lEngFile;
    IO_Def  lError = IO_SUCCESS;

    InitEng();

    if ((lEngFile = OpenInputFile(ENGSPEC_FILE, GAME_OR_ROOT_DIR | NO_MISSING_ERROR)) NEQ NULL) {
#ifdef __MSDOS__
		if (ENGINE_NR NEQ fread(gEngspecPtr+1, sizeof(Engspec_Struct), ENGINE_NR, lEngFile)) {
            Error("Can't read file '%s'", ENGSPEC_FILE);
			lError=IO_FAILURE;
		}
#else
        Uns16 lEng;
		for (lEng=1; lEng <= ENGINE_NR; lEng++) {
			if (! DOSReadStruct(EngspecStruct_Convert,
								NumEngspecStruct_Convert,
								gEngspecPtr+lEng,
								lEngFile)) {
                Error("Can't read file '%s'", ENGSPEC_FILE);
				lError=IO_FAILURE;
				break;
			}
		}
#endif

        fclose(lEngFile);
    } else lError = IO_FAILURE;
    return(lError);
}


/*************************************************************
  $HISTORY:$
**************************************************************/

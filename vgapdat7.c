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

static const char *TORPSPEC_FILE     = "torpspec.dat";
static Torpspec_Struct   *gTorpspecPtr=0;

static void FreeTorp(void)
{
    MemFree(gTorpspecPtr); gTorpspecPtr = 0;
}

static void InitTorp(void)
{
    if (gTorpspecPtr EQ 0) {
        gTorpspecPtr = (Torpspec_Struct *)MemCalloc(TORP_NR+1, sizeof(Torpspec_Struct));

        RegisterCleanupFunction(FreeTorp);
    }
}

 char* TorpName(Uns16 pTorpNr, char* pBuffer)
{
    InitTorp();

    passert((pTorpNr>=1) AND (pTorpNr<=TORP_NR) AND (pBuffer NEQ NULL));
    memcpy(pBuffer,gTorpspecPtr[pTorpNr].Name,20);
    pBuffer[20]=0;
    return(pBuffer);
}

 Uns16 TorpTorpCost(Uns16 pTorpNr)
{
    InitTorp();

    passert((pTorpNr>=1) AND (pTorpNr<=TORP_NR));
    return(gTorpspecPtr[pTorpNr].TorpCost);
}

 Uns16 TorpTubeCost(Uns16 pTorpNr)
{
    InitTorp();

    passert((pTorpNr>=1) AND (pTorpNr<=TORP_NR));
    return(gTorpspecPtr[pTorpNr].TubeCost);
}

 Uns16 TorpTritCost(Uns16 pTorpNr)
{
    InitTorp();

    passert((pTorpNr>=1) AND (pTorpNr<=TORP_NR));
    return(gTorpspecPtr[pTorpNr].TubeTritCost);
}

 Uns16 TorpDurCost(Uns16 pTorpNr)
{
    InitTorp();

    passert((pTorpNr>=1) AND (pTorpNr<=TORP_NR));
    return(gTorpspecPtr[pTorpNr].TubeDurCost);
}

 Uns16 TorpMolyCost(Uns16 pTorpNr)
{
    InitTorp();

    passert((pTorpNr>=1) AND (pTorpNr<=TORP_NR));
    return(gTorpspecPtr[pTorpNr].TubeMolyCost);
}

 Uns16 TorpTubeMass(Uns16 pTorpNr)
{
    InitTorp();

    passert((pTorpNr>=1) AND (pTorpNr<=TORP_NR));
    return(gTorpspecPtr[pTorpNr].TubeMass);
}

 Uns16 TorpTechLevel(Uns16 pTorpNr)
{
    InitTorp();

    passert((pTorpNr>=1) AND (pTorpNr<=TORP_NR));
    return(gTorpspecPtr[pTorpNr].TechLevel);
}

 Uns16 TorpKillPower(Uns16 pTorpNr)
{
    InitTorp();

    passert((pTorpNr>=1) AND (pTorpNr<=TORP_NR));
    return(gTorpspecPtr[pTorpNr].KillPower);
}

 Uns16 TorpDestructivePower(Uns16 pTorpNr)
{
    InitTorp();

    passert((pTorpNr>=1) AND (pTorpNr<=TORP_NR));
    return(gTorpspecPtr[pTorpNr].DestructivePower);
}

IO_Def Read_Torpspec_File(void)
{
    FILE    *lTorpFile;
    IO_Def  lError = IO_SUCCESS;

    InitTorp();

    if ((lTorpFile = OpenInputFile(TORPSPEC_FILE, GAME_OR_ROOT_DIR | NO_MISSING_ERROR)) NEQ NULL) {
#ifdef __MSDOS__
		if (TORP_NR NEQ fread(gTorpspecPtr+1, sizeof(Torpspec_Struct), TORP_NR, lTorpFile)) {
			Error("Unable to read from '%s'", TORPSPEC_FILE);
			lError=IO_FAILURE;
		}
#else
        Uns16 lTorp;
		for (lTorp=1; lTorp <= TORP_NR; lTorp++) {
			if (! DOSReadStruct(TorpspecStruct_Convert,
								NumTorpspecStruct_Convert,
								gTorpspecPtr+lTorp,
								lTorpFile)) {
                Error("Can't read file '%s'", TORPSPEC_FILE);
				lError=IO_FAILURE;
				break;
			}
		}
#endif

        fclose(lTorpFile);
    } else lError = IO_FAILURE;
    return(lError);
}


/*************************************************************
  $HISTORY:$
**************************************************************/

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

static const char *BEAMSPEC_FILE     = "beamspec.dat";
static Beamspec_Struct   *gBeamspecPtr=0;

static void FreeBeam(void)
{
    MemFree(gBeamspecPtr); gBeamspecPtr = 0;
}

static void InitBeam(void)
{
    if (gBeamspecPtr EQ 0) {
        gBeamspecPtr = (Beamspec_Struct *)MemCalloc(BEAM_NR+1, sizeof(Beamspec_Struct));

        RegisterCleanupFunction(FreeBeam);
    }
}

 char* BeamName(Uns16 pBeamNr, char* pBuffer)
{
    static char lName[21];
    char *lPtr = pBuffer;

    InitBeam();

    passert((pBeamNr>=1) AND (pBeamNr<=BEAM_NR));
    memcpy(lPtr, gBeamspecPtr[pBeamNr].Name, 20);
    lPtr[20] = 0;
    return lPtr;
}

 Uns16 BeamMoneyCost(Uns16 pBeamNr)
{
    InitBeam();

    passert((pBeamNr>=1) AND (pBeamNr<=BEAM_NR));
    return(gBeamspecPtr[pBeamNr].MoneyCost);
}

 Uns16 BeamTritCost(Uns16 pBeamNr)
{
    InitBeam();

    passert((pBeamNr>=1) AND (pBeamNr<=BEAM_NR));
    return(gBeamspecPtr[pBeamNr].TritCost);
}

 Uns16 BeamDurCost(Uns16 pBeamNr)
{
    InitBeam();

    passert((pBeamNr>=1) AND (pBeamNr<=BEAM_NR));
    return(gBeamspecPtr[pBeamNr].DurCost);
}

 Uns16 BeamMolyCost(Uns16 pBeamNr)
{
    InitBeam();

    passert((pBeamNr>=1) AND (pBeamNr<=BEAM_NR));
    return(gBeamspecPtr[pBeamNr].MolyCost);
}

 Uns16 BeamMass(Uns16 pBeamNr)
{
    InitBeam();

    passert((pBeamNr>=1) AND (pBeamNr<=BEAM_NR));
    return(gBeamspecPtr[pBeamNr].Mass);
}

 Uns16 BeamTechLevel(Uns16 pBeamNr)
{
    InitBeam();

    passert((pBeamNr>=1) AND (pBeamNr<=BEAM_NR));
    return(gBeamspecPtr[pBeamNr].TechLevel);
}

 Uns16 BeamKillPower(Uns16 pBeamNr)
{
    InitBeam();

    passert((pBeamNr>=1) AND (pBeamNr<=BEAM_NR));
    return(gBeamspecPtr[pBeamNr].KillPower);
}

 Uns16 BeamDestructivePower(Uns16 pBeamNr)
{
    InitBeam();

    passert((pBeamNr>=1) AND (pBeamNr<=BEAM_NR));
    return(gBeamspecPtr[pBeamNr].DestructivePower);
}

IO_Def Read_Beamspec_File(void)
{
    FILE    *lBeamFile;
    IO_Def  lError = IO_SUCCESS;

    InitBeam();

    if ((lBeamFile = OpenInputFile(BEAMSPEC_FILE, GAME_OR_ROOT_DIR | NO_MISSING_ERROR)) NEQ NULL) {
#ifdef __MSDOS__
		if (BEAM_NR NEQ fread(gBeamspecPtr+1, sizeof(Beamspec_Struct), BEAM_NR, lBeamFile)) {
			Error("Unable to read from '%s'", BEAMSPEC_FILE);
			lError=IO_FAILURE;
		}
#else
        Uns16 lBeam;
		for (lBeam=1; lBeam <= BEAM_NR; lBeam++) {
			if (! DOSReadStruct(BeamspecStruct_Convert,
								NumBeamspecStruct_Convert,
								gBeamspecPtr+lBeam,
								lBeamFile)) {
                Error("Can't read file '%s'", BEAMSPEC_FILE);
				lError=IO_FAILURE;
				break;
			}
		}
#endif

        fclose(lBeamFile);
    } else lError = IO_FAILURE;
    return(lError);
}

/*************************************************************
  $HISTORY:$
**************************************************************/

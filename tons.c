/**
  *  \file tons.c
  *  \brief "tons.hst" scores
  *  \author Stefan Reuther
  *
  *  This one does tons.hst handling. Nothing complicated happens
  *  here.
  *
  *  All files in this distribution are Copyright (C) 1995-2004 by the
  *  program authors: Andrew Sterian, Thomas Voigt, Steffen Pietsch,
  *  Stefan Reuther. You can reach the PHOST team via email
  *  (phost@gmx.net, support@phost.de).
  *
  *  This program is free software; you can redistribute it and/or
  *  modify it under the terms of the GNU General Public License as
  *  published by the Free Software Foundation; either version 2 of
  *  the License, or (at your option) any later version.
  *  
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  *  General Public License for more details.
  *  
  *  You should have received a copy of the GNU General Public License
  *  along with this program; if not, write to the Free Software
  *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
  *  02111-1307, USA.
  */

#include "phostpdk.h"
#include "private.h"

/** Tons file. */
static Uns32 gTons[4 * OLD_RACE_NR];

/** Name of tons score file. */
static const char TONS_FILE[] = "tons.hst";

static Boolean gTonsDirty = False;

/** Initialize this module. */
void
ReadTonsFile(void)
{
    int lRead = 0;
    FILE* lFile;

    lFile = OpenInputFile(TONS_FILE, NO_MISSING_ERROR | GAME_DIR_ONLY);
    if (lFile) {
        lRead = fread(&gTons, sizeof(Uns32), OLD_RACE_NR * 4, lFile);
        EndianSwap32(&gTons, lRead);
        fclose(lFile);
    }

    /* zero out partial reads */
    while (lRead < OLD_RACE_NR * 4)
        gTons[lRead++] = 0;

    gTonsDirty = False;
}

/** Shut down this module, and write out score. */
void
WriteTonsFile(void)
{
    Uns32 lAllTons[OLD_RACE_NR * 4];
    FILE* lFile;

    if (gTonsDirty) {
        /* write it out */
        lFile = OpenOutputFile(TONS_FILE, GAME_DIR_ONLY);
        memcpy(lAllTons, gTons, sizeof(gTons));
        EndianSwap32(lAllTons, 4*OLD_RACE_NR);
        if (fwrite(&lAllTons, sizeof(Uns32), OLD_RACE_NR*4, lFile) != OLD_RACE_NR*4)
            Error("Can't write '%s'", TONS_FILE);
        fclose(lFile);
    }
    gTonsDirty = False;
}

/** Add score. */
void
AddToTonsScore(RaceType_Def pWinner, RaceType_Def pLoser, Uns32 pTonnage)
{
    if (pWinner > 0 && pWinner <= OLD_RACE_NR) {
        gTons[pWinner-1]               += pTonnage;
        gTons[pWinner-1 + OLD_RACE_NR] += pTonnage;
    }
    if (pLoser > 0 && pLoser <= OLD_RACE_NR) {
        gTons[pLoser-1 + 2*OLD_RACE_NR] += pTonnage;
        gTons[pLoser-1 + 3*OLD_RACE_NR] += pTonnage;
    }
    gTonsDirty = True;
}

Uns32
GetTons(enum Tons pWhich, RaceType_Def pPlayer)
{
    if (pPlayer > 0 && pPlayer <= OLD_RACE_NR) {
        return gTons[OLD_RACE_NR*pWhich + pPlayer-1];
    } else {
        return 0;
    }
}

void
PutTons(enum Tons pWhich, RaceType_Def pPlayer, Uns32 pValue)
{
    passert (pPlayer > 0 && pPlayer <= OLD_RACE_NR);

    gTonsDirty = True;
    gTons[OLD_RACE_NR*pWhich + pPlayer-1] = pValue;
}

void
ClearThisTurnTons(void)
{
    int i;

    for (i = 0; i < OLD_RACE_NR; ++i)
        gTons[i + OLD_RACE_NR] = gTons[i + 3*OLD_RACE_NR] = 0;
    gTonsDirty = True;
}

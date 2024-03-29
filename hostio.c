
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

static const char NEXTTURN_FILE[] = "nextturn.hst";
static const char LASTTURN_FILE[] = "lastturn.hst";

/* True static data (i.e., takes up space in DATA segment) */
static Turntime_Struct gTurntime;
static Turntime_Struct gAuxTime;

IO_Def
Read_Turntime_File(void)
{
    FILE *lTurntimeFile;
    IO_Def lError = IO_SUCCESS;

    /* THost, PHost 4.0: nextturn.hst is new timestamp (in RSTs)
                         lastturn.hst is old timestamp (in TRNs)
       PHost 1.x .. 3.x: both files contain new timestamp

       Unfortunately, only NEXTTURN_FILE contains the turn number. */
    Uns16 lMajor, lMinor;
    GameFilesVersion(&lMajor, &lMinor);

    gNewlyMastered = False;
    if ((lTurntimeFile = OpenInputFile(NEXTTURN_FILE, GAME_DIR_ONLY | NO_MISSING_ERROR)) NEQ NULL) {
        if (!DOSReadStruct(TurntimeStruct_Convert, NumTurntimeStruct_Convert,
                           &gTurntime, lTurntimeFile)) {
            /* If nextturn.hst exists but is 0 length, then this is a newly
               MASTER-ed game */
            gNewlyMastered = True;
        }
        fclose(lTurntimeFile);
    } else
        gNewlyMastered = True;

    gAuxTime = gTurntime;
    if (gNewlyMastered) {
        /* Newly mastered, and host never ran, so previous turn does not exist. */
        memset(&gTurntime, 0, sizeof(gTurntime));
        memset(&gAuxTime, 0, sizeof(gAuxTime));
    } else if (gUsingTHost || lMajor >= 4) {
        /* Get real timestamp.
           We have three cases:
           (a) During a host-run in mid-game, LASTTURN.HST contains the previous turn number which is used to mark AUXDATA.HST,
               so we need to set gAuxTime accordingly.
           (b) During the first host-run, LASTTURN.HST may not exist or be empty,
               so we need to use an all-zero gAuxTime, as above.
           (c) Outside a host run, NEXTTURN.HST and LASTTURN.HST are the same. */
        Boolean ok = False;
        lTurntimeFile = OpenInputFile(LASTTURN_FILE, GAME_DIR_ONLY | NO_MISSING_ERROR);
        if (lTurntimeFile) {
            ok = (fread(&gTurntime.HostTime, sizeof gTurntime.HostTime, 1, lTurntimeFile) == 1);
            fclose(lTurntimeFile);
        }

        if (!ok) {
            /* case (b) */
            memset(&gAuxTime, 0, sizeof(gAuxTime));
        } else if (memcmp(gAuxTime.HostTime, gTurntime.HostTime, sizeof gTurntime.HostTime) != 0) {
            /* case (a) */
            gAuxTime = gTurntime;
            gAuxTime.TurnNumber--;
        } else {
            /* case (c) */
        }
    }

    return (lError);
}

char *
HostTimeStamp(char *pTimeStamp)
{
  passert(pTimeStamp NEQ NULL);
  memcpy(pTimeStamp, gTurntime.HostTime, sizeof(gTurntime.HostTime));
  return pTimeStamp;
}

Uns16
TurnNumber(void)
{
  return gTurntime.TurnNumber;
}

const Turntime_Struct *
RawTurnTime(void)
{
  return &gTurntime;
}

/* Timestamp expected in auxdata file */
const Turntime_Struct *
RawAuxTime(void)
{
  return &gAuxTime;
}

/*************************************************************
  $HISTORY:$
**************************************************************/

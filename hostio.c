
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

static const char *NEXTTURN_FILE = "nextturn.hst";
static const char *LASTTURN_FILE = "lastturn.hst";

/* True static data (i.e., takes up space in DATA segment) */
static Turntime_Struct gTurntime;

IO_Def
Read_Turntime_File(void)
{
  FILE *lTurntimeFile;
  IO_Def lError = IO_SUCCESS;

  if ((lTurntimeFile =
              OpenInputFile(NEXTTURN_FILE,
           GAME_DIR_ONLY | NO_MISSING_ERROR)) NEQ NULL) {
#ifdef __MSDOS__
    if (1 NEQ fread(&gTurntime, sizeof(gTurntime), 1, lTurntimeFile)) {
#else
    if (!DOSReadStruct(TurntimeStruct_Convert, NumTurntimeStruct_Convert,
                &gTurntime, lTurntimeFile)) {
#endif
      /* If nextturn.hst exists but is 0 length, then this is a newly
         MASTER-ed game */
      gNewlyMastered = True;
    }
    fclose(lTurntimeFile);
  }
  else
    gNewlyMastered = True;

  if (gNewlyMastered) {
    memset(&gTurntime, 0, sizeof(gTurntime));
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

/*************************************************************
  $HISTORY:$
**************************************************************/

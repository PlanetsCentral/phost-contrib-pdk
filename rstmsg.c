
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

#include <stdio.h>
#include "squishio.h"
#include <stdlib.h>
#include "phostpdk.h"
#include "private.h"

Boolean
WriteMessageToRST(RaceType_Def pRace, int pArgc, const char *pArgv[])
{
  Uns32 lTotalLength,
    lOldMsgLength,
    lOldRSTLength;
  Uns16 lCount;
  FILE *lOldRST,
   *lNewRST;
  char lOldFileName[32],
    lNewFileName[32];
  Uns32 lTempUns32;
  char lVer[8];
  Uns32 lMdataStart,
    lShipxyStart;
  Uns16 lTotalMessages;
  Uns16 lLength;
  char *lTmpMsg;

  passert(pArgc >= 0);
  passert(pArgv NEQ 0);

  if (pArgc EQ 0)
    return True;

  /* Compute length of increase in message area */
  for (lCount = 0, lTotalLength = 0; lCount < pArgc; lCount++) {
    lTotalLength += strlen(pArgv[lCount]);
  }

  /* Open up our files and make backups */
  sprintf(lOldFileName, "player%u.bak", pRace);
  sprintf(lNewFileName, "player%u.rst", pRace);

  RemoveGameFile(lOldFileName);
  if (!RenameGameFile(lNewFileName, lOldFileName)) {
    Error("Unable to rename '%s' to '%s' in the game directory.",
          lNewFileName, lOldFileName);
    return False;
  }

  /* Now open up our backup file name as the source, and the RST file as the
     destination. */
  lOldRST = OpenInputFile(lOldFileName, GAME_DIR_ONLY);
  lNewRST = OpenOutputFile(lNewFileName, GAME_DIR_ONLY);

  /* Compute the size of the old file. We'll need this later. */
  lOldRSTLength = FileLength(lOldRST);

  /* First thing to do, copy over the file start locations for the first 4
     sections (ship, target, pdata, bdata). */
  if (!CopyFileToFile(lOldRST, lNewRST, 4 * sizeof(Uns32)))
    return False;

  /* Copy over the mdata start position, but save it too */
  if (!DOSRead32(&lMdataStart, 1, lOldRST))
    goto bad_read;
  if (!DOSWrite32(&lMdataStart, 1, lNewRST))
    goto bad_write;

  /* Likewise, remember the starting position of shipxy.dat. This is where we 
     begin placing our new messages. */
  if (!DOSRead32(&lShipxyStart, 1, lOldRST))
    goto bad_read;
  lShipxyStart += lTotalLength + pArgc * 6;
  if (!DOSWrite32(&lShipxyStart, 1, lNewRST))
    goto bad_write;

  /* Now set lShipxyStart to simply the position (plus 1) of where we'll
     write our first new message. We need this for writing message headers. */
  lShipxyStart -= lTotalLength;

  /* Now read in the last 2 start locations and add the offset size to them */
  for (lCount = 0; lCount < 2; lCount++) {
    if (!DOSRead32(&lTempUns32, 1, lOldRST))
      goto bad_read;

    /* Add the size of the message plus 6 bytes for the header */
    lTempUns32 += lTotalLength + pArgc * 6;
    if (!DOSWrite32(&lTempUns32, 1, lNewRST))
      goto bad_write;
  }

  /* OK, we're sitting at just past the start offsets. Now handle the WinPlan
     case. If the next 6 bytes is 'VER3.5' then we have to update two more
     offsets. The first 4-byte offset is just past VER3.5XX, then comes 4
     dummy bytes, then another 4-byte offset to update. */

  if (8 NEQ fread(lVer, 1, 8, lOldRST))
    goto bad_read;
  if (!memcmp(lVer, "VER3.5", 6)) {
    /* It's WinPlan. The next 4 bytes is an offset that must be udpated. */
    if (!DOSRead32(&lTempUns32, 1, lOldRST))
      goto bad_read;
    lTempUns32 += lTotalLength + pArgc * 6;

    /* Write out version and offset. */
    if (8 NEQ fwrite(lVer, 1, 8, lNewRST))
      goto bad_write;
    if (!DOSWrite32(&lTempUns32, 1, lNewRST))
      goto bad_write;

    /* Copy over 4 dummy bytes */
    if (!DOSRead32(&lTempUns32, 1, lOldRST))
      goto bad_read;
    if (!DOSWrite32(&lTempUns32, 1, lNewRST))
      goto bad_write;

    /* Now update the next 4-byte offset */
    if (!DOSRead32(&lTempUns32, 1, lOldRST))
      goto bad_read;
    lTempUns32 += lTotalLength + pArgc * 6;
    if (!DOSWrite32(&lTempUns32, 1, lNewRST))
      goto bad_write;
  }
  else {
    /* We read part of ship.dat. Who cares, just write it out */
    if (8 NEQ fwrite(lVer, 1, 8, lNewRST))
      goto bad_write;
  }

  /* Now, copy over as many bytes as we need to get us from where we are to
     the mdata section. Remember, MdataStart is excess 1. */
  if (!CopyFileToFile(lOldRST, lNewRST, lMdataStart - 1 - ftell(lOldRST)))
    return False;

  /* Here we have a count of the number of messages. Bump it up. */
  if (!DOSRead16(&lTotalMessages, 1, lOldRST))
    goto bad_read;
  lTotalMessages += pArgc;
  if (!DOSWrite16(&lTotalMessages, 1, lNewRST))
    goto bad_write;

  /* Now go through message by message and update offsets. Do this only for
     the original messages. Also, count up the total number of bytes occupied 
     by the old messages. */

  lOldMsgLength = 0;
  for (lCount = 0; lCount < (lTotalMessages - pArgc); lCount++) {
    if (!DOSRead32(&lTempUns32, 1, lOldRST))
      goto bad_read;
    lTempUns32 += pArgc * 6;
    if (!DOSWrite32(&lTempUns32, 1, lNewRST))
      goto bad_write;

    if (!DOSRead16(&lLength, 1, lOldRST))
      goto bad_read;
    lOldMsgLength += lLength;
    if (!DOSWrite16(&lLength, 1, lNewRST))
      goto bad_write;
  }

  /* Now add header info for the new messages. lShipxyStart contains the file 
     position where our new messages are to be placed. Remember that we store 
     our offsets excess 1. */

  for (; lCount < lTotalMessages; lCount++) {
    if (!DOSWrite32(&lShipxyStart, 1, lNewRST))
      goto bad_write;

    lLength = strlen(pArgv[lCount - lTotalMessages + pArgc]);
    lShipxyStart += lLength;

    if (!DOSWrite16(&lLength, 1, lNewRST))
      goto bad_write;
  }

  /* Copy old message bodies to new file. Add 1 so we copy over the dummy
     byte too. */
  if (!CopyFileToFile(lOldRST, lNewRST, lOldMsgLength + 1))
    return False;

  /* Finally we can start writing out new message bodies. Allocate a string
     big enough to hold 20 lines of 42 characters per line. Bumped this up to 
     40 lines just to be safe. */
  lTmpMsg = MemCalloc(40 * 42 + 1, 1);

  for (lCount = 0; lCount < pArgc; lCount++) {
    Uns16 lMsgLen;

    /* Copy the message over to a temp area. */
    strncpy(lTmpMsg, pArgv[lCount], 40 * 42);

    /* Encrypt it. */
    lMsgLen = strlen(lTmpMsg);
    for (lLength = 0; lLength < lMsgLen; lLength++) {
      lTmpMsg[lLength] += 13;
    }

    /* Write it out. */
    if (lMsgLen NEQ fwrite(lTmpMsg, 1, lMsgLen, lNewRST))
      goto bad_write;
  }
  MemFree(lTmpMsg);
  lTmpMsg = 0;

  /* Now just copy over everything from the current position to end-of-file. */
  if (!CopyFileToFile(lOldRST, lNewRST, lOldRSTLength - ftell(lOldRST)))
    return False;

  fclose(lOldRST);
  fclose(lNewRST);

  return True;

bad_read:
  Error("Can't read from '%s'", lOldFileName);
  return False;

bad_write:
  Error("Can't write to '%s'", lNewFileName);
  return False;
}

/*************************************************************
  $HISTORY:$
**************************************************************/

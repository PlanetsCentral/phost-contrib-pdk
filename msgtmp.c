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
 *   Module:      AUXHOST Message Generation                      *
 *	 Project:	  Portable Host 								  *
 *	 File Name:   vgapmsg.c 									  *
 *	 Programmer:  Andrew Sterian								  *
 *	 Create Date:												  *
 *	 Description: This module presents an interface for other	  *
 *                modules to use in generating player messages    *
 *                during AUXHOST execution. Thus, the messages    *
 *                are written directly to the MESSPNT.TMP and     *
 *                MESS.TMP files.                                 *
 *	 Functions: 												  *
 *	 Change Record: 											  *
 *																  *
 *																  *
 ******************************************************************/

#include "phostpdk.h"
#include "private.h"

static const char *TMP_MSG_POINTER_FILE = "messpnt.tmp";
static const char *TMP_MSG_FILE         = "mess.tmp";

static Uns16 gNumExtMessages = 0;
static FILE *gPointerFile = 0;
static FILE *gMessagesFile = 0;
static Uns32 gMsgFilePointer = 0;

static Boolean gInitialized = False;

static void AUXHOSTMessageCleanup(void);

static void InitializeMessages(void)
{
    if (gInitialized) return;

    RegisterCleanupFunction(AUXHOSTMessageCleanup);

    gInitialized = True;
}

static void AUXHOSTMessageCleanup(void)
{
    if (gPointerFile NEQ 0) {
        rewind(gPointerFile);
        (void) DOSWrite16(&gNumExtMessages, 1, gPointerFile);
        fclose(gPointerFile);
        gPointerFile = 0;
    }
    if (gMessagesFile NEQ 0) {
        fclose(gMessagesFile);
        gMessagesFile = 0;
    }

    gInitialized = False;
}

Boolean WriteAUXHOSTMessage(RaceType_Def pRace, const char *pMessage)
{
    Uns16  lLength;
    char   *lStr=0;
    Uns16  lCount;
    ExtMessage_Struct lStruct;

    InitializeMessages();

    passert(pMessage NEQ 0);
    lLength = strlen(pMessage);
    if (lLength EQ 0) return True;

    lStr = (char *)MemCalloc(lLength+2, 1);

    strcpy(lStr, pMessage);
    if (lStr[lLength-1] NEQ 0x0D) lStr[lLength++] = 0x0D;

    for (lCount = 0; lCount < lLength; lCount++) {
        lStr[lCount] += 13;
    }

    if (gPointerFile EQ 0) {
        Uns32 lFPos;

        gPointerFile = OpenUpdateFile(TMP_MSG_POINTER_FILE, GAME_DIR_ONLY | REWRITE_MODE);
        rewind(gPointerFile);

        if (1 NEQ fread(&gNumExtMessages, sizeof(gNumExtMessages), 1, gPointerFile)) {
            /* Must have no messages */
            gNumExtMessages = 0;
            rewind(gPointerFile);
            (void)fwrite(&gNumExtMessages, sizeof(gNumExtMessages), 1, gPointerFile);
        }
        EndianSwap16(&gNumExtMessages, 1);

        lFPos = sizeof(gNumExtMessages) + gNumExtMessages*EXT_MESSAGE_SIZE;
        fseek(gPointerFile, lFPos, SEEK_SET);
        if (ftell(gPointerFile) NEQ lFPos) {
            Error("Host message file is corrupt");
            MemFree(lStr);
            return False;
        }
        fflush(gPointerFile);

        atexit(AUXHOSTMessageCleanup);
    }

    if (gMessagesFile EQ 0) {
        gMessagesFile = OpenOutputFile(TMP_MSG_FILE, GAME_DIR_ONLY | APPEND_MODE);
        fseek(gMessagesFile, 0, SEEK_END);
        gMsgFilePointer = ftell(gMessagesFile);
    }

    lStruct.mRace = pRace;
    lStruct.mFilePos = gMsgFilePointer+1;
    lStruct.mLength = lLength;
#ifdef __MSDOS__
        if (1 NEQ fwrite(&lStruct, sizeof(lStruct), 1, gPointerFile)) goto bad_ext_write;
#else
        if (! DOSWriteStruct(ExtMessageStruct_Convert,
                             NumExtMessageStruct_Convert,
                             &lStruct,
                             gPointerFile)) goto bad_ext_write;
#endif

    if (1 NEQ fwrite(lStr, lLength, 1, gMessagesFile))
        goto bad_write;

    gNumExtMessages++;
    gMsgFilePointer += lLength;

    MemFree(lStr);
    return True;

bad_ext_write:
    Error("Can't write file '%s'", TMP_MSG_POINTER_FILE);
    MemFree(lStr);
    return False;
bad_write:
    Error("Can't write file '%s'", TMP_MSG_FILE);
    MemFree(lStr);
    return False;
}

/*************************************************************
  $HISTORY:$
**************************************************************/

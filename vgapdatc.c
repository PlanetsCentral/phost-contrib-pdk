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

static const char *GEN_FILE          = "gen.hst";
static HostGen_Struct    *gHostGenPtr=0;

static void FreeGen(void)
{
    MemFree(gHostGenPtr); gHostGenPtr = 0;
}

static void InitGen(void)
{
    if (gHostGenPtr EQ 0) {
        gHostGenPtr = (HostGen_Struct *)MemCalloc(1, sizeof(HostGen_Struct));

        RegisterCleanupFunction(FreeGen);
    }
}

Boolean PlayerIsActive(RaceType_Def pPlayer)
{
    InitGen();

    return gHostGenPtr->Players[pPlayer-1];
}

void PutPlayerIsActive(RaceType_Def pPlayer, Boolean pActive)
{
    InitGen();

    gHostGenPtr->Players[pPlayer-1] = pActive;
}

const char *PlayerPassword(RaceType_Def pPlayer)
{
    static char lPasswd[11];
    int lIndex;

    if (! PlayerIsActive(pPlayer)) return 0;

    for (lIndex = 0; lIndex < 10; lIndex++) {
        lPasswd[lIndex] = gHostGenPtr->Passwd1[pPlayer-1][lIndex] + 0x20
                        - gHostGenPtr->Passwd2[pPlayer-1][9-lIndex];
    }
    lPasswd[lIndex] = 0;
    return lPasswd;
}

IO_Def Read_HostGen_File(void)
{
    FILE *lGenFile;
    IO_Def  lError = IO_SUCCESS;

    InitGen();

    if ((lGenFile = OpenInputFile(GEN_FILE, GAME_DIR_ONLY | NO_MISSING_ERROR)) NEQ NULL) {
#ifdef __MSDOS__
		if (1 NEQ fread(gHostGenPtr, sizeof(HostGen_Struct), 1, lGenFile)) {
#else
		if (! DOSReadStruct(HostGenStruct_Convert,
							NumHostGenStruct_Convert,
							gHostGenPtr,
							lGenFile)) {
#endif
            Error("Can't read file '%s'", GEN_FILE);
            lError = IO_FAILURE;
        }

        fclose(lGenFile);
    } else lError = IO_FAILURE;
    return lError;
}

IO_Def Write_HostGen_File(void)
{
    FILE *lGenFile;
    IO_Def lError = IO_SUCCESS;

    if (gHostGenPtr EQ 0) {
        Error("Cannot write gen file without first reading it.\n");
        return IO_FAILURE;
    }

    if ((lGenFile = OpenOutputFile(GEN_FILE, GAME_DIR_ONLY)) NEQ NULL) {
#ifdef __MSDOS__
		if (1 NEQ fwrite(gHostGenPtr, sizeof(HostGen_Struct), 1, lGenFile)) {
#else
		if (! DOSWriteStruct(HostGenStruct_Convert,
							 NumHostGenStruct_Convert,
							 gHostGenPtr,
							 lGenFile)) {
#endif
            Error("Can't write to file '%s'", GEN_FILE);
            lError = IO_FAILURE;
        }
        fclose(lGenFile);
    } else lError = IO_FAILURE;
    return lError;
}

static const char NOPASSWD_STRING[] =
    "\x80\x81\x82\x73\x85\x85\x89\x81\x84\x76";

static void handleNewPassword(RaceType_Def pPlayer, const char *pPasswd)
{
    /* password is 10 characters excess +0x32 */
    /* and it is stored excess -0x20 EXCEPT for NULLs which are stored as 0
       in Passwd1 and as a random number in Passwd2 right after mastering */
    Uns16 lIndex;
    unsigned char lChar, lOffset;

    passert(gHostGenPtr NEQ 0);

    for (lIndex = 0; lIndex < 10; lIndex++) {
        lOffset = (unsigned char) RandomRange(40) + 32;
        lChar = pPasswd[lIndex] - 0x32;

        gHostGenPtr->Passwd2[pPlayer-1][9-lIndex] = lOffset;
        gHostGenPtr->Passwd1[pPlayer-1][lIndex] =
            pPasswd[lIndex] ? (lOffset + lChar - 0x20) : 0;
    }
}

void InitializeHostGen(void)
{
    Uns16 lRace;

    InitGen();

    memset(gHostGenPtr, 0, sizeof(HostGen_Struct));

    /* Set default passwords. */
    for (lRace = 1; lRace <= OLD_RACE_NR; lRace++) {
        handleNewPassword(lRace, NOPASSWD_STRING);
    }
}

void PutPlayerPassword(RaceType_Def pPlayer, const char *pPasswd)
{
    char lPwd[10];
    int i;

    InitGen();

    if (strlen(pPasswd) EQ 0) return;

    memset(lPwd, 0, sizeof(lPwd));
    for (i=0; i<strlen(pPasswd); i++) {
        lPwd[i] = pPasswd[i] + 0x32;
    }

    handleNewPassword(pPlayer, lPwd);
}

/*************************************************************
   $HISTORY:$
 **************************************************************/

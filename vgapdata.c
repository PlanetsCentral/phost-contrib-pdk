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

static const char *RACENAME_FILE     = "race.nm";
static RaceName_Def gRaceNames;
static Boolean gInitialized = False;

static void FreeRacenames(void)
{
    int i,j;

    for (i=0; i <= 2; i++) {
        for (j=1; j <= 11; j++) {
            MemFree(gRaceNames[i][j]); gRaceNames[i][j] = 0;
        }
    }
    gInitialized = False;
}

static void InitRacenames(void)
{
    if (gInitialized) return;

    memset(gRaceNames, 0, sizeof(gRaceNames));
    RegisterCleanupFunction(FreeRacenames);

    gInitialized = True;
}

 const char* RaceLongName(RaceType_Def pRace, char* pBuffer)
{
    InitRacenames();

    if (pBuffer NEQ NULL) strcpy(pBuffer, gRaceNames[0][pRace]);
	return(gRaceNames[0][pRace]);
}

 const char* RaceShortName(RaceType_Def pRace, char *pBuffer)
{
    InitRacenames();

    if (pBuffer NEQ NULL) strcpy(pBuffer, gRaceNames[1][pRace]);
	return(gRaceNames[1][pRace]);
}

 const char* RaceNameAdjective(RaceType_Def pRace, char *pBuffer)
{
    InitRacenames();

    if (pBuffer NEQ NULL) strcpy(pBuffer, gRaceNames[2][pRace]);
	return(gRaceNames[2][pRace]);
}

 void PutRaceLongName(RaceType_Def pRace, const char* pBuffer)
{
    InitRacenames();

    passert(pBuffer NEQ 0);
	strncpy(gRaceNames[0][pRace], pBuffer, LONGNAME_SIZE);
}

 void PutRaceShortName(RaceType_Def pRace, const char *pBuffer)
{
    InitRacenames();

    passert(pBuffer NEQ 0);
	strncpy(gRaceNames[1][pRace], pBuffer, SHORTNAME_SIZE);
}

 void PutRaceNameAdjective(RaceType_Def pRace, const char *pBuffer)
{
    InitRacenames();

    passert(pBuffer NEQ 0);
	strncpy(gRaceNames[2][pRace], pBuffer, ADJECTIVE_SIZE);
}

IO_Def Read_Racenames_File(void)
{
    FILE     *lNameFile;
    Int16    lSize,
             lPosition=0,
             i,j,
             lSizes[3]={LONGNAME_SIZE,SHORTNAME_SIZE,ADJECTIVE_SIZE};
    char     lBuffer[RACENAME_SIZE+1];
    IO_Def   lError = IO_SUCCESS;

    InitRacenames();

    if ((lNameFile = OpenInputFile(RACENAME_FILE, GAME_OR_ROOT_DIR | NO_MISSING_ERROR)) NEQ NULL) {
        if (1 NEQ fread(lBuffer, RACENAME_SIZE, 1, lNameFile)) {
            Error("Can't read file '%s'", RACENAME_FILE);
            lError=IO_FAILURE;
        } else {
            for (i=0; i<=2; i++) {
                for (j=1; j<=11; j++) {
                    lSize = lSizes[i];
                    while (lSize AND (lBuffer[lPosition+lSize-1] EQ ' '))
                        lSize--;                     /* cut trailing blanks */

                    gRaceNames[i][j] = (char *) MemCalloc(lSizes[i]+1, 1);
                    if (lSize > 0) {
                        memcpy(gRaceNames[i][j], lBuffer+lPosition, lSize);
                        gRaceNames[i][j][lSize] = 0;
                    } else {
                        strcpy(gRaceNames[i][j], "NoName");
                    }
                    lPosition += lSizes[i];
                }
            }
        }
        fclose(lNameFile);
    } else lError = IO_FAILURE;
    return(lError);
}

IO_Def Write_Racenames_File(void)
{
    FILE *lFile;
    char lBuffer[RACENAME_SIZE+1];
    char *lPtr, lName[LONGNAME_SIZE+1];
    Uns16 lRace;
    Uns16 lMode;

    if (gRaceNames[0][1] EQ 0) {
        Error("Cannot write race names file without first reading it.\n");
        return IO_FAILURE;
    }

    memset(lBuffer, ' ', RACENAME_SIZE);
    lPtr = lBuffer;

    for (lMode = 0; lMode < 3; lMode++) {
        for (lRace = 1; lRace <= OLD_RACE_NR; lRace++) {
            switch (lMode) {
             case 0:    /* LONG name */
                (void)RaceLongName(lRace, lName);
                strncpy(lPtr, lName, strlen(lName));
                lPtr += LONGNAME_SIZE;
                break;

             case 1:    /* SHORT name */
                (void)RaceShortName(lRace, lName);
                strncpy(lPtr, lName, strlen(lName));
                lPtr += SHORTNAME_SIZE;
                break;

             case 2:    /* ADJECTIVE name */
                (void)RaceNameAdjective(lRace, lName);
                strncpy(lPtr, lName, strlen(lName));
                lPtr += ADJECTIVE_SIZE;
                break;
            }
        }
    }

    lFile = OpenOutputFile(RACENAME_FILE, GAME_DIR_ONLY);
    if (1 NEQ fwrite(lBuffer, RACENAME_SIZE, 1, lFile)) {
        Error("Can't write to file '%s'", RACENAME_FILE);
        return IO_FAILURE;
    }
    fclose(lFile);
    return IO_SUCCESS;
}

/*************************************************************
  $HISTORY:$
**************************************************************/

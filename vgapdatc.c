
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

#include <time.h>
#include "phostpdk.h"
#include "private.h"

static const char *GEN_FILE = "gen.hst";
static HostGen_Struct *gHostGenPtr = 0;

static Boolean KillConfig[NumKILL] = { True, True, True,
                                       True, True, True };


static void
FreeGen(void)
{
  MemFree(gHostGenPtr);
  gHostGenPtr = 0;
}

static void
InitGen(void)
{
  if (gHostGenPtr EQ 0) {
    gHostGenPtr = (HostGen_Struct *) MemCalloc(1, sizeof(HostGen_Struct));

    RegisterCleanupFunction(FreeGen);
  }
}

Boolean
PlayerIsActive(RaceType_Def pPlayer)
{
  InitGen();

  return gHostGenPtr->Players[pPlayer - 1];
}

void
PutPlayerIsActive(RaceType_Def pPlayer, Boolean pActive)
{
  InitGen();

  gHostGenPtr->Players[pPlayer - 1] = pActive;
}

const char *
PlayerPassword(RaceType_Def pPlayer)
{
  static char lPasswd[11];
  int lIndex;

  if (!PlayerIsActive(pPlayer))
    return 0;

  for (lIndex = 0; lIndex < 10; lIndex++) {
    lPasswd[lIndex] =
          gHostGenPtr->Passwd1[pPlayer - 1][lIndex] + 0x20 -
          gHostGenPtr->Passwd2[pPlayer - 1][9 - lIndex];
  }
  lPasswd[lIndex] = 0;
  return lPasswd;
}

IO_Def
Read_HostGen_File(void)
{
  FILE *lGenFile;
  IO_Def lError = IO_SUCCESS;

  InitGen();

  if ((lGenFile =
              OpenInputFile(GEN_FILE,
                GAME_DIR_ONLY | NO_MISSING_ERROR)) NEQ NULL) {
#ifdef __MSDOS__
    if (1 NEQ fread(gHostGenPtr, sizeof(HostGen_Struct), 1, lGenFile)) {
#else
    if (!DOSReadStruct(HostGenStruct_Convert, NumHostGenStruct_Convert,
                gHostGenPtr, lGenFile)) {
#endif
      Error("Can't read file '%s'", GEN_FILE);
      lError = IO_FAILURE;
    }

    fclose(lGenFile);
  }
  else
    lError = IO_FAILURE;
  return lError;
}

IO_Def
Write_HostGen_File(void)
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
    if (!DOSWriteStruct(HostGenStruct_Convert, NumHostGenStruct_Convert,
                gHostGenPtr, lGenFile)) {
#endif
      Error("Can't write to file '%s'", GEN_FILE);
      lError = IO_FAILURE;
    }
    fclose(lGenFile);
  }
  else
    lError = IO_FAILURE;
  return lError;
}

static const char NOPASSWD_STRING[] =

      "\x80\x81\x82\x73\x85\x85\x89\x81\x84\x76";

static void
handleNewPassword(RaceType_Def pPlayer, const char *pPasswd)
{
  /* password is 10 characters excess +0x32 */
  /* and it is stored excess -0x20 EXCEPT for NULLs which are stored as 0 in
     Passwd1 and as a random number in Passwd2 right after mastering */
  Uns16 lIndex;
  unsigned char lChar,
    lOffset;

  passert(gHostGenPtr NEQ 0);

  for (lIndex = 0; lIndex < 10; lIndex++) {
    lOffset = (unsigned char) RandomRange(40) + 32;
    lChar = pPasswd[lIndex] - 0x32;

    gHostGenPtr->Passwd2[pPlayer - 1][9 - lIndex] = lOffset;
    gHostGenPtr->Passwd1[pPlayer - 1][lIndex] =
          pPasswd[lIndex] ? (lOffset + lChar - 0x20) : 0;
  }
}

void
InitializeHostGen(void)
{
  Uns16 lRace;

  InitGen();

  memset(gHostGenPtr, 0, sizeof(HostGen_Struct));

  /* Set default passwords. */
  for (lRace = 1; lRace <= OLD_RACE_NR; lRace++) {
    handleNewPassword(lRace, NOPASSWD_STRING);
  }
}

void
PutPlayerPassword(RaceType_Def pPlayer, const char *pPasswd)
{
  char lPwd[10];
  int i;

  InitGen();

  if (strlen(pPasswd) EQ 0)
    return;

  memset(lPwd, 0, sizeof(lPwd));
  for (i = 0; i < strlen(pPasswd); i++) {
    lPwd[i] = pPasswd[i] + 0x32;
  }

  handleNewPassword(pPlayer, lPwd);
}

/*
  Kill functions, based on killrace.c
 */
 
Boolean
KillPlayerConfigure(PlayerKillType_Def lKillConf)
{
   return KillConfig[lKillConf];
}

void
PutKillPlayerConfigure(PlayerKillType_Def lKillConf, Boolean lValue)
{
   KillConfig[lKillConf]=lValue;
}

void
KillPlayer(RaceType_Def Race)
{
    Uns16 lIx;

    if (KillConfig[KILL_Ships])      KillPlayerShips(Race);
    if (KillConfig[KILL_Bases])      KillPlayerBases(Race);
    if (KillConfig[KILL_Planets])    KillPlayerPlanets(Race);
    if (KillConfig[KILL_Minefields]) KillPlayerMinefields(Race);

    for (lIx = 1; lIx <= RACE_NR; lIx++)
        AllyDropRequest(Race, lIx);

    PutPlayerIsActive(Race, False);

}

void
KillPlayerShips(RaceType_Def Race)
{
    Uns16 lIx;
    Boolean lShipRemoved[SHIP_NR+1];

    /* Delete Race ships */

    for (lIx = 1; lIx <= SHIP_NR; lIx++)
        {
        lShipRemoved[lIx] = False;
        
        if (IsShipExist(lIx) AND ShipOwner(lIx) EQ Race) {
            DeleteShip(lIx);
            ResetShipRemoteControl(lIx);
            lShipRemoved[lIx] = True;
        }
        }

    /* Now go through and clear tow/intercept missions on ships which
       were towing/intercepting a ship that we just deleted. */

    for (lIx = 1; lIx <= SHIP_NR; lIx++) {
        if (! IsShipExist(lIx)) continue;

        if (    (ShipMission(lIx) EQ Tow)
            AND lShipRemoved[ShipTowTarget(lIx)]
           )
            PutShipMission(lIx, NoMission);

        if (    (ShipMission(lIx) EQ Intercept)
            AND lShipRemoved[ShipInterceptTarget(lIx)]
           )
            PutShipMission(lIx, NoMission);

    }
}

void
KillPlayerPlanets(RaceType_Def Race)
{
    Uns16 lIx;
    char FC[4];
    time_t now;

    for (lIx = 1; lIx <= PLANET_NR; lIx++) {
        if (IsPlanetExist(lIx) AND PlanetOwner(lIx) EQ Race) {

            if (IsBaseExist(lIx)) {
                BuildQueueInvalidate(lIx);
                DeleteBase(lIx);
            }
        
            PutPlanetCargo(lIx, COLONISTS, 0);
            PutPlanetOwner(lIx, NoOwner);
            PutPlanetColHappy(lIx,100);

            time(&now);
            SetRandomSeed(now % 32000 );
            sprintf(FC,"%03d",RandomRange(1000));
            PutPlanetFC  (lIx,FC);

            if (KillConfig[KILL_Cash_Supplies])
            {
              PutPlanetCargo(lIx, SUPPLIES, 0);
              PutPlanetCargo(lIx, CREDITS, 0);
            }

            if (KillConfig[KILL_Structures])
            {
              PutPlanetMines    (lIx,0);
              PutPlanetFactories(lIx,0);
              PutPlanetDefense  (lIx,0);
            }
        }
    }
}

void
KillPlayerBases(RaceType_Def Race)
{
    Uns16 lIx;

    for (lIx = 1; lIx <= BASE_NR; lIx++) {
        if (IsPlanetExist(lIx) AND PlanetOwner(lIx) EQ Race) {
            if (IsBaseExist(lIx)) {
                BuildQueueInvalidate(lIx);
                DeleteBase(lIx);
            }
        }
    }
}

void
KillPlayerMinefields(RaceType_Def Race)
{
    Uns16 lIx;

    for (lIx = 1; lIx <= MINE_NR; lIx++) {
        if (IsMinefieldExist(lIx) AND MinefieldOwner(lIx) EQ Race )
           PutMinefieldUnits(lIx,0);
    }
}

/*************************************************************
   $HISTORY:$
   22.12.2001 Added race kill functions - Piotr Winiarczyk
              Kill functions are heavy based on killrace.c
   
 **************************************************************/

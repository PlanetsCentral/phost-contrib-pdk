
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

static const char *MINE_FILE = "mines.hst";
static const char *MINE_EXT_FILE = "extmines.hst";
static MineField_Struct *gMinesPtr = 0;

static void
FreeMines(void)
{
  MemFree(gMinesPtr);
  gMinesPtr = 0;
}

static void
InitMines(void)
{
  if (gMinesPtr EQ 0) {
    gMinesPtr =
          (MineField_Struct *) MemCalloc(MINE_NR + 1,
          sizeof(MineField_Struct));

    RegisterCleanupFunction(FreeMines);
  }
}

/* Minefields, unlike planets, ships, and bases, are stored in memory
   as an actual array of structs, rather than an array of pointers. This
   is because they only take up 6K of data at worst */
MineField_Struct *
GetMinefield(Uns16 pID)
{
  InitMines();

  passert((pID > 0) AND(pID <= MINE_NR));
  return (gMinesPtr + pID);
}

void
PutMinefield(Uns16 pID, MineField_Struct * pMine)
{
  InitMines();

  passert((pID > 0) AND(pID <= MINE_NR) AND(pMine NEQ NULL));
  memmove(gMinesPtr + pID, pMine, sizeof(MineField_Struct));
}

/***************************************************************************
  minefield member-like functions
 ***************************************************************************/

Uns16
GetNumMinefields(void)
{
  if (gConfigInfo EQ 0)
    return 500;
  else
    return gConfigInfo->CPNumMinefields;
}

#ifndef MICROSQUISH
double
MinefieldRadius(Uns16 pID)
{
  MineField_Struct *lMine = GetMinefield(pID);

  if (lMine->Units NEQ 0) {
    return sqrt((double) (lMine->Units));
  }
  else {
    return 0;
  }
}
#endif /* MICROSQUISH */

Uns16
MinefieldPositionX(Uns16 pID)
{
  return GetMinefield(pID)->Position[X_Part];
}

Uns16
MinefieldPositionY(Uns16 pID)
{
  return GetMinefield(pID)->Position[Y_Part];
}

RaceType_Def
MinefieldOwner(Uns16 pID)
{
  return GetMinefield(pID)->Race;
}

Uns32
MinefieldUnits(Uns16 pID)
{
  return GetMinefield(pID)->Units;
}

Boolean
IsMinefieldWeb(Uns16 pID)
{
  return (Boolean) (GetMinefield(pID)->Web & 0x1);
}

void
PutMinefieldUnits(Uns16 pID, Uns32 pUnits)
{
  GetMinefield(pID)->Units = pUnits;
}

void
PutMinefieldOwner(Uns16 pID, RaceType_Def pRace)
{
  GetMinefield(pID)->Race = pRace;
}

Boolean
IsMinefieldExist(Uns16 pID)
{
  return (Boolean) (GetMinefield(pID)->Units > 0);
}

/* This is used to create a new minefield in an empty slot. The return
   value is the new minefield ID or 0 if no slots are free */
Uns16
CreateMinefield(Uns16 pXloc, Uns16 pYloc, RaceType_Def pRace, Uns32 pUnits,
      Boolean pIsWeb)
{
  Uns16 pID;

  passert((pXloc >= MIN_COORDINATE) AND(pXloc <= MAX_COORDINATE)
        AND(pYloc >= MIN_COORDINATE) AND(pYloc <= MAX_COORDINATE)
        AND(pUnits > 0));

  for (pID = 1; pID <= GetNumMinefields(); pID++) {
    MineField_Struct *lMine = GetMinefield(pID);

    if (lMine->Units EQ 0) {
      lMine->Position[X_Part] = pXloc;
      lMine->Position[Y_Part] = pYloc;
      lMine->Race = pRace;
      lMine->Units = pUnits;
      lMine->Web = pIsWeb;
      return pID;
    }
  }
  return 0;
}

/* @func
   to ensure compatibility with HOST and earlier PHOST versions we split the
   minefield info into 2 parts: mines.hst contains the usual 500 minefields
   and extmines.hst contains the rest

   @rdesc success of operation */
IO_Def
Read_Mines_File(void)
{
  FILE *lMineFile;
  IO_Def lError = IO_SUCCESS;

  InitMines();
  memset(gMinesPtr, 0, (MINE_NR + 1) * sizeof(MineField_Struct));

  passert(MINE_NR >= 500);
  if ((lMineFile = OpenInputFile(MINE_FILE, GAME_DIR_ONLY)) NEQ NULL) {
    int lMine;

    /* read first 500 mines */
#ifdef __MSDOS__
    if (500 NEQ fread(gMinesPtr + 1, sizeof(MineField_Struct), 500,
                      lMineFile)) {
      Error("Can't read file '%s'", MINE_FILE);
      lError = IO_FAILURE;
    }
#else
    for (lMine = 1; lMine <= 500; lMine++) {
      if (!DOSReadStruct(MineStruct_Convert, NumMineStruct_Convert,
                  gMinesPtr + lMine, lMineFile)) {
        Error("Can't read file '%s'", MINE_FILE);
        lError = IO_FAILURE;
        break;
      }
    }
#endif

    fclose(lMineFile);
  }
  else
    lError = IO_FAILURE;

  /* same procedure again with extra mines */
  if ((lError EQ IO_FAILURE) OR(MINE_NR EQ 500)) {
    return (lError);
  }

  /* ! Note: if extmines.hst can't be found then we have no error. It
     probably means that the host data was generated by an earlier version of 
     PHOST or by HOST. */
  if ((lMineFile =
              OpenInputFile(MINE_EXT_FILE,
               GAME_DIR_ONLY + NO_MISSING_ERROR)) NEQ NULL) {
    int lMine;

    /* read extra mines */
#ifdef __MSDOS__
    if ((MINE_NR - 500) NEQ fread(gMinesPtr + 501, sizeof(MineField_Struct),
                (MINE_NR - 500), lMineFile)) {
      Error("Can't read file '%s'", MINE_EXT_FILE);
      lError = IO_FAILURE;
    }
#else
    for (lMine = 501; lMine <= (MINE_NR - 500); lMine++) {
      if (!DOSReadStruct(MineStruct_Convert, NumMineStruct_Convert,
                         gMinesPtr + lMine, lMineFile)) {
        Error("Can't read file '%s'", MINE_EXT_FILE);
        lError = IO_FAILURE;
        break;
      }
    }
#endif

    fclose(lMineFile);
  }

  return (lError);
}

/* we split the mine data into 2 files, mines.hst containing the first 500 and
   extmines.hst the rest */
IO_Def
Write_Mines_File(void)
{
  FILE *lMineFile;
  IO_Def lError = IO_SUCCESS;

  if ((lMineFile = OpenOutputFile(MINE_FILE, GAME_DIR_ONLY)) NEQ NULL) {
#ifdef __MSDOS__
    if (500 NEQ fwrite(gMinesPtr + 1, sizeof(MineField_Struct), 500,
                       lMineFile)) {
      Error("Can't write file '%s'", MINE_FILE);
      lError = IO_FAILURE;
    }
#else
    int lMine;

    for (lMine = 1; lMine <= 500; lMine++) {
      if (!DOSWriteStruct(MineStruct_Convert, NumMineStruct_Convert,
                  gMinesPtr + lMine, lMineFile)) {
        Error("Can't write file '%s'", MINE_FILE);
        lError = IO_FAILURE;
        break;
      }
    }
#endif
    fclose(lMineFile);
  }
  else
    lError = IO_FAILURE;

  /* write extra minefields */
  if ((lMineFile = OpenOutputFile(MINE_EXT_FILE, GAME_DIR_ONLY)) NEQ NULL) {
#ifdef __MSDOS__
    if ((MINE_NR-500) NEQ fwrite(gMinesPtr + 501, sizeof(MineField_Struct),
                                 MINE_NR - 500, lMineFile)) {
      Error("Can't write file '%s'", MINE_EXT_FILE);
      lError = IO_FAILURE;
    }
#else
    int lMine;

    for (lMine = 501; lMine <= MINE_NR; lMine++) {
      if (!DOSWriteStruct(MineStruct_Convert, NumMineStruct_Convert,
                  gMinesPtr + lMine, lMineFile)) {
        Error("Can't write file '%s'", MINE_EXT_FILE);
        lError = IO_FAILURE;
        break;
      }
    }
#endif
    fclose(lMineFile);
  }
  else
    lError = IO_FAILURE;

  return (lError);
}

void
InitializeMines(void)
{
  InitMines();

  memset(gMinesPtr, 0, sizeof(MineField_Struct) * (MINE_NR + 1));
}

void
WrapMineLocation(Uns16 pID)
{
  MineField_Struct *lMine = GetMinefield(pID);

  lMine->Position[X_Part] = WrapMapX(lMine->Position[X_Part]);
  lMine->Position[Y_Part] = WrapMapY(lMine->Position[Y_Part]);
}

/*************************************************************
  $HISTORY:$
**************************************************************/


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

static const char *PLANETNAME_FILE = "planet.nm";
static Planetname_Struct *gPlanetnamePtr = 0;

static void
FreePlanetname(void)
{
  MemFree(gPlanetnamePtr);
  gPlanetnamePtr = 0;
}

static void
InitPlanetname(void)
{
  if (gPlanetnamePtr EQ 0) {
    gPlanetnamePtr =
          (Planetname_Struct *) MemCalloc(PLANET_NR + 1,
          sizeof(Planetname_Struct));

    RegisterCleanupFunction(FreePlanetname);
  }
}

const char *
PlanetName(Uns16 pID, char *pBuffer)
{
  static char lName[PLANETNAME_SIZE + 1];
  char *lPtr = pBuffer;

  InitPlanetname();

  passert(IsPlanetExist(pID));

  if (lPtr EQ 0)
    lPtr = lName;
  strncpy(lPtr, gPlanetnamePtr[pID].Name, PLANETNAME_SIZE);
  lPtr[PLANETNAME_SIZE] = 0;
  return lPtr;
}

void
PutPlanetName(Uns16 pID, const char *pName)
{
  static char lName[PLANETNAME_SIZE + 1];
  int i;

  InitPlanetname();

  passert(IsPlanetExist(pID));
  passert(pName NEQ NULL);
  
  memcpy(lName, pName, PLANETNAME_SIZE);
  lName[PLANETNAME_SIZE] = 0;
  for (i = strlen(lName); i < PLANETNAME_SIZE; i++)
    lName[i] = ' ';
  
  memcpy(gPlanetnamePtr[pID].Name, lName, PLANETNAME_SIZE);    
}

IO_Def
Read_Planetname_File(void)
{
  FILE *lPlanetnameFile;
  IO_Def lError = IO_SUCCESS;;

  InitPlanetname();

  if ((lPlanetnameFile =
              OpenInputFile(PLANETNAME_FILE,
         GAME_OR_ROOT_DIR | NO_MISSING_ERROR)) NEQ NULL) {
    if (PLANET_NR NEQ fread(gPlanetnamePtr + 1, sizeof(Planetname_Struct),
                PLANET_NR, lPlanetnameFile)) {
      Error("Can't read file '%s'", PLANETNAME_FILE);
      lError = IO_FAILURE;
    }
    fclose(lPlanetnameFile);
  }
  else
    lError = IO_FAILURE;

  return (lError);
}

IO_Def
Write_Planetname_File(void)
{
  FILE *lPlanetnameFile;
  IO_Def lError = IO_SUCCESS;;

  InitPlanetname();

  if ((lPlanetnameFile =
              OpenOutputFile(PLANETNAME_FILE,
         ROOT_DIR_ONLY | NO_MISSING_ERROR)) NEQ NULL) {
    if (PLANET_NR NEQ fwrite(gPlanetnamePtr + 1, sizeof(Planetname_Struct),
                PLANET_NR, lPlanetnameFile)) {
      Error("Can't write file '%s'", PLANETNAME_FILE);
      lError = IO_FAILURE;
    }
    fclose(lPlanetnameFile);
  }
  else
    lError = IO_FAILURE;

  return (lError);
}

/*************************************************************
  $HISTORY:$
**************************************************************/

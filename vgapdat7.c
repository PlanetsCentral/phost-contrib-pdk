
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

static const char *TORPSPEC_FILE = "torpspec.dat";
static Torpspec_Struct *gTorpspecPtr = 0;

static void
FreeTorp(void)
{
  MemFree(gTorpspecPtr);
  gTorpspecPtr = 0;
}

static void
InitTorp(void)
{
  if (gTorpspecPtr EQ 0) {
    gTorpspecPtr =
          (Torpspec_Struct *) MemCalloc(TORP_NR + 1, sizeof(Torpspec_Struct));

    RegisterCleanupFunction(FreeTorp);
  }
}

char *
TorpName(Uns16 pTorpNr, char *pBuffer)
{
  static char lName[21];
  char *lPtr = pBuffer;

  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));

  if (lPtr EQ NULL)
    lPtr = lName;
  memcpy(lPtr, gTorpspecPtr[pTorpNr].Name, 20);
  lPtr[20] = 0;
  return lPtr;
}

Uns16
TorpTorpCost(Uns16 pTorpNr)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  return (gTorpspecPtr[pTorpNr].TorpCost);
}

Uns16
TorpTubeCost(Uns16 pTorpNr)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  return (gTorpspecPtr[pTorpNr].TubeCost);
}

Uns16
TorpTritCost(Uns16 pTorpNr)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  return (gTorpspecPtr[pTorpNr].TubeTritCost);
}

Uns16
TorpDurCost(Uns16 pTorpNr)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  return (gTorpspecPtr[pTorpNr].TubeDurCost);
}

Uns16
TorpMolyCost(Uns16 pTorpNr)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  return (gTorpspecPtr[pTorpNr].TubeMolyCost);
}

Uns16
TorpTubeMass(Uns16 pTorpNr)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  return (gTorpspecPtr[pTorpNr].TubeMass);
}

Uns16
TorpTechLevel(Uns16 pTorpNr)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  return (gTorpspecPtr[pTorpNr].TechLevel);
}

Uns16
TorpKillPower(Uns16 pTorpNr)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  return (gTorpspecPtr[pTorpNr].KillPower);
}

Uns16
TorpDestructivePower(Uns16 pTorpNr)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  return (gTorpspecPtr[pTorpNr].DestructivePower);
}

void
PutTorpName(Uns16 pTorpNr, char *pName)
{  
  static char lName[21];
  int i;

  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  passert(pName NEQ NULL);

  memcpy(lName, pName, 20);
  lName[20] = 0;
  for (i = strlen(lName); i < 20; i++)
    lName[i] = ' ';
  
  memcpy(gTorpspecPtr[pTorpNr].Name, lName, 20);   
  
}

void
PutTorpTorpCost(Uns16 pTorpNr, Uns16 pTorpCost)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  gTorpspecPtr[pTorpNr].TorpCost = pTorpCost;
}

void
PutTorpTubeCost(Uns16 pTorpNr, Uns16 pTubeCost)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  gTorpspecPtr[pTorpNr].TubeCost = pTubeCost;
}

void
PutTorpTritCost(Uns16 pTorpNr, Uns16 pTubeTritCost)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  gTorpspecPtr[pTorpNr].TubeTritCost = pTubeTritCost;
}

void
PutTorpDurCost(Uns16 pTorpNr, Uns16 pTubeDurCost)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  gTorpspecPtr[pTorpNr].TubeDurCost = pTubeDurCost;
}

void
PutTorpMolyCost(Uns16 pTorpNr, Uns16 pTubeMolyCost)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  gTorpspecPtr[pTorpNr].TubeMolyCost = pTubeMolyCost;
}

void
PutTorpTubeMass(Uns16 pTorpNr, Uns16 pTubeMass)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  gTorpspecPtr[pTorpNr].TubeMass = pTubeMass;
}

void
PutTorpTechLevel(Uns16 pTorpNr, Uns16 pTechLevel)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  gTorpspecPtr[pTorpNr].TechLevel = pTechLevel;
}

void
PutTorpKillPower(Uns16 pTorpNr, Uns16 pKillPower)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  gTorpspecPtr[pTorpNr].KillPower = pKillPower;
}

void
PutTorpDestructivePower(Uns16 pTorpNr, Uns16 pDestructivePower)
{
  InitTorp();

  passert((pTorpNr >= 1) AND(pTorpNr <= TORP_NR));
  gTorpspecPtr[pTorpNr].DestructivePower = pDestructivePower;
}

IO_Def
Read_Torpspec_File(void)
{
  FILE *lTorpFile;
  IO_Def lError = IO_SUCCESS;

  InitTorp();

  if ((lTorpFile =
              OpenInputFile(TORPSPEC_FILE,
               GAME_OR_ROOT_DIR | NO_MISSING_ERROR)) NEQ NULL) {
#ifdef __MSDOS__
    if (TORP_NR NEQ fread(gTorpspecPtr + 1, sizeof(Torpspec_Struct), TORP_NR,
                lTorpFile)) {
      Error("Unable to read from '%s'", TORPSPEC_FILE);
      lError = IO_FAILURE;
    }
#else
    Uns16 lTorp;

    for (lTorp = 1; lTorp <= TORP_NR; lTorp++) {
      if (!DOSReadStruct(TorpspecStruct_Convert, NumTorpspecStruct_Convert,
                  gTorpspecPtr + lTorp, lTorpFile)) {
        Error("Can't read file '%s'", TORPSPEC_FILE);
        lError = IO_FAILURE;
        break;
      }
    }
#endif

    fclose(lTorpFile);
  }
  else
    lError = IO_FAILURE;
  return (lError);
}

IO_Def
Write_Torpspec_File(void)
{
  FILE *lTorpFile;
  IO_Def lError = IO_SUCCESS;

  InitTorp();

  if ((lTorpFile =
              OpenOutputFile(TORPSPEC_FILE,
               ROOT_DIR_ONLY | NO_MISSING_ERROR)) NEQ NULL) {
#ifdef __MSDOS__
    if (TORP_NR NEQ fwrite(gTorpspecPtr + 1, sizeof(Torpspec_Struct), TORP_NR,
                lTorpFile)) {
      Error("Can't write file '%s'", TORPSPEC_FILE);
      lError = IO_FAILURE;
    }
#else
    Uns16 lTorp;

    for (lTorp = 1; lTorp <= TORP_NR; lTorp++) {
      if (!DOSWriteStruct(TorpspecStruct_Convert, NumTorpspecStruct_Convert,
                  gTorpspecPtr + lTorp, lTorpFile)) {
        Error("Can't write file '%s'", TORPSPEC_FILE);
        lError = IO_FAILURE;
        break;
      }
    }
#endif

    fclose(lTorpFile);
  }
  else
    lError = IO_FAILURE;
  return (lError);
}

/*************************************************************
  $HISTORY:$
**************************************************************/

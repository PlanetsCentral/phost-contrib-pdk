
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
 *                  *
 *  Module:   PHOST Main Module          *
 *  Project:   Portable Host           *
 *  File Name:   main.c            *
 *  Programmer:  Andrew Sterian          *
 *  Create Date: October 18, 1994          *
 *  Description: This module is the mainline of the application. *
 *      The command line is parsed and the appropriate  *
 *      global switches are set. Host data files are   *
 *      read in and turn processing occurs. The mission *
 *      order is determined in this module.     *
 *  Functions:               *
 *  Change Record:              *
 *                  *
 *                  *
 ******************************************************************/

#include "phostpdk.h"
#include "private.h"

/* These variables store the names of the root directory and the game
   directory. Unlike the VGAP HOST, the game directory need not be a
   subdirectory of the root directory and the root directory need not
   be the current working directory. */
const char *gRootDirectory = ".";
const char *gGameDirectory = ".";
Boolean gNewlyMastered = False;

/* This variable controls the display of warnings for non-PHOST games. If
   a program doesn't care whether or not the game is PHOST, this should be
   set to False. */
Boolean gNonPHOSTWarnings = False;

/* This variable is the file handle of the host log file. It is always open. */
FILE *gLogFile = 0;

/* We put this here because lots of modules access this and we don't want
   to link in pconfig.c unnecessarily */

Hconfig_Struct *gConfigInfo = 0;

/* This holds the list of functions we are to call for cleanup */
#define MAX_CLEANUP_FUNCS 32
static CleanupFunction_T *gCleanupFuncs = 0;
static int NumCleanupFuncs;

void
RegisterCleanupFunction(CleanupFunction_T pFunc)
{
  passert(NumCleanupFuncs < MAX_CLEANUP_FUNCS);
  passert(gCleanupFuncs);
  gCleanupFuncs[NumCleanupFuncs++] = pFunc;
}

void
InitPHOSTLib(void)
{
  static Boolean firstTime = True;

  if (gCleanupFuncs) {
#ifdef PDK_DEBUG
    Warning("InitPHOSTLib called without FreePHOSTLib");
#endif
    FreePHOSTLib();
  }
  gCleanupFuncs = MemCalloc(MAX_CLEANUP_FUNCS, sizeof(CleanupFunction_T));
  NumCleanupFuncs = 0;

  if (firstTime) {
    firstTime = False;
    fprintf(stdout, "PHOST Development Kit Version %u.%u\n",
            PDK_VERSION_MAJOR, PDK_VERSION_MINOR);
    fprintf(stdout, "Copyright (C) 1995-2003 Portable Host Project\n\n");
  }

  /* We DO need to create a gConfigInfo structure else InitWraparounds() will 
     fail. */
  passert(gConfigInfo EQ 0);
  gConfigInfo = (Hconfig_Struct *) MemCalloc(sizeof(Hconfig_Struct), 1);
  InitConfig();
}

void
FreePHOSTLib(void)
{
  if (!gCleanupFuncs) {
#ifdef PDK_DEBUG
    Warning("FreePHOSTLib called without InitPHOSTLib");
#endif
    return;
  }
  while (NumCleanupFuncs--)
    (*(gCleanupFuncs[NumCleanupFuncs])) ();

  MemFree(gCleanupFuncs);
  gCleanupFuncs = 0;
  NumCleanupFuncs = 0;

  passert(gConfigInfo);
  MemFree(gConfigInfo);
  gConfigInfo = 0;
}

Uns16
EffRace(Uns16 pPlayer)
{
  return (pPlayer >= 1 AND pPlayer <= OLD_RACE_NR)
        ? gConfigInfo->PlayerRace[pPlayer]
        : 0;
}

/*************************************************************
  $HISTORY:$
**************************************************************/

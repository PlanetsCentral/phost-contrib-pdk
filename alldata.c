
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

static Int16 gPlanetControl;
static Int16 gShipControl;
static Int16 gBaseControl1,
  gBaseControl2;

/*--------------------------------------------------------------------------
Name      : ReadGlobalData()
Purpose   : initializes the host data structures by reading data files
            These are the "fixed" data files that are constant for all
            games, such as the planet names and positions, ship hull specs,
            etc.
--------------------------------------------------------------------------*/

IO_Def
ReadGlobalData(void)
{
  if (Read_Beamspec_File()NEQ IO_SUCCESS)
    goto read_fail;
  if (Read_Torpspec_File()NEQ IO_SUCCESS)
    goto read_fail;
  if (Read_Racenames_File()NEQ IO_SUCCESS)
    goto read_fail;
  if (Read_Hullspec_File()NEQ IO_SUCCESS)
    goto read_fail;
  if (Read_Engspec_File()NEQ IO_SUCCESS)
    goto read_fail;
  if (Read_Truehull_File()NEQ IO_SUCCESS)
    goto read_fail;
  if (Read_Xyplan_File()NEQ IO_SUCCESS)
    goto read_fail;
  if (Read_Planetname_File()NEQ IO_SUCCESS)
    goto read_fail;

  return IO_SUCCESS;

read_fail:
  Error("Unable to read global game data");
  return IO_FAILURE;
}

/*--------------------------------------------------------------------------
Name      : ReadHostData()
Purpose   : initializes the host data structures by reading data files
            These data files change for each different hosted game.
--------------------------------------------------------------------------*/

IO_Def
ReadHostData(void)
{
  /* Must read config file first so we have Language[] initialized for
     StringRetrieve. In fact, we may need lots of things from our config
     structure. */
  if (Read_HConfig_File() NEQ IO_SUCCESS)
    if (Read_THost_HConfig_File() NEQ IO_SUCCESS)
      goto read_fail;

  /* Read the NEXTTURN.HST file first to see if this is a newly mastered game 
   */
  if (Read_Turntime_File()NEQ IO_SUCCESS)
    goto read_fail;
  if (Read_Planets_File(&gPlanetControl) NEQ IO_SUCCESS)
    goto read_fail;
  if (Read_Ships_File(&gShipControl) NEQ IO_SUCCESS)
    goto read_fail;
  if (Read_Bases_File(&gBaseControl1, &gBaseControl2) NEQ IO_SUCCESS)
    goto read_fail;
  if (Read_Mines_File()NEQ IO_SUCCESS)
    goto read_fail;
  if (Read_HostGen_File()NEQ IO_SUCCESS)
    goto read_fail;
#ifndef MICROSQUISH
  if (!ReadWormholeFile())
    goto read_fail;
#endif

  /* No auxdata file if this is a newly mastered game */
  if (!gNewlyMastered) {
    if (Read_AuxData_File()NEQ IO_SUCCESS)
      goto read_fail;
  } else {
    HandleMissingAuxdata();
  }

  /* read the CLOAKC.HST file after auxdata. This may get the correct cloak
     status even if HOST is used */
  ReadCLOAKCFile();
  ReadHullfunc();

  return IO_SUCCESS;

read_fail:
  Error("Unable to read game data files");
  return IO_FAILURE;
}

/*--------------------------------------------------------------------------
Name      : WriteHostData()
Purpose   : writes out updated host data to files
--------------------------------------------------------------------------*/

IO_Def
WriteHostData(void)
{
  if (Write_Planets_File(gPlanetControl) NEQ IO_SUCCESS)
    goto write_fail;
  if (Write_Ships_File(gShipControl) NEQ IO_SUCCESS)
    goto write_fail;
  if (Write_Bases_File(gBaseControl1, gBaseControl2) NEQ IO_SUCCESS)
    goto write_fail;
  if (Write_Mines_File()NEQ IO_SUCCESS)
    goto write_fail;
  if (Write_AuxData_File()NEQ IO_SUCCESS)
    goto write_fail;
  if (Write_HostGen_File()NEQ IO_SUCCESS)
    goto write_fail;
  if (WriteCLOAKCFile()NEQ IO_SUCCESS)
    goto write_fail;

#ifndef MICROSQUISH
  if (!WriteWormholeFile())
    goto write_fail;
#endif
  return IO_SUCCESS;

write_fail:
  Error("Unable to successfully write game data files");
  return IO_FAILURE;
}

/*******************************************************************************
$History: . $
*******************************************************************************/

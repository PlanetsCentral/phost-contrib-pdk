
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

#ifndef CBUILDER
#include <ctype.h>
#endif
#include "phostpdk.h"
#include "private.h"
#include "listmat.h"

int
ListMatch(const char *token, const char *list)
{
  const char *tptr;
  int match;

  match = 0;

  while (*list) {
    int required = 1;

    for (tptr = token; *token && *list && !isspace(*list); tptr++, list++) {
      if (required && islower(*list))
        required = 0;

      if (toupper(*tptr) NEQ toupper(*list))
        break;                  /* Not this one */
    }
    if (*tptr == 0) {
      if (*list && islower(*list))
        required = 0;
      if (!required || isspace(*list) || *list == 0)
        return match;
    }
    while (*list && !isspace(*list))
      list++;
    if (*list)
      list++;
    match++;
  }

  return -1;
}

/*************************************************************
  $HISTORY:$
**************************************************************/

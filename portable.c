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
 *                                                                *
 *   Module:      Portability Support                             *
 *   Project:     Portable Host                                   *
 *   File Name:   portable.c                                      *
 *   Programmer:  Andrew Sterian                                  *
 *   Create Date: October 18, 1994                                *
 *   Description: This module is concerned with making the rest   *
 *                of the modules platform-independent. Functions  *
 *                that don't exist on some systems are defined    *
 *                (or stubbed out) here, some constants are       *
 *                defined, etc.                                   *
 *   Functions:                                                   *
 *   Change Record:                                               *
 *                                                                *
 *                                                                *
 ******************************************************************/

#include <ctype.h>
#include "phostpdk.h"
#include "private.h"

#ifndef HAVE_MEMMOVE
#  ifndef HAVE_BCOPY
#    error "You need either 'memmove' or 'bcopy'"
#  endif
void *memmove(void *dst, const void *src, size_t size)
{
    bcopy(src, dst, size);
    return dst;
}
#endif /* ! HAVE_MEMMOVE */

#ifndef HAVE_STRERROR
char *strerror(int errnum)
{
    return "(no error information available)";
}
#endif /* ! HAVE_STRERROR */

#ifndef HAVE_STRICMP
int stricmp(const char *s1, const char *s2)
{
    while ((*s1 != 0) 
             && 
           (toupper(*s1) == toupper(*s2))) {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}
#endif /* ! HAVE_STRICMP */

#ifndef HAVE_STRUPR
char *strupr(char *s)
{
    char *retval = s;

    if (!s) return 0;
    while (*s) {
        *s = toupper(*s);
        s++;
    }
    return retval;
}
#endif

/*
 *                       S Y S T E M    I N F O
 */

const char *SystemName(int syscode)
{
    static const char *sysNames[] = {
        "PC MS-DOS",            /* 1 */
        "PC Win32/Windows NT",  /* 2 */
        "SPARC SunOS",          /* 3 */
        "HP-PA HP-UX",          /* 4 */
        "RS/6000 AIX",          /* 5 */
        "Sun Solaris",          /* 6 */
        "Alpha AXP OSF/1",      /* 7 */
        "PC Linux",             /* 8 */
        "DEC 5000 Ultrix",      /* 9 */
        "PC IBM OS/2",          /* 10 */
        "PowerMac System 7.5",  /* 11 */
        "i386 BSD/386 1.1",     /* 12 */
        "i386 FreeBSD 2.0",     /* 13 */
        "NeXT Mach 3.0",        /* 14 */
        "mc88100 DG/UX R4.11",  /* 15 */
        "SCO Unix 3.2 i386",    /* 16 */
        "SGI IRIX 5.4",         /* 17 */
        "i386 DG/UX R4.11",     /* 18 */
        "PC DJGPP V2.0"         /* 19 */
    };
#define numSysNames (sizeof(sysNames)/sizeof(sysNames[0]))

    if (--syscode < (int)numSysNames) return sysNames[syscode];
    return "Unknown System";
}


/*************************************************************
  $HISTORY:$
**************************************************************/

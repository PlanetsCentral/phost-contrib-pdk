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
 *																  *
 *	 Module:	  Command Line Option Processing				  *
 *	 Project:	  Portable Host 								  *
 *	 File Name:   getopt.c										  *
 *	 Programmer:  Carnegie Mellon								  *
 *	 Create Date:												  *
 *	 Description: This module implements the standard 'getopt'    *
 *				  command-line processing function. 			  *
 *	 Functions:   getopt										  *
 *	 Change Record: 											  *
 *																  *
 *																  *
 ******************************************************************/

/*	Copyright 1986 by Carnegie Mellon  */
/*	See permission and disclaimer notice in file "cmu-note.h"  */
/* #include    <cmu-note.h> */
/*
 * pgetopt - get option letter from argv
 */

#include <stdio.h>
#include "squishio.h"
#include <stdlib.h>
#include <string.h>
#include "phostpdk.h"
#include "private.h"

#ifdef __MSDOS__
#ifdef __cplusplus
extern "C" {
#endif
extern void _splitpath(const char *, char *, char *, char *, char *);
#ifdef __cplusplus
}
#endif
#endif

char	*poptarg;	 /* Global argument pointer. */
int poptind = 0; /* Global argv index. */

static char *scan = NULL;	/* Private scan pointer. */

int
pgetopt(int argc, char *argv[], char *optstring)
{
	register char c;
	register char *place;

	poptarg = NULL;

	if (scan == NULL || *scan == '\0') {
		if (poptind == 0)
			poptind++;

		if (poptind >= argc || argv[poptind][0] != '-' || argv[poptind][1] == '\0')
			return(EOF);
		if (strcmp(argv[poptind], "--")==0) {
			poptind++;
			return(EOF);
		}

		scan = argv[poptind]+1;
		poptind++;
	}

	c = *scan++;
	place = strchr(optstring, c);	/* DDP */

	if (place == NULL || c == ':') {
#ifdef __MSDOS__			/* ADS */
		char fname[256];

        _splitpath(argv[0],0,0,fname,0);
#else
#define fname argv[0]
#endif
		fprintf(stderr,"%s: unknown option -%c\n", fname, c);
		return('?');
	}

	place++;
	if (*place == ':') {
		if (*scan != '\0') {
			poptarg = scan;
			scan = NULL;
		} else {
            if (poptind >= argc) {
                fprintf(stderr, "%s: option -%c requires a parameter\n", argv[0], c);
                return('?');
            }
            poptarg = argv[poptind];
			poptind++;
		}
	}

	return(c);
}


/*************************************************************
  $HISTORY:$
**************************************************************/

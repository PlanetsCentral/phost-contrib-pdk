/**
   \file fcode.c
   \brief Friendly Code Functions
   
   All files in this distribution are Copyright (C) 1995-2002 by the program
   authors: Andrew Sterian, Thomas Voigt, and Steffen Pietsch.
   You can reach the PHOST team via email (phost@gmx.net).
   This file compiled by Stefan Reuther.

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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */

#include <string.h>
#include "phostpdk.h"
#include "private.h"

static const char EXTRA_FCODES_FILE[] = "xtrfcode.txt";

typedef char ExtraFCode_Type[4];

/** Array of special fcodes */
static ExtraFCode_Type *gExtraFCodes = 0;
/** Number of special fcodes */
static Uns16 gNumExtraFCodes = 0;
/** Number of allocated places in gExtraFCodes */
static Uns16 gSizeExtraFCodes = 0;
/** True iff the FCode list was changed in memory */
static Boolean gFCodesChanged = 0;
/** True iff this module was initialized */
static Boolean gFCodesInitialized = 0;
/** True iff this module has registered its cleanup function */
static Boolean gFCodesRegistered = 0;

/** Add one fcode to the list of special codes. */
static void PushFCode(const char* pCode)
{
    if (gNumExtraFCodes EQ gSizeExtraFCodes) {
        gSizeExtraFCodes += 16;
        if (gExtraFCodes) {
            gExtraFCodes = MemRealloc(gExtraFCodes,
                                      gSizeExtraFCodes*sizeof(ExtraFCode_Type));
        } else {
            gExtraFCodes = MemCalloc(gSizeExtraFCodes,
                                     sizeof(ExtraFCode_Type));
        }
    }
    strcpy(gExtraFCodes[gNumExtraFCodes], pCode);
    gNumExtraFCodes++;
}    

static Boolean isFCodeExtraSpecial(const char *pFCode)
{
    Uns16 lCode;

    for (lCode = 0; lCode < gNumExtraFCodes; lCode++) {
        if (0 EQ strncmp(pFCode, gExtraFCodes[lCode],
                         strlen(gExtraFCodes[lCode]))) return True;
    }
    return False;
}

static void AddPHostFCodes(void)
{
    /* The original list contained "PB md gs mi", which is overly broad */
    static const char lFCodes[][4] = {
        "mkt", "mdh", "mdq", "msc",
        "alt", "ald", "alm", "NAL",
        "HYP", "NTP", "ATT", "NUK",
        "WRS", "WRT", "lfm", "dmp",
        "bdm", "nat", "nad", "nam",
        "cln", "btt", "btf", "btm",
        "bum", "pop", "trg", "con",
        "nbr",

        "PB1", "PB2", "PB3", "PB4", "PB5", "PB6", "PB7", "PB8", "PB9",
        "md0", "md1", "md2", "md3", "md4", "md5", "md6", "md7", "md8", "md9",

        "gs1", "gs2", "gs3", "gs4", "gs5", "gs6", "gs7", "gs8", "gs9", "gsa", "gsb",
        "mi1", "mi2", "mi3", "mi4", "mi5", "mi6", "mi7", "mi8", "mi9", "mia", "mib"
    };
    unsigned i;
    for (i = 0; i < sizeof(lFCodes)/sizeof(lFCodes[0]); ++i) {
        if (!isFCodeExtraSpecial(lFCodes[i])) {
            /* These codes do not cause the file to be rewritten */
            PushFCode(lFCodes[i]);
        }
    }
}

static void ReadExtraFCodes(void)
{
    FILE *lFile;
    char lToken[4];

    lFile = OpenInputFile(EXTRA_FCODES_FILE, GAME_OR_ROOT_DIR | TEXT_MODE | NO_MISSING_ERROR);
    if (lFile) {
        while (fscanf(lFile, "%3s", lToken) NEQ EOF) {
            PushFCode(lToken);
        }
        fclose(lFile);
    }
    AddPHostFCodes();
}

/** API function: List special friendly codes on pFile */
void DumpExtraFCodes(FILE *pFile)
{
    Uns16 lCode;
    InitFCodes();

    passert(pFile);
    if (gExtraFCodes EQ 0) return;

    for (lCode = 0; lCode < gNumExtraFCodes; lCode++) {
        fprintf(pFile,"%s\n", gExtraFCodes[lCode]);
    }
}

/** API function: declare a friendly code as being special. Add-ons can use
    this to make known their codes. Returns true iff this actually was
    a change (i.e returns false if that was already a special code) */
Boolean DefineSpecialFCode(const char* pCode)
{
    passert(strlen(pCode) <= 3);
    InitFCodes();
    if (! isFCodeExtraSpecial(pCode)) {
        PushFCode(pCode);
        gFCodesChanged = True;
        return True;
    } else
        return False;
}

/** API Function: Initialize this module. Can be called explicitly; is
    called automatically when needed. */
void InitFCodes(void)
{
    if (gFCodesInitialized)
        return;
    gFCodesInitialized = True;
    ReadExtraFCodes();
    if (!gFCodesRegistered) {
        gFCodesRegistered = 1;
        RegisterCleanupFunction(ShutdownFCodes);
    }
}

/** API function: Shutdown this module. Can be called explicitly; is
    called automatically when needed. */
void ShutdownFCodes(void)
{
    FILE* lFile;
    if (!gFCodesInitialized)
        return;

    if (gFCodesChanged) {
        lFile = OpenOutputFile(EXTRA_FCODES_FILE, GAME_DIR_ONLY | TEXT_MODE);
        Info("Updating %s...", EXTRA_FCODES_FILE);
        DumpExtraFCodes(lFile);
        fclose(lFile);
    }
    gFCodesChanged = gFCodesInitialized = 0;
    gNumExtraFCodes = gSizeExtraFCodes = 0;
    MemFree(gExtraFCodes);
}

/** API function: True iff pFCode is a special code */
Boolean IsFCodeSpecial(const char* pFCode)
{
    InitFCodes();
    return isFCodeExtraSpecial(pFCode);
}

/** API function: True iff ship pShip has a special friendly code */
Boolean IsShipFCodeSpecial(Uns16 pShip)
{
    char tmp[4];
    return IsFCodeSpecial(ShipFC(pShip, tmp));
}

/** API function: True iff planet pPlanet has a special friendly code */
Boolean IsPlanetFCodeSpecial(Uns16 pPlanet)
{
    char tmp[4];
    return IsFCodeSpecial(PlanetFCode(pPlanet, tmp));
}

/** API function: True iff friendly codes p1 and p2 match. */
Boolean IsFCodeMatch(const char* p1, const char* p2)
{
    if (strcmp(p1, p2) == 0)
        return !IsFCodeSpecial(p1);
    else
        return False;
}

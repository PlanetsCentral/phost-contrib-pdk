
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
#include "battle.h"
#include "listmat.h"

#define PDK                     /* config.hi wants this */

static const char *CONFIG_FILE  = "pconfig.src";
static const char *HCONFIG_FILE = "hconfig.hst";
static const char* gCurrentConfigFile;

/** Used to mirror gConfigInfo. This is char * so we can
    index into it based upon byte position. */
static char *Data = 0;

static void DoDefaultAssignments(void);
static Boolean DoAssignment(const char *name, char *val,
                            const char *pInputLine);
static const char *languageName(Language_Def pLanguage);

Boolean gUsingTHost = False;    /*!< True if we are running in THost mode.
                                     This is the case when we loaded the
                                     HCONFIG file instead of PCONFIG.SRC. */

/* ---------------------------------------------------------------------- */

/** Config File Reader (original version).
    See also ConfigFileReaderEx for more information.
    \param pInFile    config file
    \param pFileName  name of this file, used for error reporting
    \param pSection   name of config file section to read
    \param pUseDefaultSection assume we're in the right section when
                      there's no section delimiter.
    \param pAssignFunc Function called for each line.
    \return True when everything went ok, False on error. When there
    is a syntax error in the config file, this function terminates
    by calling Error(). */
IO_Def
ConfigFileReader(FILE * pInFile, const char *pFileName, const char *pSection,
                 Boolean pUseDefaultSection, configAssignment_Func pAssignFunc)
{
  return ConfigFileReaderEx(pInFile, pFileName, pSection, pUseDefaultSection,
                            pAssignFunc, Error, True);
}

/** Config File Reader (Extended Version).
    Reads a configuration file in standard PHost format. In a nutshell:
    - comments with \#
    - section delimiters with \%, as in "\% PHOST"
    - assignments using the "=" character

    \param pInFile   input file. Must be open for reading in text mode.
    \param pFileName file name. Used for generating error messages.
    \param pSection  section to read. The name of the section, without
                     the percent sign, in either caps.
    \param pUseDefaultSection if true, assume that assignments before
                     a section delimiter belong to our section. If false,
                     only start parsing when we actually saw the
                     section delimiter.
    \param pAssignFunc assignment function. A function which takes
                     three `char*' pointers:
                     - const char* lhs: left-hand side (variable name)
                     - char* rhs: right-hand side (variable value)
                     - const char* line: the whole line
                     If the line is not an assignment, this is
                     called with lhs=rhs=NULL to allow copying comments
                     etc. If this is an assignment, the function can
                     return False to cause an error message be displayed.
    \param pError    error function. Called when an error occurs.
                     This can be one of Error, Warning or Info, or
                     an own function with a similar prototype.
    \param pContinue if true, continue after an error (after calling the
                     error function). If false, return IO_FAILURE.
    \return IO_SUCCESS (True) when okay, IO_FAILURE (False) on error. */
IO_Def
ConfigFileReaderEx(FILE * pInFile, const char *pFileName, const char *pSection,
      Boolean pUseDefaultSection, configAssignment_Func pAssignFunc,
      configError_Func pError, Boolean pContinue)
{
  char inbuf[256];
  char lInputLine[256];
  unsigned line;
  Boolean lInSection = pUseDefaultSection;

  line = 0;

  while (!feof(pInFile)) {
    char *p, *ep;

    line++;
    if (0 EQ fgets(inbuf, 255, pInFile)) {
      if (!feof(pInFile)) {
        pError("%s: unable to read file at line %u", pFileName, line);
        return IO_FAILURE;
      }
      break;
    }

    p = SkipWS(inbuf);
    if ((p EQ 0) OR(*p EQ '#')) {
      (*pAssignFunc) (0, 0, inbuf); /* Allow copy of line */
      continue;
    }

    if (*p EQ '%') {
      char *lSection;

      (*pAssignFunc) (0, 0, inbuf); /* Copy line */

      lSection = strtok(p + 1, " \t\n\r");
      if (lSection EQ 0)
        continue;

      if (stricmp(lSection, pSection) EQ 0) {
        /* We're starting our target section */
        lInSection = True;
      }
      else {
        /* Some other section */
        lInSection = False;
      }
      continue;
    }

    if (!lInSection) {
      (*pAssignFunc) (0, 0, inbuf);
      continue;
    }

    /* Save input line */
    strcpy(lInputLine, inbuf);

    /* We have a name on the LHS */
    ep = strchr(p, '=');
    if (ep EQ 0) {
      pError("%s: expected assignment at line %u", pFileName, line);
      if (pContinue)
        continue;
      else
        return IO_FAILURE;
    }

    *ep++ = 0;
    ep = SkipWS(ep);
    if (ep EQ 0) {
      pError("%s: expecting value to assign at line %u", pFileName, line);
      if (pContinue)
        continue;
      else
        return IO_FAILURE;
    }
    TrimTrailingWS(ep);

    TrimTrailingWS(p);
    if (*p EQ 0) {
      pError("%s: expecting parameter name to assign to at line %u", pFileName,
             line);
      if (pContinue)
        continue;
      else
        return IO_FAILURE;
    }

    if (!(*pAssignFunc) (p, ep, lInputLine)) {
      pError("%s: parameter error at line %u", pFileName, line);
      if (pContinue)
        continue;
      else
        return IO_FAILURE;
    }
  }
  return IO_SUCCESS;
}

/** Read host configuration. Reads the pconfig.src file.
    \return IO_SUCCESS on success, IO_FAILURE on error. */
IO_Def
Read_HConfig_File(void)
{
  FILE *lConfigFile;
  IO_Def lRes;

  lConfigFile = OpenInputFile(CONFIG_FILE, GAME_DIR_ONLY | NO_MISSING_ERROR);
  if (lConfigFile == NULL)
    return IO_FAILURE;

  Data = (char *) gConfigInfo;
  gCurrentConfigFile = CONFIG_FILE;

  DoDefaultAssignments();

  lRes = ConfigFileReaderEx(lConfigFile, CONFIG_FILE, "phost", True,
                            DoAssignment, Warning, True);

  fclose(lConfigFile);

  return lRes;
}

/* ---------------------------------------------------------------------- */

/* Scoring info is put in here just because it's too trivial */
typedef Uns16 ScoreMethod_Def;

typedef enum {
  CFType_Uns8,
  CFType_Uns16,
  CFType_Int16,
  CFType_Uns32,
  CFType_Boolean,
  CFType_Language_Def,
  CFType_ScoreMethod_Def,
  CFType_char,
  CFType_NoType
} CFType;
typedef char NoType;

#define CFDefine(name, member, num, type, def, MA, Min, Max) #name,
static const char *Names[] = {
#include "config.hi"
  0
};

#define CF_NumItems ((sizeof(Names)/sizeof(Names[0]))-1)

#define CFDefine(name, member, num, type, def, MA, Min, Max) offsetof(Hconfig_Struct, member),
static size_t Pos[] = {
#include "config.hi"
  0
};

#define CFDefine(name, member, num, type, def, MA, Min, Max) num,
static Uns16 Elements[] = {
#include "config.hi"
  0
};

#define CFDefine(name, member, num, type, def, MA, Min, Max) CFType_ ## type ,
static CFType Types[] = {
#include "config.hi"
  CFType_NoType
};

#define CFDefine(name, member, num, type, def, MA, Min, Max) def,
static const char *Defaults[] = {
#include "config.hi"
  0
};

#define CFDefine(name, member, num, type, def, MA, Min, Max) MA,
static Boolean gAllowMA[] = {
#include "config.hi"
  0
};

#define CFDefine(name, member, num, type, def, MA, Min, Max) CFI_ ## name,
enum {
#include "config.hi"
    CFI_MAX_INDEX
};

static Boolean UserSet[CF_NumItems];

static int readUns8(Uns16 ix, char *val);
static int readUns16(Uns16 ix, char *val);
static int readInt16(Uns16 ix, char *val);
static int readUns32(Uns16 ix, char *val);
static int readBooleanType(Uns16 ix, char *val);
static int readLanguageType(Uns16 ix, char *val);
static int readScoreMethodType(Uns16 ix, char *val);
static int readCharType(Uns16 ix, char *val);

/** Get name of a language. \internal */
static const char *
languageName(Language_Def pLanguage)
{
  static const char *lLang[] = {
    "English",
    "German",
    "French",
    "Spanish",
    "Italian",
    "Dutch",
    "Russian",
    "Estonian"
  };

  return lLang[pLanguage];
}

/** Check assignment for a valid parameter value. Boolean-type
    assignments need not be checked since they've already been
    verified by ListMatch.
    \param ix index into config.hi (number of config option)
    \param val desired value for that option
    \internal */
static int
checkValue(Uns16 ix, long val)
{
  static struct {
    long minval;
    long maxval;
  } sCheck[CF_NumItems] = {
#define CFDefine(name, member, num, type, def, MA, Min, Max) { Min, Max },
#include "config.hi"
  };
  passert(ix < CF_NumItems);
  return (val >= sCheck[ix].minval AND val <= sCheck[ix].maxval);
}

/** true while DoDefaultAssignments is active.
    When this variable is true, we don't print out any errors w.r.t.
    assigning to obsolete variables
    \internal */
static Boolean gAssigningDefaults = False;

/** Assign default values to config options. \internal */
static void
DoDefaultAssignments(void)
{
  int ix;
  char defstr[1024];

  gAssigningDefaults = True;

  for (ix = 0; ix < CF_NumItems; ix++) {
    strcpy(defstr, Defaults[ix]);
    DoAssignment(Names[ix], defstr, 0);
  }
  memset(UserSet, 0, sizeof(UserSet));

  gAssigningDefaults = False;
}

/** Assign one config option. This is the configAssignment_Func used
    to parse pconfig.src. \internal */
static Boolean
DoAssignment(const char *name, char *val, const char *lInputLine)
{
  Uns16 ix;

  if (name EQ 0 OR val EQ 0)
    return True;

  (void) lInputLine;

  for (ix = 0; ix < CF_NumItems; ix++) {
    if (!stricmp(Names[ix], name))
      break;
  }
  if (ix >= CF_NumItems) {
    Warning("%s: Parameter '%s' is not recognized", gCurrentConfigFile, name);
    return False;
  }

  if (!gAssigningDefaults AND UserSet[ix]) {
    Warning("%s: Parameter '%s' is being assigned twice", gCurrentConfigFile,
            Names[ix]);
    return False;
  }

  UserSet[ix] = True;

  /* Check for obsolete parameters */
  if (!gAssigningDefaults) {
    switch (ix) {
    case 10:                   /* old TaxRate */
    case 158:                  /* old ColFighterSweepRate */
    case 159:                  /* old ColFighterSweepRange */
      Warning("%s: the '%s' parameter is obsolete", gCurrentConfigFile, Names[ix]);
      return False;

    default:
      break;
    }
  }

  switch (Types[ix]) {
  case CFType_Uns8:
    return readUns8(ix, val);
  case CFType_Uns16:
    return readUns16(ix, val);
  case CFType_Int16:
    return readInt16(ix, val);
  case CFType_Uns32:
    return readUns32(ix, val);
  case CFType_Boolean:
    return readBooleanType(ix, val);
  case CFType_Language_Def:
    return readLanguageType(ix, val);
  case CFType_ScoreMethod_Def:
    return readScoreMethodType(ix, val);
  case CFType_char:
    return readCharType(ix, val);
  default:
    passert(0);
  }

  return True;
}

static int
checkEOLGarbage(void)
{
  char *p;

  p = strtok(0, ", \t");

  if (p == 0)
    return 1;

  if (*p == '#')
    return 1;

  Warning("%s: extraneous data beyond assignment", gCurrentConfigFile);
  return 0;
}

static int
getLong(char *str, long *retval, Boolean pAllowEOL)
{
  char *endptr;
  char *p;

  p = strtok(str, ", \t");
  if (p EQ 0) {
    if (!pAllowEOL) {
      Warning("%s: not enough elements in assignment", gCurrentConfigFile);
    }
    return 0;
  }

  *retval = strtol(p, &endptr, 0);
  if (*endptr NEQ 0) {
    Warning("%s: illegal numeric value '%s'", gCurrentConfigFile, str);
    return 0;
  }
  return 1;
}

static int
readUns8(Uns16 ix, char *val)
{
  long uval;
  Uns16 n = Elements[ix];
  Uns16 i;
  Boolean lAllowEOL = gAllowMA[ix];
  Boolean lDoMA = False;

  for (i = 0; i < n; i++) {
    if (!lDoMA) {
      lAllowEOL = (i EQ 1) ? True : False;
      if (!getLong(val, &uval, lAllowEOL)) {
        if (!lAllowEOL)
          return 0;

        lDoMA = True;
      }
    }

    if (!lDoMA) {
      val = 0;
      if (uval < 0 OR uval > 255) {
        Warning("%s: illegal 8-bit quantity '%ld'", gCurrentConfigFile, uval);
        return 0;
      }
      if (!checkValue(ix, uval))
        return 0;
    }

    *(Uns8 *) (Data + Pos[ix] + i * sizeof(Uns8)) = (Uns8) uval;
  }

  return checkEOLGarbage();
}

static int
readUns16(Uns16 ix, char *val)
{
  long uval;
  Uns16 n = Elements[ix];
  Uns16 i;
  Boolean lAllowEOL = gAllowMA[ix];
  Boolean lDoMA = False;

  for (i = 0; i < n; i++) {
    if (!lDoMA) {
      lAllowEOL = (i EQ 1) ? True : False;
      if (!getLong(val, &uval, lAllowEOL)) {
        if (!lAllowEOL)
          return 0;

        lDoMA = True;
      }
    }

    if (!lDoMA) {
      val = 0;
      if (uval < 0 OR uval > 65535U) {
        Warning("%s: illegal 16-bit unsigned quantity '%ld'", gCurrentConfigFile,
                uval);
        return 0;
      }
      if (!checkValue(ix, uval))
        return 0;
    }

    *(Uns16 *) (Data + Pos[ix] + i * sizeof(Uns16)) = (Uns16) uval;
  }

  return checkEOLGarbage();
}

static int
readInt16(Uns16 ix, char *val)
{
  long uval;
  Uns16 n = Elements[ix];
  Uns16 i;
  Boolean lAllowEOL = gAllowMA[ix];
  Boolean lDoMA = False;

  for (i = 0; i < n; i++) {
    if (!lDoMA) {
      lAllowEOL = (i EQ 1) ? True : False;
      if (!getLong(val, &uval, lAllowEOL)) {
        if (!lAllowEOL)
          return 0;

        lDoMA = True;
      }
    }

    if (!lDoMA) {
      val = 0;
      if (uval < -32767 OR uval > 32767) {
        Warning("%s: illegal 16-bit signed quantity '%ld'", gCurrentConfigFile, uval);
        return 0;
      }
      if (!checkValue(ix, uval))
        return 0;
    }

    *(Int16 *) (Data + Pos[ix] + i * sizeof(Int16)) = (Int16) uval;
  }

  return checkEOLGarbage();
}

static int
readUns32(Uns16 ix, char *val)
{
  long uval;
  Uns16 n = Elements[ix];
  Uns16 i;
  Boolean lAllowEOL = gAllowMA[ix];
  Boolean lDoMA = False;

  for (i = 0; i < n; i++) {
    if (!lDoMA) {
      lAllowEOL = (i EQ 1) ? True : False;
      if (!getLong(val, &uval, lAllowEOL)) {
        if (!lAllowEOL)
          return 0;

        lDoMA = True;
      }
    }

    if (!lDoMA) {
      val = 0;

      if (!checkValue(ix, uval))
        return 0;
    }
    *(Uns32 *) (Data + Pos[ix] + i * sizeof(Uns32)) = (Uns32) uval;
  }

  return checkEOLGarbage();
}

static int
readBooleanType(Uns16 ix, char *val)
{
  Uns16 n = Elements[ix];
  Uns16 i;
  char *p;
  Boolean lDoMA = False;
  int match;

  for (i = 0; i < n; i++) {
    if (!lDoMA) {
      p = strtok(val, ", \t");
      val = 0;

      if (p EQ 0) {
        if (i EQ 1 AND gAllowMA[ix]) {
          lDoMA = True;
        } else {
          Warning("%s: not enough elements in assignment", gCurrentConfigFile);
          return 0;
        }
      }

      if (!lDoMA) {
        match = ListMatch(p, "False True No Yes"); /* order is important */
        if (match < 0) {
          Warning("%s: illegal boolean parameter '%s'", gCurrentConfigFile, p);
          return 0;
        }
        if (match > 1)
          match -= 2;           /* Convert no/yes to false/true */
      }
    }

    *(Boolean *) (Data + Pos[ix] + i * sizeof(Boolean)) = (Boolean) match;
  }

  return checkEOLGarbage();
}

static int
readLanguageType(Uns16 ix, char *val)
{
  Uns16 n = Elements[ix];
  Uns16 i;
  char *p;
  int match;
  Boolean lDoMA = False;

  for (i = 0; i < n; i++) {
    if (!lDoMA) {
      p = strtok(val, ", \t");
      val = 0;

      if (p == 0) {
        if (i EQ 1 AND gAllowMA[ix]) {
          lDoMA = True;
        } else {
          Warning("%s: not enough elements in assignment", gCurrentConfigFile);
          return 0;
        }
      }

      if (!lDoMA) {
        /* The order of languages in the following list is important and must 
           match the order of definitions in strlang.h */
        match =
              ListMatch(p,
              "ENglish German French Spanish Italian Dutch Russian EStonian");

        if (match < 0) {
          Warning("%s: illegal language '%s'", gCurrentConfigFile, p);
          return 0;
        }
      }
    }
    *(Language_Def *) (Data + Pos[ix] + i * sizeof(Language_Def)) =
          (Language_Def) match;
  }

  return checkEOLGarbage();
}

static int
readScoreMethodType(Uns16 ix, char *val)
{
  Uns16 n = Elements[ix];
  Uns16 i;
  char *p;
  int match;

  for (i = 0; i < n; i++) {
    p = strtok(val, ", \t");
    val = 0;

    if (p == 0) {
      Warning("%s: not enough elements in assignment", gCurrentConfigFile);
      return 0;
    }

    /* The order of methods in the following list is important and must match 
       the order of definitions in auxdata.c */
    match = ListMatch(p, "None Compatible");

    if (match < 0) {
      Warning("%s: illegal scoring method '%s'", gCurrentConfigFile, p);
      return 0;
    }
    *(ScoreMethod_Def *) (Data + Pos[ix] + i * sizeof(ScoreMethod_Def)) =
          (ScoreMethod_Def) match;
  }

  return checkEOLGarbage();
}

static int
readCharType(Uns16 ix, char *val)
{
  Uns16 n = Elements[ix] - 1;

  if (strlen(val) > n) {
    Warning("%s: assignment to '%s' exceeds string length", gCurrentConfigFile,
          Names[ix]);
  }
  n = min(n, strlen(val));

  memset(Data + Pos[ix], 0, Elements[ix]);
  strncpy(Data + Pos[ix], val, n);

  return 1;
}

typedef struct {
    CFType   mType;             /*!< type (CFType_Boolean, CFType_Uns16, CFType_Uns32) */
    short    mCount;            /*!< number of items  */
    unsigned mHOffset;          /*!< offset into HCONFIG */
    unsigned mPIndex;           /*!< number of Pconfig_Struct member (CFI_xxx) */
} Hconfig_Import_Struct;

static Hconfig_Import_Struct gHconfigImport[] = {
#define TCDefine(type, count, offset, name)  { CFType_ ## type, count, offset, CFI_ ## name },
#include "tconfig.hi"
#undef TCDefine
};
/* maximum size of hconfig we support. */
#define THOST_HCONFIG_SIZE 500

/** Load HCONFIG.HST. This loads the HCONFIG.HST file and parses
    it into the gPconfigInfo struct. Parameters not read from
    HCONFIG.HST are set to defaults. */
IO_Def
Read_THost_HConfig_File(void)
{
  FILE   *lConfigFile;
  IO_Def lRes;
  char   *lBuffer;
  Hconfig_Import_Struct* lItem;
  int    lSize;
  int    i, j;

  lConfigFile = OpenInputFile(HCONFIG_FILE, GAME_DIR_ONLY | NO_MISSING_ERROR);
  if (lConfigFile == NULL)
    return IO_FAILURE;

  Data = (char *) gConfigInfo;
  gCurrentConfigFile = HCONFIG_FILE;

  DoDefaultAssignments();

  lBuffer = MemAlloc(THOST_HCONFIG_SIZE);
  lSize = fread(lBuffer, 1, THOST_HCONFIG_SIZE, lConfigFile);
  lRes = IO_SUCCESS;

  for(i = 0, lItem = gHconfigImport; i < sizeof(gHconfigImport) / sizeof(gHconfigImport[0]); ++i, ++lItem) {
    /* Convert item to string, and assign that. This is
       not the performance king, but safe and easy to maintain.
       In particular, it has no problems with arrayized options. */
    char lValue[100];         /* max size currently is 11 Uns16's */
    char* lPtr = lValue;
    char* lData = lBuffer + lItem->mHOffset;
    for (j = 0; j < lItem->mCount; ++j) {
      if (j)
        *lPtr++ = ',';
      if (lData - lBuffer > lSize)  /* item not (completely) in hconfig.hst */
        goto next_item;
      switch (lItem->mType) {
       case CFType_Boolean:
          strcpy(lPtr, (ReadDOSUns16(lData) ? "Yes" : "No"));
          lData += 2;
          break;
       case CFType_Uns32:
          sprintf(lPtr, "%lu", ReadDOSUns32(lData));
          lData += 4;
          break;
       case CFType_Uns16:
          sprintf(lPtr, "%u", ReadDOSUns16(lData));
          lData += 2;
          break;
       default:
          passert(0);       /* can't happen */
      }
      lPtr += strlen(lPtr);
    }

    /* lPtr now is a complete assignment string. */
    switch(Types[lItem->mPIndex]) {
     case CFType_Uns16:
        if (!readUns16(lItem->mPIndex, lValue))
          lRes = IO_FAILURE;
        break;
     case CFType_Uns32:
        if (!readUns32(lItem->mPIndex, lValue))
          lRes = IO_FAILURE;
        break;
     case CFType_Boolean:
        if (!readBooleanType(lItem->mPIndex, lValue))
          lRes = IO_FAILURE;
        break;
     default:;
    }
  next_item:
  }
  if (lRes == IO_FAILURE)
    Warning("%s: some options had invalid values and were ignored", HCONFIG_FILE);

  MemFree(lBuffer);
  fclose(lConfigFile);
  
  gUsingTHost = True;

  return lRes;
}

/*************************************************************
  $HISTORY:$
**************************************************************/

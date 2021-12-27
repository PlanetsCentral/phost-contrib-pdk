
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

#include <ctype.h>
#include <errno.h>
#include "phostpdk.h"
#include "private.h"
#include "battle.h"
#include "listmat.h"

#define PDK                     /* config.hi wants this */

extern const char SHIPLIST_FILE[];
static const char CONFIG_FILE[]  = "pconfig.src";
static const char HCONFIG_FILE[] = "hconfig.hst";
static const char* gCurrentConfigFile;

static void ClearConfig();
static void DoDefaultAssignments(void);
static Boolean DoAssignment(const char *name, char *val,
                            const char *pInputLine);

Boolean gUsingTHost = False;    /*!< True if we are running in THost mode.
                                     This is the case when we loaded the
                                     HCONFIG file instead of PCONFIG.SRC. */
Boolean gConfigWarnings = False;

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
    if ((p EQ 0) OR(*p EQ '#') OR(*p EQ ';')) {
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

static void
Ignore(const char *pStr, ...)
{
    /* intentionally left blank */
    (void) pStr;
}


/** Read some phost configuration file.

    This function is meant as a flexible version of Read_HConfig_File,
    enabling you to choose which config file gets read, if the current
    config should be cleaned and if the defaults should be assigned.

    \return IO_SUCCESS on success, IO_FAILURE on error.
    See also ConfigFileReaderEx for more information.
    \param pInFile    config file
    \param pFileName  name of this file, used for error reporting
    \param pSection   name of config file section to read
    \param pUseDefaultSection assume we're in the right section when
                      there's no section delimiter.
    \param clear      If True then clear the current config.
    \param setDefaults If True then do the default assignments after
                      reading the current config file.
*/
IO_Def
Read_SomeConfig_File(FILE *pInFile, const char *pFileName, \
                     const char *pSection, Boolean pUseDefaultSection,
                     Boolean clear, Boolean setDefaults)
{
    if ( clear ) {
        ClearConfig();
    }

    gCurrentConfigFile = pFileName;
    if ( ConfigFileReaderEx(pInFile, pFileName, pSection, pUseDefaultSection,
                            DoAssignment, \
                            gConfigWarnings ? Warning : Ignore, True) \
         EQ IO_FAILURE) {
        return IO_FAILURE;
    }

    if ( setDefaults ) {
        DoDefaultAssignments();
    }

    ReinitWraparound();
  
    return IO_SUCCESS;
}

/** Read host configuration. Reads the pconfig.src file.
    \return IO_SUCCESS on success, IO_FAILURE on error. */
IO_Def
Read_HConfig_File(void)
{
  FILE *lConfigFile;

  lConfigFile = OpenInputFile(CONFIG_FILE, GAME_DIR_ONLY | NO_MISSING_ERROR);
  if (lConfigFile == NULL)
    return IO_FAILURE;

  ClearConfig();
  gCurrentConfigFile = CONFIG_FILE;
  ConfigFileReaderEx(lConfigFile, CONFIG_FILE, "phost", True,
                     DoAssignment, gConfigWarnings ? Warning : Ignore, True);
  fclose(lConfigFile);

  lConfigFile = OpenInputFile(SHIPLIST_FILE, GAME_DIR_ONLY | NO_MISSING_ERROR);
  if (lConfigFile) {
      gCurrentConfigFile = SHIPLIST_FILE;
      ConfigFileReaderEx(lConfigFile, SHIPLIST_FILE, "phost", False,
                         DoAssignment, gConfigWarnings ? Warning : Ignore, True);
      fclose(lConfigFile);
  }
  DoDefaultAssignments();

  ReinitWraparound();
  
  return IO_SUCCESS;
}

/** Initialize blank configuration. */
void
InitConfig(void)
{
    Boolean save = gConfigWarnings;
    gConfigWarnings = False;
    
    ClearConfig();
    DoDefaultAssignments();
    ReinitWraparound();

    gConfigWarnings = save;
}

/* ---------------------------------------------------------------------- */

/* Scoring info is put in here just because it's too trivial */
typedef Uns16 ScoreMethod_Def;

/************************* PHost 4 Config Reader *************************/

struct ConfigItem_Struct;

typedef enum {
    CFType_Uns16,
    CFType_Int16,
    CFType_Uns32,
    CFType_Boolean,
    CFType_Language_Def,
    CFType_ScoreMethod_Def,
    CFType_String,
    CFType_BuildQueue_Def,
    CFType_Tristate,
    CFType_Cost,
    CFType_MAX
} CFType;

static const int sConfigItemSize[CFType_MAX] = {
    sizeof(Uns16),
    sizeof(Int16),
    sizeof(Uns32),
    sizeof(Boolean),
    sizeof(Language_Def),
    sizeof(ScoreMethod_Def),
    sizeof(char),
    sizeof(BuildQueue_Def),
    sizeof(Tristate),
    sizeof(Cost_Struct)
};

static const char* const sConfigItemNames[CFType_MAX] = {
    "unsigned 16 bit",
    "signed 16 bit",
    "unsigned 32 bit",
    "boolean",
    "language",
    "scoring method",
    "string",
    "build queue",
    "tristate",
    "cost"
};

typedef Boolean (*ConfigReader_Func)(const struct ConfigItem_Struct*, char* value);

typedef struct ConfigItem_Struct {
    const char* mName;          /* Name of config item */
    size_t      mOffset : 16;   /* Position in gConfigInfo */
    Uns16       mCount : 8;     /* Number of entries (for strings: length) */
    CFType      mType : 8;      /* Type */
    Int32       mMin, mMax;     /* Range */
    Uns16       mFlags;         /* Options (arrayized, obsolete, ...) */
    Uns16       mDepend;        /* Dependency */
    const char* mDefault;       /* Default value */
} ConfigItem_Struct;

enum {
#define CFDefine4(name, ofs, count, typ, min, max, flags, dep, def) CF_ ## name,
#include "config.hi4"
    CF_NumItems
};

/*
   Type checking. This checks *at compile time* that all dependency
   settings are correct (i.e. refer to a single boolean option).
   When such a dependency is invalid, you'll get an error message
   about TYPECK_name_Boolean_1 being undefined, where name is the name
   of the referred-to option.

   config.hi4:35: `TYPECK_RecycleRate_Boolean_1' undeclared here (not in a function)
   config.hi4:35: enumerator value for `TYPECK2_DisablePasswords' not integer constant
*/
enum {
#define CFDefine4(name, ofs, count, typ, min, max, flags, dep, def) \
    TYPECK_ ## name ## _ ## typ ## _ ## count,
#include "config.hi4"
#undef CFDefine4
#define IFYES(x) TYPECK_ ## x ## _Boolean_1
#define IFNO(x) TYPECK_ ## x ## _Boolean_1
#define CFDefine4(name, ofs, count, typ, min, max, flags, dep, def) \
    TYPECK2_ ## name = dep,
#include "config.hi4"
    TYPECK3_Ends_here
#undef CFDefine4
#undef IFYES
#undef IFNO
};
/*
   Some more checking. The 'ofs' values are generally either 'Name' or
   'Name[1]'. Thus, the size of 'ofs' should either be the size of the
   option type times count (first case) or just size of the option
   type. In case an option is arrayized and we forget to change the
   config.hi4 definition, this will fail with an "array size negative"
   error.
 */
#define CFDefine4(name, ofs, count, typ, min, max, flags, dep, def) \
    typedef int SIZECK_##name[(sizeof(gConfigInfo->ofs) == sizeof(typ) \
                              || sizeof(gConfigInfo->ofs) == sizeof(typ) * (count)) ? 2 : -2];
#define String char
#define Cost Cost_Struct
#include "config.hi4"
#undef String
#undef Cost
#undef CFDefine4

enum {
    CFOPT_Arrayized    = 1,
    CFOPT_Obsolete     = 2,
    CFOPT_Optional     = 4,
    CFOPT_Incompatible = 8
};

enum {
    CF_MAX_ARRAY_SIZE = 20      /* 3.3f: actual limit is 12 */
};

static const ConfigItem_Struct sConfigDef[CF_NumItems] = {
#define ARRAYIZED CFOPT_Arrayized
#define OBSOLETE  CFOPT_Obsolete
#define OPTIONAL  CFOPT_Optional
#define INCOMPATIBLE CFOPT_Incompatible
#define RACEARRAY 0     /* PHost does not need this (yet?) */
#define IFYES(x)  (2 * CF_ ## x + 2)
#define IFNO(x)   (2 * CF_ ## x + 3)
#define CFDefine4(name, ofs, count, typ, min, max, flags, dep, def) \
    { #name, offsetof(Hconfig_Struct, ofs), count, CFType_ ## typ, min, max, flags, dep, def },
#include "config.hi4"
#undef ARRAYIZED
#undef OBSOLETE
#undef RACEARRAY
#undef OPTIONAL
#undef INCOMPATIBLE
#undef IFYES
#undef IFNO
};

/** nonzero if option has been set. Use `char' not Boolean, because
    that occupies less space */
static char sOptionSet[CF_NumItems];

/** Look up an option, by name.
    \return index into sConfigDef, or -1 */
static int
lookupOptionByName(const char* pName)
{
    int i;
    for (i = 0; i < CF_NumItems; ++i)
        if (stricmp(pName, sConfigDef[i].mName) == 0)
            return i;
#define CFAlias(old,new) if (stricmp(pName, #old) == 0) return CF_##new;
#include "config.hi4"
#undef CFAlias
    return -1;
}

/*---------- Assignment Functions ---------*/

/* This must be a file-scope variable, otherwise Turbo C won't grok
   it. */
static const char SEPARATORS[] = " \t,\r\032";
static const char WHITESPACE[] = " \t\r\032";
static const char SEP_COMMA[] = ",";

/** Generic Tokenizer. Handles the following
    - splitting into pieces (a, b, c)
    - arrayisation and verification of array bounds
    - comments
    \param pItem   Config item we're parsing
    \param pValue  Value (string), must be modifyable
    \param pFunc   Function to call for each item (if everything goes
                   well, this function is called exactly pItem->mCount times,
                   with index \in [0, pItem->mCount-1])
    \param pCommaOnly Allow only commas as separators. */
static Boolean Tokenize(const struct ConfigItem_Struct* pItem,
                        char* pValue,
                        Boolean (*pFunc)(const ConfigItem_Struct*,
                                         char* value, int index, void* data),
                        void* pData,
                        Boolean pCommaOnly)
{
    int lIndex;
    int lLength;
    char lEol;
    char* lStr;

    const char* lSep = pCommaOnly ? SEP_COMMA : SEPARATORS;

    /* Kill comment */
    if ((lStr = strchr(pValue, '#')) != 0)
        *lStr = 0;

    lStr = 0;
    lIndex = 0;
    while (1) {
        pValue += strspn(pValue, lSep);
        lLength = strcspn(pValue, lSep);

        if (lLength == 0)
            break;
        /* we have an item we can assign now */
        lStr = pValue;
        lEol = pValue[lLength];
        pValue[lLength] = 0;
        pValue += lLength+1;

        if (lIndex >= pItem->mCount) {
            Error("%s: too many values in assignment to '%s'",
                  CONFIG_FILE, pItem->mName);
            return False;
        }
        if (! pFunc(pItem, lStr, lIndex, pData))
            return False;

        ++lIndex;
        if (lEol == 0)
            break;
    }

    /* End of line reached. Valid? */
    if (lIndex < pItem->mCount) {
        if ((pItem->mFlags & CFOPT_Arrayized) && lStr) {
            while (lIndex < pItem->mCount) {
                if (! pFunc(pItem, lStr, lIndex, pData))
                    return False;
                ++lIndex;
            }
        } else {
            Error("%s: too few values in assignment to '%s'", CONFIG_FILE, pItem->mName);
            return False;
        }
    }
    return True;
}

/** Parse integer into a Int32 array. For use with Tokenize. This one
    checks bounds. */
static Boolean
parseInteger(const ConfigItem_Struct* pItem,
             char* pValue, int pIndex, void* pData)
{
    long i;
    char* p;

    i = strtol(pValue, &p, 10);
    if (*pValue == 0 || *p != 0 || ((i == LONG_MIN || i == LONG_MAX) && errno == ERANGE)) {
        Error("%s: invalid value '%s' in assignment to '%s'", CONFIG_FILE,
              pValue, pItem->mName);
        return False;
    }
    if (i < pItem->mMin || i > pItem->mMax) {
        /* brute force error exit on parameter error. This is needed
           to guarantee correct operation when for example users run a
           non-999 enabled program on a game directory with
           NumShips = 999 */
        ErrorExit("%s: value %ld outside allowed range [%ld, %ld] in assignment to '%s'",
                  CONFIG_FILE, i, (long)pItem->mMin, (long)pItem->mMax, pItem->mName);
        return False;
    }

    ((Int32*)pData)[pIndex] = i;
    return True;
}

/** Parse boolean, into a boolean array. For use with Tokenize. */
static Boolean
parseBoolean(const ConfigItem_Struct* pItem,
             char* pValue, int pIndex, void* pData)
{
    int match = ListMatch(pValue, "False True No Yes"); /* order is important */
    if (match < 0 && ListMatch(pValue, "Allies External") >= 0)
        match = 1;
    if (match < 0) {
        Error("%s: invalid boolean value for option '%s'", CONFIG_FILE, pItem->mName);
        return False;
    } else {
        if (match >=2 )
            match -= 2;
        ((Boolean*) pData)[pIndex] = match;
        return True;
    }
}

/** Parse cost option. */
static Boolean
parseCost(const ConfigItem_Struct* pItem,
          char* pValue, int pIndex, void* pData)
{
    Cost_Struct lValue = {{ 0 }};
    Uns16* lCurrent = 0;
    Boolean lNeedNumber = False;
    Boolean lHadAny = False;

    while (*pValue != 0) {
        if (lCurrent == 0) {
            /* we're skipping across the whitespace before a
               specification */
            if (isspace((unsigned char) *pValue)) {
                /* nix */
            } else {
                switch (toupper((unsigned char) *pValue)) {
                 case 'T': lCurrent = &lValue.mCost[TRITANIUM]; break;
                 case 'D': lCurrent = &lValue.mCost[DURANIUM]; break;
                 case 'M': lCurrent = &lValue.mCost[MOLYBDENUM]; break;
                 case '$': lCurrent = &lValue.mCost[CREDITS]; break;
                 case 'S': lCurrent = &lValue.mCost[SUPPLIES]; break;
                 default:
                    Error("%s: invalid cost for option '%s', %s", CONFIG_FILE, pItem->mName,
                          "need cargo type");
                    return False;
                }
            }
            if (lCurrent) {
                *lCurrent = 0;
                lNeedNumber = True;
                lHadAny = True;
            }
        } else {
            /* we're reading a number */
            if (*pValue >= '0' && *pValue <= '9') {
                Uns32 lNewValue = *lCurrent * 10 + (*pValue - '0');
                if (lNewValue > 10000) {
                    Error("%s: invalid cost for option '%s', %s", CONFIG_FILE, pItem->mName,
                          "amount too large");
                    return False;
                }
                *lCurrent = lNewValue;
                lNeedNumber = False;
            } else if (isspace((unsigned char) *pValue) && !lNeedNumber) {
                lCurrent = 0;
            } else {
                goto need_number;
            }
        }
        ++pValue;
    }

    if (lNeedNumber) {
        goto need_number;
    }
    if (!lHadAny) {
        Error("%s: invalid cost for option '%s', %s", CONFIG_FILE, pItem->mName,
              "value is empty");
        return False;
    }

    ((Cost_Struct*) pData)[pIndex] = lValue;
    return True;

 need_number:
    Error("%s: invalid cost for option '%s', %s", CONFIG_FILE, pItem->mName,
          "need number");
    return False;
}

/** Parse tristate, into a tristate array. For use with Tokenize. */
static Boolean
parseTristate(const ConfigItem_Struct* pItem,
             char* pValue, int pIndex, void* pData)
{
    int match = ListMatch(pValue, "False True Allies External No Yes"); /* order is important */
    if (match >= 4)
        match -= 4;
    if (match < 0 || match > pItem->mMax) {
        Error("%s: invalid value for option '%s'", CONFIG_FILE, pItem->mName);
        return False;
    } else {
        ((Tristate*) pData)[pIndex] = match;
        return True;
    }
}

/** Parse language into a Language_Def array. For use with Tokenize. */
static Boolean
parseLanguage(const ConfigItem_Struct* pItem,
              char* pValue, int pIndex, void* pData)
{
    int match = ListMatch(pValue,
                          "ENglish German French Spanish Italian Dutch Russian EStonian"
                          " NEWENglish Polish");

    /* Ignore this parameter */
    (void) pItem;

    /* PHost 3.5/4.1 allows other languages than our hardwired set.
       Thus, if we don't know it, assume it's NewEnglish. */
    if (match < 0)
        match = LANG_NewEnglish;

    ((Language_Def*) pData)[pIndex] = match;
    return True;
}

/** Parse score method into an array of ScoreMethod_Def. For use with
    Tokenize. */
static Boolean
parseScoreMethod(const ConfigItem_Struct* pItem,
                 char* pValue, int pIndex, void* pData)
{
    int match = ListMatch(pValue, "None Compatible");
    (void) pItem;
    if (match < 0) {
        Error("%s: invalid scoring method '%s'", CONFIG_FILE, pValue);
        return False;
    } else {
        ((ScoreMethod_Def*) pData)[pIndex] = match;
        return True;
    }
}

/** Parse build queue method into an (array of) BuildQueue_Def. For use
    with Tokenize. */
static Boolean
parseBuildQueue(const ConfigItem_Struct* pItem,
                 char* pValue, int pIndex, void* pData)
{
    int match = ListMatch(pValue, "PALs Fifo PBPs");
    (void) pItem;
    if (match < 0) {
        Error("%s: invalid build queue mode '%s'", CONFIG_FILE, pValue);
        return False;
    } else {
        ((BuildQueue_Def*) pData)[pIndex] = match;
        return True;
    }
}

/** Read String-type option. Only for `GameName', so far. Unlike for
    other options, we do not accept comments or commas in string
    options, to allow for things like `GameName = Game #3'. */
static Boolean
readString(const struct ConfigItem_Struct* pItem, char* pValue)
{
    Uns16  n = pItem->mCount - 1;
    size_t l = strlen(pValue);
    if (l > n) {
        Warning("%s: assignment to '%s' exceeds string length", CONFIG_FILE, pItem->mName);
        l = n;
    }

    memset((char*)gConfigInfo + pItem->mOffset, 0, pItem->mCount);
    memcpy((char*)gConfigInfo + pItem->mOffset, pValue, l);
    return True;
}

static Boolean
readUns32(const struct ConfigItem_Struct* pItem, char* pValue)
{
    Int32 lData[CF_MAX_ARRAY_SIZE];
    Uns32* lOption = (Uns32*) ((char*)gConfigInfo + pItem->mOffset);
    int i;

    if (!Tokenize(pItem, pValue, parseInteger, &lData, False))
        return False;
    for (i = 0; i < pItem->mCount; ++i)
        lOption[i] = lData[i];
    return True;
}

static Boolean
readUns16(const struct ConfigItem_Struct* pItem, char* pValue)
{
    Int32 lData[CF_MAX_ARRAY_SIZE];
    Int16* lOption = (Int16*) ((char*)gConfigInfo + pItem->mOffset);
    int i;

    if (!Tokenize(pItem, pValue, parseInteger, &lData, False))
        return False;
    for (i = 0; i < pItem->mCount; ++i)
        lOption[i] = lData[i];
    return True;
}

static Boolean
readInt16(const struct ConfigItem_Struct* pItem, char* pValue)
{
    Int32 lData[CF_MAX_ARRAY_SIZE];
    Int16* lOption = (Int16*) ((char*)gConfigInfo + pItem->mOffset);
    int i;

    if (!Tokenize(pItem, pValue, parseInteger, &lData, False))
        return False;
    for (i = 0; i < pItem->mCount; ++i)
        lOption[i] = lData[i];
    return True;
}

static void DoDefaultAssignment(int pOpt);

/** Assign one option. */
static Boolean
DoAssignOption(int pOptInd, char* pValue, Boolean pDefault)
{
    const ConfigItem_Struct* lItem = &sConfigDef[pOptInd];

    pValue += strspn(pValue, WHITESPACE);
    /* Check for reference. We check the second character as well to
       do the right thing in case a CFType_Cost options starts with
       '$'. No parameter name starts with a digit, so this cannot
       legally be a reference. */
    if (*pValue == '$' && (pValue[1] < '0' || pValue[1] > '9')) {
        /* It is a reference */
        char* lEnd = ++pValue;
        char* p;
        const ConfigItem_Struct* lRef;
        int   lRefIndex;

        while (*lEnd && isalnum(*lEnd))
            ++lEnd;
        p = lEnd + strspn(lEnd, WHITESPACE);
        if (*p && *p != '#') {
            Error("%s: syntax error in reference for option '%s'",
                  CONFIG_FILE, lItem->mName);
            return False;
        }
        *lEnd = 0;
        lRefIndex = lookupOptionByName(pValue);
        if (lRefIndex < 0) {
            Error("%s: unknown option name '%s'", CONFIG_FILE, pValue);
            return False;
        }
        lRef = &sConfigDef[lRefIndex];
        if (lRef->mType != lItem->mType
            || lRef->mCount < lItem->mCount
            || lRef->mMin < lItem->mMin
            || lRef->mMax > lItem->mMax)
        {
            Error("%s: option '%s' and '%s' have incompatible types",
                  CONFIG_FILE, lItem->mName, pValue);
            Error("%s:   %s : %s x %d", CONFIG_FILE, lItem->mName,
                  sConfigItemNames[lItem->mType], lItem->mCount);
            Error("%s:   %s : %s x %d", CONFIG_FILE, lRef->mName,
                  sConfigItemNames[lRef->mType], lRef->mCount);
            return False;
        }
        if (!sOptionSet[lRefIndex]) {
            if (pDefault) {
                /* we are assigning defaults, and the "target option" was
                   not set. */
                DoDefaultAssignment(lRefIndex);
            } else {
                Error("%s: option '%s' was not yet set", CONFIG_FILE, pValue);
                return False;
            }
        }
        memcpy((char*) gConfigInfo + lItem->mOffset,
               (char*) gConfigInfo + lRef->mOffset,
               lItem->mCount * sConfigItemSize[lItem->mType]);
        return True;
    }

    switch(lItem->mType) {
     case CFType_Uns16:
        return readUns16(lItem, pValue);
     case CFType_Int16:
        return readInt16(lItem, pValue);
     case CFType_Uns32:
        return readUns32(lItem, pValue);
     case CFType_Boolean:
        return Tokenize(lItem, pValue, parseBoolean,
                        (char*) gConfigInfo + lItem->mOffset, False);
     case CFType_Language_Def:
        return Tokenize(lItem, pValue, parseLanguage,
                        (char*) gConfigInfo + lItem->mOffset, False);
     case CFType_ScoreMethod_Def:
        return Tokenize(lItem, pValue, parseScoreMethod,
                        (char*) gConfigInfo + lItem->mOffset, False);
     case CFType_String:
        return readString(lItem, pValue);
     case CFType_BuildQueue_Def:
        return Tokenize(lItem, pValue, parseBuildQueue,
                        (char*) gConfigInfo + lItem->mOffset, False);
     case CFType_Tristate:
        return Tokenize(lItem, pValue, parseTristate,
                        (char*) gConfigInfo + lItem->mOffset, False);
     case CFType_Cost:
        return Tokenize(lItem, pValue, parseCost,
                        (char*) gConfigInfo + lItem->mOffset, True);
     case CFType_MAX:
        return False;
    }
    return False;
}

static void
DoDefaultAssignment(int pOpt)
{
    char defstr[1024];

    strcpy(defstr, sConfigDef[pOpt].mDefault);
    DoAssignOption(pOpt, defstr, True);
    sOptionSet[pOpt] = 1;

    /* No warnings! Leave that to PHost.
       For reference, PHost's conditions are:
       - no warning for obsolete
       - no warning for optional settings if ConfigLevel=0
         - experience options count as optional if NumExperienceLevels=0
       - no warning for incompatible options if AllowIncompatibleConfiguration=0
       - no warning for conditional settings if their controlling option
         has the other value */
}

/** Assign default values to all config options that were not assigned
    by the user. */
static void
DoDefaultAssignments(void)
{
    int i;
    for (i = 0; i < CF_NumItems; ++i) {
        if (! sOptionSet[i])
            DoDefaultAssignment(i);
    }
}

/** Assign one config option. For use with ConfigFileReader. */
static Boolean
DoAssignment(const char *name, char *val, const char *pInputLine)
{
    int lOptInd;

    (void) pInputLine;

    if (!name)
        return True;

    lOptInd = lookupOptionByName(name);
    if (lOptInd < 0)
        return False;           /* unknown option */

    if (sOptionSet[lOptInd])
        return False;           /* option being assigned twice */

    if (!DoAssignOption(lOptInd, val, False))
        return False;
    sOptionSet[lOptInd] = 1;
    return True;
}

/** Set all options to "unset". */
static void
ClearConfig(void)
{
    int i;
    for (i = 0; i < CF_NumItems; ++i)
        sOptionSet[i] = 0;
}

/**************************** HConfig Reading ****************************/

typedef struct {
    CFType   mType;             /*!< type (CFType_Boolean, CFType_Uns16, CFType_Uns32) */
    short    mCount;            /*!< number of items  */
    unsigned mHOffset;          /*!< offset into HCONFIG */
    unsigned mPIndex;           /*!< number of Pconfig_Struct member (CFI_xxx) */
} Hconfig_Import_Struct;

static Hconfig_Import_Struct gHconfigImport[] = {
#define TCDefine(type, count, offset, name)  { CFType_ ## type, count, offset, CF_ ## name },
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
  size_t i;
  int    j;

  lConfigFile = OpenInputFile(HCONFIG_FILE, GAME_DIR_ONLY | NO_MISSING_ERROR);
  if (lConfigFile == NULL)
    return IO_FAILURE;

  gCurrentConfigFile = HCONFIG_FILE;

  ClearConfig();
/*  DoDefaultAssignments();*/

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
          sprintf(lPtr, "%lu", (unsigned long) ReadDOSUns32(lData));
          lData += 4;
          break;
       case CFType_Uns16:
          sprintf(lPtr, "%u", (unsigned int) ReadDOSUns16(lData));
          lData += 2;
          break;
       default:
          passert(0);       /* can't happen */
      }
      lPtr += strlen(lPtr);
    }

    /* lPtr now is a complete assignment string. */
    if (!DoAssignOption(lItem->mPIndex, lValue, False)) {
      Warning("HCONFIG.HST value for `%s' not accepted",
              sConfigDef[lItem->mPIndex].mName);
      lRes = IO_FAILURE;
    }
   next_item:;
  }
  DoDefaultAssignments();
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


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
 *   Module:      Miscellaneous Utility Routines                  *
 *   Project:     Portable Host                                   *
 *   File Name:   vgaputil.c                                      *
 *   Programmer:  Andrew Sterian                                  *
 *   Create Date: October 18, 1994                                *
 *   Description: This module contains miscellaneous functions    *
 *                used throughout other modules.                  *
 *   Functions:   OpenInputFile                                   *
 *                OpenOutputFile                                  *
 *                MemAlloc                                        *
 *                MemCalloc                                       *
 *                MemFree                                         *
 *                MemMinAvailable                                 *
 *                Error                                           *
 *                ErrorExit                                       *
 *                Warning                                         *
 *                Info                                            *
 *                RandomRange                                     *
 *                RandomReal                                      *
 *                                                                *
 *   Change Record:                                               *
 *                                                                *
 *                                                                *
 ******************************************************************/

#define gMuteOperation 0
#define gQuietOperation 0

#include <ctype.h>
#include "phostpdk.h"
#include "private.h"

#ifdef CBUILDER
#include <string.h>
#endif

/*
 *     F I L E     I / O
 */

/* This routine opens a file handle to a file in input mode. The file name
   provided is without directory information (e.g., "pdata.hst"). The routine
   automatically looks for the file in either the game directory, the root
   directory, or both, depending upon the pLocation parameter. The filename
   is first tried as-is and then again in uppercase if not found. If:
       pLocation is GAME_DIR_ONLY:
                The file is searched in the gGameDirectory directory only
                and an error generated if not found

       pLocation is ROOT_DIR_ONLY:
                The file is searched in the gRootDirectory directory only
                and an error generated if not found

       pLocation is GAME_OR_ROOT_DIR:
                The file is first sought in the gGameDirectory directory,
                then in the gRootDirectory if not found, and if still not
                found an error is generated.

   The pLocation parameter may have the following optional flags OR'ed
   in:
        NO_MISSING_ERROR    -- do not generate an error message if the file
                               cannot be found

        TEXT_MODE           -- open the file in text mode instead of binary
*/

FILE *
OpenInputFile(const char *pName, int pLocation)
{
  FILE *lFile = 0;
  char lFullname[_MAX_PATH + 1];
  char lFilename[32];
  const char *lMode = (pLocation & TEXT_MODE) ? "rt" : "rb";

  passert(pName NEQ NULL);
  passert(pLocation & (ROOT_DIR_ONLY | GAME_DIR_ONLY | GAME_OR_ROOT_DIR));

  if (pLocation & (GAME_DIR_ONLY | GAME_OR_ROOT_DIR)) {
#if defined(__MSDOS__) || defined(__WIN32__)
    sprintf(lFullname, "%s\\%s", gGameDirectory, pName);
#else
    sprintf(lFullname, "%s/%s", gGameDirectory, pName);
#endif
    lFile = fopen(lFullname, lMode);
    if (lFile NEQ NULL)
      return lFile;

#ifndef __MSDOS__
    /* Try uppercase */
    passert(strlen(pName) < 32);
    strcpy(lFilename, pName);
#if defined(__MSDOS__) || defined(__WIN32__)
    sprintf(lFullname, "%s\\%s", gGameDirectory, strupr(lFilename));
#else
    sprintf(lFullname, "%s/%s", gGameDirectory, strupr(lFilename));
#endif
    lFile = fopen(lFullname, lMode);
    if (lFile NEQ NULL)
      return lFile;
#endif
  }

  if (pLocation & (ROOT_DIR_ONLY | GAME_OR_ROOT_DIR)) {
#if defined(__MSDOS__) || defined(__WIN32__)
    sprintf(lFullname, "%s\\%s", gRootDirectory, pName);
#else
    sprintf(lFullname, "%s/%s", gRootDirectory, pName);
#endif
    lFile = fopen(lFullname, lMode);
    if (lFile NEQ NULL)
      return lFile;

#ifndef __MSDOS__
    /* Try uppercase */
    passert(strlen(pName) < 32);
    strcpy(lFilename, pName);
#if defined(__MSDOS__) || defined(__WIN32__)
    sprintf(lFullname, "%s\\%s", gRootDirectory, strupr(lFilename));
#else
    sprintf(lFullname, "%s/%s", gRootDirectory, strupr(lFilename));
#endif
    lFile = fopen(lFullname, lMode);
    if (lFile NEQ NULL)
      return lFile;
#endif
  }

  if (lFile EQ NULL) {
    if (!(pLocation & NO_MISSING_ERROR))
      ErrorExit("unable to open '%s' for reading", lFullname);
  }
  return lFile;
}

/* This routine opens a file handle to an output file (in binary mode). The
   file name provided is without directory information (e.g., "mdata.hst").
   The routine automatically tries to create the file in either the game
   directory or the root directory, depending upon the pLocation parameter:
        pLocation is GAME_DIR_ONLY:
                    The file is created in the gGameDirectory directory

        pLocation is ROOT_DIR_ONLY:
                    The file is created in the gRootDirectory directory

        pLocation is GAME_OR_ROOT_DIR:   NOT ALLOWED

   An error is generated if the file cannot be created.
*/

FILE *
OpenOutputFile(const char *pName, int pLocation)
{
  char lFullname[_MAX_PATH + 1];
  FILE *lFile;
  char lMode[3];

  passert(pName NEQ NULL);
  passert(pLocation & (ROOT_DIR_ONLY | GAME_DIR_ONLY));
  passert(!(pLocation & GAME_OR_ROOT_DIR));

  lMode[0] = (pLocation & APPEND_MODE) ? 'a' : 'w';
  lMode[1] = (pLocation & TEXT_MODE) ? 't' : 'b';
  lMode[2] = 0;

  sprintf(lFullname, "%s/%s",
        (pLocation & GAME_DIR_ONLY ? gGameDirectory : gRootDirectory), pName);

  lFile = fopen(lFullname, lMode);
  if (lFile EQ NULL) {
    ErrorExit("unable to open '%s' for writing: %s", lFullname,
          strerror(errno));
  }
  return lFile;
}

FILE *
OpenUpdateFile(const char *pName, int pLocation)
{
  char lFullname[_MAX_PATH + 1];
  FILE *lFile;
  char lMode[4];

  passert(pName NEQ NULL);
  passert(pLocation & (ROOT_DIR_ONLY | GAME_DIR_ONLY));
  passert(!(pLocation & GAME_OR_ROOT_DIR));

  lMode[0] = (pLocation & REWRITE_MODE) ? 'r' : 'w';
  lMode[1] = '+';
  lMode[2] = (pLocation & TEXT_MODE) ? 't' : 'b';
  lMode[3] = 0;

  sprintf(lFullname, "%s/%s",
        (pLocation & GAME_DIR_ONLY ? gGameDirectory : gRootDirectory), pName);

  lFile = fopen(lFullname, lMode);
  if (lFile EQ NULL AND ! (pLocation & NO_MISSING_ERROR)) {
    ErrorExit("unable to open '%s' for update: %s", lFullname,
          strerror(errno));
  }
  return lFile;
}

Boolean
RemoveGameFile(const char *pName)
{
  char lFullname[_MAX_PATH + 1];
  char lFilename[32];

  passert(pName NEQ NULL);
  sprintf(lFullname, "%s/%s", gGameDirectory, pName);
  if (remove(lFullname) EQ 0)
    return True;

#ifndef __MSDOS__
  /* Try uppercase */

  passert(strlen(pName) < 32);
  strcpy(lFilename, pName);
  sprintf(lFullname, "%s/%s", gGameDirectory, strupr(lFilename));
  if (remove(lFullname) EQ 0)
    return True;
#endif

  return False;
}

Boolean
CopyFileToFile(FILE * pSrc, FILE * pDst, Uns32 pSize)
{
  char lBuffer[512];
  size_t lToWrite;

  passert(pSrc NEQ NULL);
  passert(pDst NEQ NULL);

  while (pSize > 0) {
    lToWrite = (size_t) min(pSize, 512UL);
    if (lToWrite NEQ fread(lBuffer, 1, lToWrite, pSrc))
      goto bad_read;
    if (lToWrite NEQ fwrite(lBuffer, 1, lToWrite, pDst))
      goto bad_write;

    pSize -= lToWrite;
  }

  return True;

bad_read:
  Error("Read error in file copy");
  return False;
bad_write:
  Error("Write error in file copy");
  return False;
}

Boolean
CopyGameFile(const char *pSrcName, const char *pDstName)
{
  Uns32 lLength;
  FILE *pInFile,
   *pOutFile;
  Boolean lRetVal;

  passert(pSrcName NEQ NULL);
  passert(pDstName NEQ NULL);

  pInFile = OpenInputFile(pSrcName, GAME_DIR_ONLY);
  pOutFile = OpenOutputFile(pDstName, GAME_DIR_ONLY);

  lLength = FileLength(pInFile);

  lRetVal = CopyFileToFile(pInFile, pOutFile, lLength);

  fclose(pInFile);
  fclose(pOutFile);

  return lRetVal;
}

Uns32
FileLength(FILE * lFile)
{
  Uns32 lSize;

  fseek(lFile, 0L, SEEK_END);
  lSize = (Uns32) ftell(lFile);
  rewind(lFile);

  return lSize;
}

Boolean
RenameGameFile(const char *pSrcName, const char *pDstName)
{
  char lFullname1[_MAX_PATH + 1],
    lFullname2[_MAX_PATH + 1];

  passert(pSrcName NEQ NULL AND pDstName NEQ NULL);
  sprintf(lFullname1, "%s/%s", gGameDirectory, pSrcName);
  sprintf(lFullname2, "%s/%s", gGameDirectory, pDstName);
  return (Boolean) (rename(lFullname1, lFullname2) EQ 0);
}

/*
 *   M E M O R Y    A L L O C A T I O N
 */

#ifdef __MSDOS__
extern C_LINKAGE void *farcalloc(Uns32 pUnits, Uns32 pSize);
extern C_LINKAGE Uns32 farcoreleft(void);
extern C_LINKAGE void farfree(void *block);
extern C_LINKAGE void *farmalloc(Uns32 pBytes);
extern C_LINKAGE void *farrealloc(void *pPtr, Uns32 pBytes);

#ifdef MICROSQUISH
#define farcalloc calloc
#define farmalloc malloc
#define farrealloc realloc
#define farfree   free
#endif
#else
#ifndef farmalloc
#define farmalloc  malloc
#define farcalloc  calloc
#define farrealloc realloc
#define farfree    free
#endif
#endif

static Boolean gAllocFailed = False;

static void
quickExit(void)
{
  fprintf(stderr, "***\n*** Out of memory\n***\n");
  exit(1);
}

void *
MemAlloc(size_t pAmount)
{
  void *lPtr;

  if (gAllocFailed)
    quickExit();

#ifdef SUBALLOC

/*    fprintf(stderr,"Allocating %u bytes...\n",pAmount); */
  lPtr = suballoc_farmalloc(pAmount);
  if (lPtr EQ NULL)
    CheckMemory();

  minMemory = min(minMemory, suballoc_memfree());
#else
  lPtr = farmalloc(pAmount);
#endif
  if (lPtr EQ NULL) {
    gAllocFailed = True;
    ErrorExit("out of memory");
  }
  return lPtr;
}

void *
MemRealloc(void *pPtr, size_t pAmount)
{
  void *lPtr;

  if (gAllocFailed)
    quickExit();

  passert(pPtr NEQ 0);
  passert(pAmount NEQ 0);
#ifdef SUBALLOC
  lPtr = suballoc_farrealloc(pPtr, pAmount);
  if (lPtr EQ NULL)
    CheckMemory();

  minMemory = min(minMemory, suballoc_memfree());
#else
  lPtr = farrealloc(pPtr, pAmount);
#endif
  if (lPtr EQ NULL) {
    gAllocFailed = True;
    ErrorExit("out of memory");
  }
  return lPtr;
}

void *
MemCalloc(size_t pNumber, size_t pSize)
{
  void *lPtr;

  if (gAllocFailed)
    quickExit();

#ifdef SUBALLOC

/*    fprintf(stderr,"Allocating %u X %u bytes...\n", pNumber, pSize); */
  lPtr = suballoc_farcalloc(pNumber, pSize);
  if (lPtr EQ NULL)
    CheckMemory();
  minMemory = min(minMemory, suballoc_memfree());
#else
  lPtr = farcalloc(pNumber, pSize);
#endif
  if (lPtr EQ NULL) {
    gAllocFailed = True;
    ErrorExit("out of memory");
  }
  return lPtr;
}

void
MemFree(void *pPtr)
{
#ifdef SUBALLOC
  if (pPtr) {
    if (!suballoc_farfree(pPtr)) {
      CheckMemory();
      ErrorExit("MemFree of pointer failed");
    }
  }
#else
  if (pPtr)
    farfree(pPtr);
#endif
}

/*
 *    E R R O R / D I A G N O S T I C    H A N D L I N G
 */

static void
reportMessage(const char *pHeader, const char *pStr, va_list pAP,
      Boolean pMute)
{
  FILE *lFiles[2];
  Uns16 i;
  char *lPtr1,
   *lPtr2;
  static char *lStr = 0;

  lFiles[0] = gLogFile;
  lFiles[1] = pMute ? 0 : stdout;

  if (lStr EQ 0) {
    lStr = (char *) MemCalloc(1024, 1);
    if (lStr EQ 0)
      quickExit();
  }
  vsprintf(lStr, pStr, pAP);

  for (i = 0; i < (sizeof(lFiles) / sizeof(lFiles[0])); i++) {
    if (lFiles[i] NEQ 0) {
      fprintf(lFiles[i], "%s", pHeader);

      for (lPtr1 = lStr, lPtr2 = strchr(lStr, 0x0D);;
            lPtr1 = lPtr2 + 1, lPtr2 = strchr(lPtr1, 0x0D)) {

        if (lPtr2)
          *lPtr2 = 0;
        fprintf(lFiles[i], "%s\n", lPtr1);
        if (lPtr2)
          *lPtr2 = 0x0D;
        else
          break;
      }
    }
  }
#ifdef PVCR
  if (!pMute) {
    VCRKeyWait();
  }
#endif
}

/* Report an error. Errors are written to stderr and to the log file
   unless the Mute Operation command-line option is given. */
void
Error(const char *pStr, ...)
{
  va_list lAP;

  va_start(lAP, pStr);

  reportMessage("Error: ", pStr, lAP, gMuteOperation);
  va_end(lAP);
}

/* Same as Error() but with a va_list argument */
void
VError(const char *pStr, va_list pAP)
{
  reportMessage("Error: ", pStr, pAP, gMuteOperation);
}

/* Report an error and exit the program with a (fixed) error code. Behavior
   is the same as Error() except that instead of returning from this routine,
   the program exits. */
void
ErrorExit(const char *pStr, ...)
{
  va_list lAP;

  va_start(lAP, pStr);

  VErrorExit(pStr, lAP);
}

/* Same as ErrorExit but with a va_list parameter */
void
VErrorExit(const char *pStr, va_list pAP)
{
  reportMessage("Error: ", pStr, pAP, gMuteOperation);

  if (gLogFile NEQ 0)
    fflush(gLogFile);

  exit(-1);
}

/* Report a warning. Warnings behave the same as errors (written to stderr
   and the log file unless Mute Operation is in effect). */
void
Warning(const char *pStr, ...)
{
  va_list lAP;

  va_start(lAP, pStr);

  reportMessage("Warning: ", pStr, lAP, gMuteOperation);
  va_end(lAP);
}

/* Same as Warning() but with a va_list parameter */
void
VWarning(const char *pStr, va_list pAP)
{
  reportMessage("Warning: ", pStr, pAP, gMuteOperation);
}

/* Print info during the progress of the host. Info messages are written
   to stderr and the log file unless the Quiet Operation command-line switch
   is given (also implied by Mute Operation). */
void
Info(const char *pStr, ...)
{
  va_list lAP;

  va_start(lAP, pStr);

  reportMessage("", pStr, lAP, gQuietOperation);
  va_end(lAP);
}

/* Same as Info() but with a va_list parameter */
void
VInfo(const char *pStr, va_list pAP)
{
  reportMessage("", pStr, pAP, gQuietOperation);
}

void
AssertFail(const char *pExpr, const char *pFile, int pLine)
{
  ErrorExit("Assertion failed! File: %s  Line: %d\n", pFile, pLine);
  (void) pExpr;
}

/*
 *    R A N D O M    N U M B E R    G E N E R A T I O N
 */

/* This routine returns a raw random number in the range [0,65536). This
   is from Knuth 3.6. We include it here so that we do not have to
   worry about different random number implementations on different
   machines. This is important for PVCR. A random seed is needed to
   initialize the combat and we must ensure that from the same seed,
   the same pseudorandom sequence ensues. The only way to do this is
   to generate the sequence ourselves. This routine replaces rand(). */

static Uns16 RandRawSeed = 0x1211; /* why not -- it gets changed anyways */

#pragma warn -sig
static Uns16
RandRaw(void)
{
  RandRawSeed = (RandRawSeed * 65277U + 13489U) & 0xFFFFU;
  return RandRawSeed;
}

#pragma warn .sig

/* This routine replaces srand() */
void
SetRandomSeed(Uns16 seed)
{
  RandRawSeed = seed;
}

/* This is a random number generator based upon the simple rand() library
 * function which doesn't usually have stellar random properties but for
 * a game it's good enough. We do, however, take a small stab towards improving
 * randomness by not simply returning (rand() % range) since this returns
 * the lowest-order bits of the random number which (for linear modulo
 * random generators) is not very random. In fact, the k LSB bits are
 * periodic with period 2^k. We use the upper bits instead. Thanks to
 * Colin Plumb for this code. Unfortunately, he screwed up his theory
 * and cost me hours looking for a bug because this code actually used
 * to return values greater than pRange. The test for equality in the
 * 'while' statement was changed to a >= test to ensure this doesn't
 * happen.
 *
 * The return value is uniformly distributed in the range [0,pRange).
 */
#pragma warn -ccc               /* Condition is always true OFF */
#pragma warn -sig               /* Significant digit loss OFF */
RandType_Def
RandomRange(RandType_Def pRange)
{
  RandType_Def lResult;
  RandType_Def lDiv;

  passert((pRange > 0) AND(pRange <= RAND_MAX));

  lDiv = ((RandType_Def) RAND_MAX) / pRange;
  while ((lResult = (RandType_Def) (RandRaw() / lDiv)) >= pRange)
    /* retry */ ;

  return lResult;
}

#pragma warn .ccc
#pragma warn .sig

/* This routine returns a floating point random number
   uniformly distributed over [0,1). */

#ifdef MICROSQUISH
double
RandomReal(void)
{
  SquishError("RandomReal");
}
#else
double
RandomReal(void)
{
  return (double) RandRaw() / ((Uns32) RAND_MAX + 1UL);
}
#endif

void
SquishError(const char *str)
{
  ErrorExit("INTERNAL PROGRAM ERROR:\n" "Your call to the PDK function:\n"
        "     %s\n" "is not supported in this PDK distribution.", str);
}

/*
 *          S T R I N G    P R O C E S S I N G
 */

char *
SkipWS(const char *p)
{
  while (*p AND isspace(*p))
    p++;

  if (*p EQ 0)
    return 0;
  return (char *) p;
}

void
TrimTrailingWS(char *p)
{
  char *q = p + strlen(p);

  while (--q >= p) {
    if (isspace(*q)) {
      *q = 0;
    }
    else
      return;
  }
}

void
ZeroTrailingWS(char *pStr, Uns16 pLen)
{
  pStr += pLen;
  do {
    *pStr-- = 0;
  } while (pLen-- > 0 AND * pStr EQ ' ');
}

/*************************************************************
  $HISTORY:$
**************************************************************/

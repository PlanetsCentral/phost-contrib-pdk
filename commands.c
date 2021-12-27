/**
  *  \file commands.c
  *
  *  Reading text commands to add-ons. Players can send commands to
  *  PHost and to add-ons. We provide numerous ways to access this
  *  communication channel easily, to encourage use of commands over
  *  use of special friendly codes to do things.
  *
  *  - players can send `x' command messages containing the command.
  *    PHost will write these to `xterncmd.ext'.
  *  - add-ons can support private files that contain the commands
  *    intended for them. For example, people can give commands to mfq
  *    with `mfqN.txt' files.
  *
  *  Unfortunately, `x' makes it unclear what command belongs to which
  *  add-on, so syntax errors can't be easily detected. Hence, we
  *  provide a convention of directing commands to an add-on:
  *
  *  - `foo: cmd' is a command directed to add-on `foo'. So if we are
  *    `foo', we know that this command is for us and can reject it if
  *    it is bad. This is the preferred form (future PHost versions will
  *    even allow players to omit the `x' in this case).
  *  - `foo cmd' is a command directed to `foo', too.
  *  - `cmd' is a command to everyone (i.e. everyone who understands
  *    it will handle it). When we don't know the command, we can't
  *    reject it, though, because there might be someone else who knows it.
  *
  *  All files in this distribution are Copyright (C) 1995-2000 by the
  *  program authors: Andrew Sterian, Thomas Voigt, and Steffen
  *  Pietsch. This file Copyright (C) 2002 Stefan Reuther. You can
  *  reach the PHOST team via email (support@phost.de).
  *
  *  This program is free software; you can redistribute it and/or
  *  modify it under the terms of the GNU General Public License as
  *  published by the Free Software Foundation; either version 2 of
  *  the License, or (at your option) any later version.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  *  General Public License for more details.
  *
  *  You should have received a copy of the GNU General Public License
  *  along with this program; if not, write to the Free Software
  *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
  */
#include <stdio.h>
#include <stdlib.h>
#include "phostpdk.h"
#include "private.h"

#define GETLINE_BUFFER 1024

/** Read a line from file. Allocates sufficient memory with MemAlloc
    (i.e. must be MemFree'd).
    \returns line or NULL */
char*
GetLine(FILE* pFile)
{
  char lBuffer[GETLINE_BUFFER];
  char* lResult = 0;
  size_t lLength = 0;

  while (fgets(lBuffer, sizeof lBuffer, pFile)) {
    size_t len = strlen(lBuffer);
    lResult = MemRealloc(lResult, lLength + len + 1);
    memcpy(lResult + lLength, lBuffer, len+1);
    lLength += len;
    if (!lLength)
      break;
    if (lResult[lLength-1] == '\n') {
      lResult[lLength-1] = 0;
      break;
    }
  }
  return lResult;
}

/** Like strtok, but
    - thread-safe
    - doesn't modify input text
    - returns pointer to malloced storage, or 0
    - updates *pString to point to just-after-the-token */
char*
GetToken(const char** pString, const char* pDelim)
{
  size_t lLen;
  
  *pString += strspn(*pString, pDelim);
  lLen = strcspn(*pString, pDelim);
  if (!lLen) {
    return 0;
  } else {
    char* lRV = MemAlloc(lLen + 1);
    memcpy(lRV, *pString, lLen);
    lRV[lLen] = 0;
    *pString += lLen;
    return lRV;
  }
}

/** Simple Command Reader.
    \param pRace      which race's commands we're interested in; 0 to
                      get them all.
    \param pFunc      function to call for each command.
    \param pComplain  function to call on errors.
    \param pProgName  our program name, for filtering.
    \param pPrivateFile name of a file private to this add-on, in which
                      to look for commands, too. Must be a sprintf template
                      for exactly one integer argument; expansion must
                      yield a plain file name (< 100 chars :-). NULL if you
                      don't want to support this feature.
    \param pData      value to pass to pFunc/pComplain */
void
CommandFileReader(Uns16 pRace, CommandReader_Func pFunc,
                  CommandComplain_Func pComplain, const char* pProgName,
                  const char* pPrivateFile, void* pData)
{
  FILE* lFile;
  char lFilenameBuffer[100];

  if (!pProgName)
    pProgName = "";

  /* xterncmd.txt */
  lFile = OpenInputFile("xterncmd.ext", TEXT_MODE | GAME_DIR_ONLY | NO_MISSING_ERROR);
  if (lFile) {
    CommandFileReaderFP(pRace, 0, pFunc, pComplain, pProgName, lFile, pData);
    fclose(lFile);
  }

  /* private files */
  if (pPrivateFile) {
    if (pRace) {
      sprintf(lFilenameBuffer, pPrivateFile, pRace);
      if ((lFile = OpenInputFile(lFilenameBuffer, TEXT_MODE | GAME_DIR_ONLY | NO_MISSING_ERROR))) {
        CommandFileReaderFP(pRace, pRace, pFunc, pComplain, 0, lFile, pData);
        fclose(lFile);
      }
    } else {
      int lRace;
      for (lRace = 1; lRace <= OLD_RACE_NR; ++lRace) {
        sprintf(lFilenameBuffer, pPrivateFile, lRace);
        if ((lFile = OpenInputFile(lFilenameBuffer, TEXT_MODE | GAME_DIR_ONLY | NO_MISSING_ERROR))) {
          CommandFileReaderFP(0, lRace, pFunc, pComplain, 0, lFile, pData);
          fclose(lFile);
        }
      }
    }
  }
}

/** Complete Command Reader.
    \param pRace      which race's commands we're interested in. May
                      be 0 for all races.
    \param pFileIsFor type of file we're reading. Zero if "xterncmd.ext",
                      that is, all commands are prefixed by their race
                      number; nonzero if this file contains only commands
                      from that race (without prefix).
    \param pFunc      command handler.
    \param pComplain  complain function. NULL means ComplainWithWarningMessage
    \param pProgName  our program name, for filtering.
                      - if NULL, we're certain the file is for us (we get
                        all commands, commands we don't understand are errors)
                      - if "", we get all commands but not errors
                      - if "foo", we get commands directed to add-on "foo"
                        or to everyone.
    \param pFile      file to read
    \param pData      pData parameter to pass to pFunc/pComplain. */
void
CommandFileReaderFP(Uns16 pRace, Uns16 pFileIsFor, CommandReader_Func pFunc,
                    CommandComplain_Func pComplain, const char* pProgName,
                    FILE* pFile, void* pData)
{
  char   *lLine;
  const char *lPtr;
  char   *lPtr2, *lPtr3;
  Boolean lFirst = True;
  char    lTime[19];
  int     lTurn;
  Uns16   lLineIsFor;
  Boolean lLineHadColon;
  
  if (!pComplain)
    pComplain = ComplainWithWarningMessage;
  while ((lLine = GetLine(pFile)) != 0) {
    if (lFirst && !pFileIsFor
        && (sscanf(lLine, "%d %18[:0-9-]", &lTurn, lTime) == 2
            || (lLine[strspn(lLine, " 0")] == '\0')))
    {
      /* looks like a timestamp. We can't test it, though, because all
         current PHost versions handle `lastturn.hst' wrong.
         As a special case, on the first turn, the file starts with a line "0 " with no timestamp. */
    } else {
      /* might be a command line */
      lPtr = lLine + strspn(lLine, " \t\r");
      if (*lPtr == '#' || *lPtr == 0)
        goto next_line;

      if (pFileIsFor) {
        lLineIsFor = pFileIsFor;
      } else {
        /* file is "xterncmd.txt", so each line contains a player number */
        long lPlayer = strtol(lPtr, &lPtr2, 10);
        if (lPlayer > 0 && lPlayer <= OLD_RACE_NR && *lPtr2 == ':') {
          lLineIsFor = lPlayer;
          lPtr = lPtr2 + 1;
          lPtr += strspn(lLine, " \t\r");
        } else {
          /* line is invalid */
          pComplain (0, lLine, "Syntax error", pData);
          goto next_line;
        }
      }

      /* do we want this command? */
      if (pRace && lLineIsFor != pRace)
        goto next_line;

      /* Now, lPtr points at the actual command and lLineIsFor says
         who sent it. */
      if (*lPtr == 0 || *lPtr == '#')
        goto next_line;
      lPtr2 = GetToken(&lPtr, " \t\r");
      if (lPtr2) {
        /* four cases:
             pProgName == 0            -> we know it's for us, no "preprocessing"
             lPtr2 == pProgName        -> we know it's for us
             lPtr2 == pProgName + ':'  -> we know it's for us
             lPtr2 == whatever + ':'   -> we know it's not for us
             otherwise                 -> might be, might be not */
        if (!pProgName) {
          /* case 0 */
          if (!pFunc(lLineIsFor, lPtr2, lPtr + strspn(lPtr, " \t\r"), lLine, pData))
            pComplain(lLineIsFor, lLine, "Command not understood", pData);
        } else {
          lPtr3 = lPtr2 + strlen(lPtr2) - 1;
          if (*lPtr3 == ':') {
            lLineHadColon = True;
            *lPtr3 = 0;
          } else {
            lLineHadColon = False;
          }
          if (*pProgName && stricmp(pProgName, lPtr2) == 0) {
            /* case 1 or 2 */
            MemFree(lPtr2);
            lPtr2 = GetToken(&lPtr, " \t\r");
            if (lPtr2)
              if (!pFunc(lLineIsFor, lPtr2, lPtr + strspn(lPtr, " \t\r"), lLine, pData))
                pComplain(lLineIsFor, lLine, "Command not understood", pData);
          } else if (*pProgName && lLineHadColon) {
            /* case 3 */
          } else {
            pFunc(lLineIsFor, lPtr2, lPtr + strspn(lPtr, " \t\r"), lLine, pData);
            /* case 4 */
          }
        }
        if (lPtr2)
          MemFree(lPtr2);
      }
    }
  next_line:
    lFirst = False;
    MemFree(lLine);
  }
}

/** Send a complaint by subspace message. Malformed lines get no complaint. */
void
ComplainWithSubspaceMessage(Uns16 pRace, const char* pLine, const char* pReason, void* pData)
{
  (void) pData;
  if (pRace) {
    char lBuffer[700];
    sprintf(lBuffer,
            "(-h000)<<< Command Failed >>>\015"
            "\015"
            "The command you sent:\015"
            "  %.38s\015"
            "could not be executed:\015"
            "  %.38s\015",
            pLine, pReason);
    WriteAUXHOSTMessage(pRace, lBuffer);
  }
}

/** Show warning on stdout / host.log. */
void
ComplainWithWarningMessage(Uns16 pRace, const char* pLine, const char* pReason, void* pData)
{
  (void) pData;
  if (!pRace)
    Warning("Error executing command `%s': %s", pLine, pReason);
  else
    Warning("Error executing command `%s' for player %d: %s", pLine, pRace, pReason);
}

#ifdef TEST
Boolean func (Uns16 pRace, const char* pCommand, const char* pArgs,
              const char* pWholeLine, void* pData)
{
  printf("Command for race %d:\n"
         "  Verb = `%s'\n"
         "  Args = `%s'\n"
         "  Line = `%s'\n", pRace, pCommand, pArgs, pWholeLine);
  return strlen(pCommand) % 2 == 0;
}

int main()
{
  CommandFileReader(4, func, 0, "foo", "foo%d.txt", 0);
}
#endif

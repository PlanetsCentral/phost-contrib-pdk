/*
 *   Portable Host Development Kit
 *   EXAMPLE PROGRAM #3
 *
 * This program is Miche l Keane's INVv1.2x program ported to use the
 * PDK. The documentation for Invasion! follows at the end of the file.
 *
 * This file is copyrighted by Miche l Keane in 1995.
 * You may copy and distribute this file freely as long as you retain
 * this notice and explicitly document any changes you make to this file
 * (along with your name and date) before distributing modified
 * versions.
 *
 * This is an example program for instructional purposes only. It is not
 * guaranteed to work correctly, or even to necessarily compile on all
 * systems. You use this program at your own risk.
 */

#include <stdio.h>
#include <string.h>
#include <phostpdk.h>

FILE * planfile;

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif

void crash(void);
Int16 invoptions(void);

Int16 * extra;  Int16 * expts;
Int16 exval;
char * messages[21];
char * message;

int main (Int16 args, char* cline[]) {
  Int16 pts[12] = {0};
  Int16 newpts[12] = {0};
  Int16 hws[12] = {0};
  Int16 list[11] = {0};
  Int16 x, y, z, hwval, listtype, argst;
  char string[20];
  char filename[256] = {'\0'};
  char fill;
  Int16 dummy1, dummy2;

  InitPHOSTLib();

  gGameDirectory = (char *)MemAlloc(256);

#ifdef THINK_C  /* For Macintosh */
  strcpy(gGameDirectory, "");
#else
  strcpy(gGameDirectory, "./");
#endif

  hwval = 1; exval = 0; listtype = 1;
  argst = 1;

  if (args > 1) {
    if (cline[1][0] != '\\' && cline[1][0] != '/' && cline[1][0] != '-') {
      strcpy(gGameDirectory, cline[1]);
      argst++;
    }
    for (x = argst; x < args; x++) {
      if (cline[x][0] == '\\' || cline[x][0] == '-' || cline[x][0] == '/') {
        if ((x + 1) < args) {
          switch(cline[x][1]) {
            case 'h':
              hwval = atoi(cline[x + 1]);
              x++;
              break;
            case 'l':
              listtype = atoi(cline[x + 1]);
              x++;  
              break;
          }
        }
      }
    }
    if ((hwval < 0)) {
      invoptions();
      crash(); return 1;
    }
    if ((listtype < 1) || (listtype > 3)) {
      Warning("Illegal ranking type. Defaulting to 1\n");
      listtype = 1;
    }
  }

  x = strlen(gGameDirectory) - 1;

  if (gGameDirectory[x] != '\\' && gGameDirectory[x] != '/') {
#ifdef THINK_C
    strcat(gGameDirectory, ":");
#else
    strcat(gGameDirectory, "/");
#endif
  }

  gLogFile = OpenOutputFile("inv.log", TEXT_MODE | GAME_DIR_ONLY);
  if (gLogFile == NULL)  {
    Error("Invalid path\n");
    crash(); return 1;
  }

  Info("Invasion! v1.2x by Miche l Keane\n");
  Info(" An invasion scenario util for hosts\n");
  Info("%d %s %s\n", args, (args > 1 ? cline[0] : ""),
                                 (args > 2 ? cline[1] : ""));
  Info("Homeworld value = %d\n", hwval);
  Info("Ranking type = %d\n", listtype);
  Info("gGameDirectory = %s\n", gGameDirectory);
  
  if ((message = (char *)MemAlloc(1600)) == NULL) {
    Info("(message = (char *)MemAlloc(1600)) failed\n");
    crash(); return 1;
  }
  message[0] = '\0';

  for (x = 0; x < 21; x++) {
    if ((messages[x] = (char *)MemAlloc(40)) == NULL) {
      Info("(messages[%i] = (char *)MemAlloc(40)) failed\n", x);
      crash(); return 1;
    }
    strcpy(messages[x], "\0");
  }  

  if (! ReadGlobalData()) { crash(); return 1; }
  if (! ReadHostData()) { crash(); return 1; }

  Info("Now opening and reading inv1.hst\n");
  planfile = OpenInputFile("inv1.hst", GAME_DIR_ONLY | TEXT_MODE);
  if (planfile != NULL) {
    for (x = 0; x < 11; x++) {
      if (feof(planfile) != 0) {
        Error("inv1.hst is too short!\n");
        crash(); return 1;
      }
      fgets(string, 20, planfile);
      sscanf(string, "%d %d", &hws[x], &pts[x]);
    }
    fclose(planfile);
  }
  else {
    fclose(planfile);
    strcpy(filename, "refdata.hst");
    Info("inv1.hst not found... scanning refdata.hst\n");
    planfile = OpenInputFile("refdata.hst", GAME_DIR_ONLY);
    if (planfile != 0) {
       fread(hws, 2, 3, planfile);
       fread(hws, 2, 11, planfile);
    }
    else {
       Info("refdata.hst not found... scanning bdata.hst\n");
       for (x = 1; x <= PLANET_NR; x++) {
         if (IsBaseExist(x)) hws[BaseOwner(x)] = x;
       }
    }
  }
  fclose(planfile);

  Info("Looking for inv2.hst...\n");
  planfile = OpenInputFile("inv2.hst", GAME_DIR_ONLY | TEXT_MODE |
   NO_MISSING_ERROR);
  if (planfile != NULL) {
    fgets(string, 20, planfile);
    sscanf(string, "%d", &exval);
    extra = (Int16 *)MemAlloc(2 * (exval + 1));
    expts = (Int16 *)MemAlloc(2 * (exval + 1));

    for (x = 0; x < exval; x++) {
      fgets(string, 20, planfile);
      sscanf(string, "%d %d", (extra + x), (expts + x));
    }
  }
  else {
    Info("Optional inv2.hst not found\n");
  }
  fclose(planfile);

  Info("Tallying homeworld points\n");
  for (x = 1; x <= 11; x++) {
    if ((hws[x] != 0) && (hws[x] < 501)) {
      if ((PlanetOwner(hws[x]) != NoOwner)
       && (PlanetOwner(hws[x]) <= RACE_NR)
       && (PlanetOwner(hws[x]) != x)) {
          newpts[PlanetOwner(hws[x])] += hwval;
      }
    }
  }

  if (exval > 0) {
    Info("Tallying extra points\n");
    for (x = 0; x < exval; x++) {
      if ((PlanetOwner(extra[x]) != NoOwner)
       && (PlanetOwner(extra[x]) <= RACE_NR)) {
        newpts[PlanetOwner(extra[x])] += expts[x];
      }
    }
  }

  Info("Now opening and writing inv1.hst\n", filename);
  planfile = OpenOutputFile("inv1.hst", GAME_DIR_ONLY | TEXT_MODE);
  for (x = 1; x <= 11; x++) {
    pts[x] += newpts[x];
    fprintf(planfile, "%i %i\n", hws[x], pts[x]);
  }
  fclose(planfile);

  Info("Creating messageto to send to players\n");
  sprintf(messages[0], "<<<<  Invasion Report!  >>>>\r");
  sprintf(messages[1], "Turn: %i\r", TurnNumber());
  sprintf(messages[2], " # Name                 Total  New\r");
  for (x = 1; x <= 11; x++) {
    sprintf(messages[x + 2], "%2d %20s  %3d  %3d\r",
     x, RaceShortName(x, 0), pts[x], newpts[x]);
  }
  for (x = 14; x < 18; x++) {
    sprintf(messages[x], "\r");
  }
  sprintf(messages[18], "Invasion! v1.2x by Miche l Keane\r");
  sprintf(messages[19], "\r");

  Info("Sorting scores\n");

  switch(listtype) {
    case 1:
      for (x = 1; x <= 11; x++) {
        list[x] = pts[x];
      }
      break;
    case 2:
      for (x = 1; x <= 11; x++) {
        list[x] = newpts[x];
      }
      break;
    case 3:
      for (x = 1; x <= 11; x++) {
        list[x] = 12 - x;
      }
      break;
  }
  for (x = 1; x <= 11; x++) {
    for (y = x; y <= 11; y++) {
      if (list[y] > list[x]) {
        strcpy(messages[20], messages[x + 2]);
        strcpy(messages[x + 2], messages[y + 2]);
        strcpy(messages[y + 2], messages[20]);
        z = list[x]; list[x] = list[y]; list[y] = z;
      }
    }
  }

  planfile = OpenOutputFile("invscore.txt", GAME_DIR_ONLY | APPEND_MODE | TEXT_MODE);
  for (x = 0; x < 20; x++) {
    Info("%s\n", messages[x]);
    fprintf(planfile, "%s\n", messages[x]);
    strcat(message, messages[x]);
    MemFree(messages[x]);
  }
  fclose(planfile);

  for (x = 1; x <= 11; x++) {
    const char *messagePtr[1];
    if (! PlayerIsActive(x)) continue;

    if (hws[x - 1] != 0) {
      Info("Sending message to player %i\n", x);
      messagePtr[0] = message;
      WriteMessageToRST(x, 1, messagePtr);
    }
  }

  if (exval != 0) {
    MemFree(extra);
    MemFree(expts);
  }
  MemFree(message);

  FreePHOSTLib();

  Info("Finished!\n");
  return 0;
}

Int16 invoptions() {
  Info("INV command line options\n");
  Info(" INV [Path] [/H ##]\n");
  Info("[Path] must be a valid directory\n");
  Info("/H ## where ## is the point value for homeworlds. Defaults to 1\n\n");

  return 0;
}

void crash(void) {
  Int16 x;
  if (exval != 0) {
    MemFree(extra);
    MemFree(expts);
  }
  MemFree(message);
  for (x = 0; x < 20; x++) {
    MemFree(messages[x]);
  }    

  FreePHOSTLib();
}


/*      DOCUMENTATION FOR INV

INV: The Invasion Scenario
v1.2x by Miche l Keane(micheal.keane@tpalley.eskimo.com)

 For those of us who think ref is just too limiting!

 Warranty:
  If you use & you lose, you pay & I don't.

 This is a relatively simple program for invasion scenarios. Basically, run
the program as follows:

INV [path] [options]

  [path] obviously

  valid options are:
    /h hwval  -- num is the number of points awarded for captured homeworlds.
                 defaults to 1.

    /l type   -- How you'd like the races ranked in the message.
                 valid values for type are:
                  1  -- rank by total points (default)
                  2  -- rank by new points this turn
                  3  -- list in order of race, 1 to 11

 Run it AFTER (P)Host. Not during the AUXHOST phases or the messages won't
 be sent. I'm sure it'll tally everything fine though.

 Files in INV12X.ZIP
  INV.EXE      -- main EXE
  INV.DOC      -- Docs you're reading right now
  INV.C        -- ANSI C source code
  ALTSPEC.H    -- VGAP file structure based largely on VPCPLAY.BAS's specs

 INV will create two files each run in the path directory:
  INVSCORE.TXT -- a copy of the message sent to players, suitable for
                  BBS bulletins and the like
  INV.LOG      -- a log file of the program's run

 files read & where they should be:
  RACE.NM     -- game or current dir (required)
  PDATA.HST   -- game dir (required)
  REFDATA.HST -- game dir (used for init if present)
  BDATA.HST   -- game dir (required for init if refdata.hst ain't there)

 Configuration:
  INV1.HST    -- main text datafile for homeworlds and total scores
  INV2.HST    -- optional text file for additional point planets

  The format of INV1.HST is:

   [homeworld] [score]

  for _ALL_ 11 lines. A sample INV1.HST:

250 5
220 24
279 26
189 117
102 125
474 34
459 90
224 67
4 13
460 21
262 103

  The format of INV2.HST is:
   The first line contains the number of ADDITIONAL point planets. Don't
   include the 11 homeworlds in this number.
   For the number of lines after that, the format goes:
   [planet] [points]

   [points] being the number of points awarded per turn when a race holds
   that planet.

   A short sample INV2.HST:

4
21 5
134 1
256 1
345 1

 Notes:
  In order to receive a status message, a player must have a homeworld
  number. What if you're not playing a "capture enemy homeworlds" scenario?
  Just set the homeworld value to 0. ie: INV [path] /h 0

History:
  1.0  -- released for use in game 11 on Lame BBS only
        - writes to seperate textfile for hostmess to use
  1.1a -- thanks to Thomas Voigt, this version writes directly to the RST
        - no longer generates text file of message
        - lock up bugs fixed somehow
        - reworking of code to speed up some things which might give a
          smaller EXE size as well
        - I've finally learned how to use strings in C! =-)
  1.1b -- at Gordy's request, INV generates the text file again
  1.1c -- INV wasn't sending messages to player 11 as well as other
          message sending problems
        - not as much stuff written to the log anymore
        - more error checking for malloc() failures added
  1.1d -- INV.CPP renamed INV.C to show ANSI compatibility
        - altspec.h included and is ANSI compatible now
        - released to the general public
       -- vicious bug discovered and quashed, didn't rename version
          but one copy did make it to an ftp site. hopefully killed.
  1.2a -- race.nm required in either the current or game directory
        - the format of the scoring message changed
          here's a short sample:
             # Name                 Total  New
            11 The Colonies           99    0
             8 The Evil Empire        57    0
             6 The Cyborg             56    0

          notice the inclusion of short racenames and the fact that the
          scores are listed in descending order
  1.2b -- new command line option of /l to make the listing method
          configurable
        - cleaned up the docs quite a bit
        - new code for the command line processing, should be more
          flexible now. ie: bad commands are ignored
  1.2x -- rewritten with PDK and I cleaned up the code quite a bit.
        - turn number written in message


Thanks to:
  Tim Wisseman for writing VGA Planets, of course.
  Thomas Voigt & Andrew Sterian for PDK package
  Gordy Pine of Lame BBS for making me write this utility =-)
  My parents for paying the phone bill

Portability? :
  With the PDK package, this program is 100% portable.

  Please Email me about this if you want to port it to another platform.
  Tell me if you're sucessful in your venture and where a copy of the
  ported version can be obtained.

other projects:
SHIPLIST v1.1 -- short program for players and hosts alike. dumps shipspecs
 into a human readable text file. simple to use! 100% ANSI C
CAPFLAG -- Capture the flag! The object is to tow the flag to your homeworld
 for 5 turns. But, the catch is that everyone else knows who has it and
 where they are. Great fun if everyone actually plays. Should be released
 very soon. written in QuickBasic, thinking of an ANSI C version.
 "When I Have The Time"(tm)
BEAMS -- my shiplist editor, probably won't be released although it IS a
 pretty decent first C++ effort. thought I'd mention it.

Project X -- may never be released for certain reasons... but you'll
 know what is it if/when it's released to the general public. in private
 testing and is ANSI C. Don't ask. =-)

*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <memory.h>
#include "phostpdk.h"
#include "private.h"
#include "pgetopt.h"

#define PMASTER_VERSION_MAJOR PHOST_VERSION_MAJOR
#define PMASTER_VERSION_MINOR PHOST_VERSION_MINOR

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#ifndef min
#define min(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) ((a) > (b) ? (a) : (b))
#endif
#define sgn(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))

/* The scenario we're playing */
typedef enum {
  Classic,
  Ashes,                        /* Ashes of the old empire */
  Disunited,                    /* Disunited Kingdoms */

  NumScenarios
} Scenario_Type;

typedef struct {
  Uns16 mHomeworlds[12];
  Uns16 mHomeworldsRadius;      /* 0 for custom homeworlds */
  Uns32 mHomeworldClans[12];
  Boolean mDesertCrystalHomeworld; /* True if Crystals like it hot */
  Uns32 mHomeworldCash[12];
  Boolean mHomeworldBases[12];
  Uns32 mHomeworldCoreN[12];
  Uns32 mHomeworldCoreT[12];
  Uns32 mHomeworldCoreD[12];
  Uns32 mHomeworldCoreM[12];
  Uns16 mHomeworldSurfaceN[12];
  Uns16 mHomeworldSurfaceT[12];
  Uns16 mHomeworldSurfaceD[12];
  Uns16 mHomeworldSurfaceM[12];
  Uns16 mBaseEngineTech[12];
  Boolean mTwoFreeShips[12];
  Scenario_Type mScenario;
  Boolean mRaces[12];           /* True if race is participating */
  char mPasswords[12][11];
  Uns16 mPlanetCoreRangesUsual[8]; /* Nmin, Tmin, Dmin, Mmin, Nmax, Tmax,
                                      Dmax, Mmax */
  Uns16 mPlanetCoreRangesAlternate[8]; /* as above */
  Uns16 mPlanetCoreUsualFrequency[4]; /* Nfreq, Tfreq, Dfreq, Mfreq */
  Uns16 mPlanetSurfaceRanges[8]; /* Nmin, Tmin, Dmin, Mmin, Nmax, Tmax, Dmax, 
                                    Mmax */
  Uns16 mPlanetDensityRanges[8]; /* Nmin, Tmin, Dmin, Mmin, Nmax, Tmax, Dmax, 
                                    Mmax */
  Uns16 mNativesOnPlanetFrequency; /* 0-100% chance of natives on a planet */
  Uns16 mNativeTypeFrequencies[9]; /* Relative ratios of native types */
  Uns16 mNativeGovFrequencies[9]; /* Relative ratios of native governments */
  Uns32 mNativeClansRange[2];   /* ClansMin, ClansMax */
  Uns16 mOldEmpireRace;         /* AotE parameter */
  Uns16 mOldEmpirePlanets;      /* AotE parameter */
  Uns16 mOwnedPlanets[12];      /* # of planets owned by each race */
  Boolean mAllOwnedPlanetsTheSame; /* all disunited planets the same */
} MConfig_Struct;

/* This is the structure of the REF.HST file. This should really be in
   hostio.c but the integration with PMASTER is so tight I'd hate to have
   to pass all those values back and forth. */
typedef struct {
  Uns16 mActive;                /* REF on or off */
  Uns16 mScenario;              /* 
                                 * 1-Classic, 
                                 * 3-Wandering, 
                                 * 6-Ashes, 
                                 * 7-Crazy, 
                                 * 8-Disunited 
                                 */
  Uns16 mHomeworlds[12];
  Uns16 mFlagPoints[12];        /* # of turns a race has controlled flag
                                   world */
  Uns16 mFlagWorld;             /* the flag world */
  Uns16 mFlagGoalPoints;        /* # of points needed to win a flag game */
  Uns16 mOldEmpireRace;         /* Race that plays the Old Empire */
  Uns16 mGameGoal;              /* 
                                 * Goal of the game:
                                 *   1 - Colonist and Natives Vote
                                 *   2 - Points Awarded for Ship Tonnage
                                 *   3 - Points Awarded for Ship Tonnage and Votes
                                 *   4 - Tantalus Machine
                                 *   5 - Moly, the Spice of Life
                                 *   6 - Ashes of the Old Empire
                                 *   7 - Invasion
                                 */
  Uns32 mPoints[12];            /* Points scored */
  Uns32 mMegaPoints[12];        /* Billions of points scored */
  Uns32 mPointsGoal;            /* Number of points needed to win */
  Uns32 mMegaPointsGoal;        /* Number of mega points needed to win */
  Uns16 mGameWinner;
} Refdata_Struct;

extern int stricmp(const char *s1, const char *s2);

static int
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

/*
 *                          C O N F I G U R A T I O N
 */

typedef enum {
  CFType_Uns8,
  CFType_Uns16,
  CFType_Int16,
  CFType_Uns32,
  CFType_Boolean,
  CFType_Float,
  CFType_String,
  CFType_List,
  CFType_NoType
} CFType;
typedef char NoType;

#define CFDefine(name, member, num, type, def) name,
static const char *Names[] = {
#include "pmaster.hi"
  0
};

#define CF_NumItems ((sizeof(Names)/sizeof(Names[0]))-1)

#define CFDefine(name, member, num, type, def) offsetof(MConfig_Struct, member),
static size_t Pos[] = {
#include "pmaster.hi"
  0
};

#define CFDefine(name, member, num, type, def) num,
static Uns16 Elements[] = {
#include "pmaster.hi"
  0
};

#define CFDefine(name, member, num, type, def) CFType_ ## type ,
static CFType Types[] = {
#include "pmaster.hi"
  CFType_NoType
};

#define CFDefine(name, member, num, type, def) def,
static const char *Defaults[] = {
#include "pmaster.hi"
  0
};

static Boolean UserSet[CF_NumItems];

static int firstTime = 1;

static char *Data = 0;

static int readUns8(Uns16 ix, char *val);
static int readUns16(Uns16 ix, char *val);
static int readInt16(Uns16 ix, char *val);
static int readUns32(Uns16 ix, char *val);
static int readBooleanType(Uns16 ix, char *val);
static int readFloatType(Uns16 ix, char *val);
static int readListType(Uns16 ix, char *val);
static int readStringType(Uns16 ix, char *val);

/* Check each assignment for a valid parameter value. Boolean-type and List-type
   assignments need not be checked since they've already been verified by
   ListMatch */

static int
checkValue(Uns16 ix, long val)
{
  static struct {
    long minval;
    long maxval;
  } sCheck[CF_NumItems] = {
    {
    1, 500},                    /* Homeworlds */
    {
    0, 1000},                   /* HomeworldsRadius */
    {
    1, 100000UL},               /* HomeworldClans */
    {
    0, 1},                      /* DesertCrystalHomeworld */
    {
    0, 0x7FFFFFFFUL},           /* HomeworldCash */
    {
    0, 1},                      /* HomeworldBase */
    {
    0, 0x7FFFFFFFUL},           /* HomeworldCoreN */
    {
    0, 0x7FFFFFFFUL},           /* HomeworldCoreT */
    {
    0, 0x7FFFFFFFUL},           /* HomeworldCoreD */
    {
    0, 0x7FFFFFFFUL},           /* HomeworldCoreM */
    {
    0, 32767},                  /* HomeworldSurfaceN */
    {
    0, 32767},                  /* HomeworldSurfaceT */
    {
    0, 32767},                  /* HomeworldSurfaceD */
    {
    0, 32767},                  /* HomeworldSurfaceM */
    {
    1, 10},                     /* StartingEngineTech */
    {
    0, 1},                      /* TwoFreeShips */
    {
    0, 0},                      /* Scenario */
    {
    0, 1},                      /* RaceIsPlaying */
    {
    0, 0},                      /* Password1 */
    {
    0, 0},                      /* Password2 */
    {
    0, 0},                      /* Password3 */
    {
    0, 0},                      /* Password4 */
    {
    0, 0},                      /* Password5 */
    {
    0, 0},                      /* Password6 */
    {
    0, 0},                      /* Password7 */
    {
    0, 0},                      /* Password8 */
    {
    0, 0},                      /* Password9 */
    {
    0, 0},                      /* Password10 */
    {
    0, 0},                      /* Password11 */
    {
    0, 32767},                  /* PlanetCoreRangesUsual */
    {
    0, 32767},                  /* PlanetCoreRangesAlternate */
    {
    0, 100},                    /* PlanetCoreUsualFrequency */
    {
    0, 32767},                  /* PlanetSurfaceRanges */
    {
    10, 100},                   /* PlanetDensityRanges */
    {
    0, 100},                    /* NativesOnPlanetFrequency */
    {
    0, 100},                    /* NativeTypeFrequencies */
    {
    0, 100},                    /* NativeGovFrequencies */
    {
    1, 160000UL},               /* NativeClansRange */
    {
    1, 11},                     /* OldEmpireRace */
    {
    1, 489},                    /* OldEmpirePlanets */
    {
    1, 45},                     /* OwnedPlanets */
    {
    0, 1}                       /* AllOwnedPlanetsTheSame */
  };
  passert(ix < CF_NumItems);
  return (val >= sCheck[ix].minval AND val <= sCheck[ix].maxval);
}

static void
doInit(void)
{
  if (!firstTime)
    return;
  firstTime = 0;

  Data = (char *) MemCalloc(sizeof(MConfig_Struct), 1);
}

static int
DoAssignment(const char *name, char *val)
{
  Uns16 ix;

  doInit();

  for (ix = 0; ix < CF_NumItems; ix++) {
    if (!stricmp(Names[ix], name))
      break;
  }
  if (ix >= CF_NumItems) {
    fprintf(stderr, "Parameter '%s' is not known\n", name);
    return 0;
  }

  if (UserSet[ix]) {
    fprintf(stderr, "Warning: Duplicate assignment for '%s'\n", Names[ix]);
  }

  UserSet[ix] = True;

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
  case CFType_Float:
    return readFloatType(ix, val);
  case CFType_String:
    return readStringType(ix, val);
  case CFType_List:
    return readListType(ix, val);
  default:
    passert(0);
  }

  return 1;
}

static void
DoDefaultAssignments(void)
{
  int ix;
  char defstr[1024];

  doInit();

  for (ix = 0; ix < CF_NumItems; ix++) {
    strcpy(defstr, Defaults[ix]);
    DoAssignment(Names[ix], defstr);
  }
  memset(UserSet, 0, sizeof(UserSet));
}

static void
PrintDefaultSets(void)
{
  int ix;

  for (ix = 0; ix < CF_NumItems; ix++) {
    if (!UserSet[ix]) {
      printf("Warning: A default value will be used for parameter '%s'\n",
            Names[ix]);
    }
  }
}

static int
getLong(char *str, long *retval)
{
  char *endptr;
  char *p;

  p = strtok(str, ", \t");
  if (p == 0) {
    fprintf(stderr, "Not enough elements in assignment\n");
    return 0;
  }

  *retval = strtol(p, &endptr, 0);
  if (*endptr NEQ 0) {
    fprintf(stderr, "Illegal numeric value '%s'\n", str);
    return 0;
  }
  return 1;
}

static int
getDouble(char *str, double *retval)
{
  char *endptr;
  char *p;

  p = strtok(str, ", \t");
  if (p == 0) {
    fprintf(stderr, "Not enough elements in assignment");
    return 0;
  }

  *retval = strtod(p, &endptr);
  if (*endptr NEQ 0) {
    fprintf(stderr, "Illegal numeric value '%s'\n", str);
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

  for (i = 0; i < n; i++) {
    if (!getLong(val, &uval))
      return 0;
    val = 0;
    if (uval < 0 OR uval > 255) {
      fprintf(stderr, "Illegal 8-bit quantity '%ld'\n", uval);
      return 0;
    }
    if (!checkValue(ix, uval))
      return 0;

    *(Uns8 *) (Data + Pos[ix] + i * sizeof(Uns8)) = (Uns8) uval;
  }
  return 1;
}

static int
readUns16(Uns16 ix, char *val)
{
  long uval;
  Uns16 n = Elements[ix];
  Uns16 i;

  for (i = 0; i < n; i++) {
    if (!getLong(val, &uval))
      return 0;
    val = 0;
    if (uval < 0 OR uval > 65535U) {
      fprintf(stderr, "Illegal 16-bit unsigned quantity '%ld'\n", uval);
      return 0;
    }
    if (!checkValue(ix, uval))
      return 0;

    *(Uns16 *) (Data + Pos[ix] + i * sizeof(Uns16)) = (Uns16) uval;
  }
  return 1;
}

static int
readInt16(Uns16 ix, char *val)
{
  long uval;
  Uns16 n = Elements[ix];
  Uns16 i;

  for (i = 0; i < n; i++) {
    if (!getLong(val, &uval))
      return 0;
    val = 0;
    if (uval < -32767 OR uval > 32767) {
      fprintf(stderr, "Illegal 16-bit signed quantity '%ld'\n", uval);
      return 0;
    }
    if (!checkValue(ix, uval))
      return 0;

    *(Int16 *) (Data + Pos[ix] + i * sizeof(Int16)) = (Int16) uval;
  }
  return 1;
}

static int
readUns32(Uns16 ix, char *val)
{
  long uval;
  Uns16 n = Elements[ix];
  Uns16 i;

  for (i = 0; i < n; i++) {
    if (!getLong(val, &uval))
      return 0;
    val = 0;

    if (!checkValue(ix, uval))
      return 0;
    *(Uns32 *) (Data + Pos[ix] + i * sizeof(Uns32)) = (Uns32) uval;
  }
  return 1;
}

static int
readBooleanType(Uns16 ix, char *val)
{
  Uns16 n = Elements[ix];
  Uns16 i;
  char *p;

  for (i = 0; i < n; i++) {
    int match;

    p = strtok(val, ", \t");
    val = 0;

    if (p == 0) {
      fprintf(stderr, "Not enough elements in assignment\n");
      return 0;
    }

    match = ListMatch(p, "False True No Yes"); /* order is important */
    if (match < 0) {
      fprintf(stderr, "Illegal boolean parameter: '%s'\n", p);
      return 0;
    }
    if (match > 1)
      match -= 2;               /* Convert no/yes to false/true */

    *(Boolean *) (Data + Pos[ix] + i * sizeof(Boolean)) = (Boolean) match;
  }
  return 1;
}

static int
readFloatType(Uns16 ix, char *val)
{
  double fval;
  Uns16 n = Elements[ix];
  Uns16 i;

  for (i = 0; i < n; i++) {
    if (!getDouble(val, &fval))
      return 0;
    val = 0;

    if (!checkValue(ix, fval))
      return 0;
    *(double *) (Data + Pos[ix] + i * sizeof(double)) = fval;
  }
  return 1;
}

static int
readListType(Uns16 ix, char *val)
{
  Uns16 n = Elements[ix];
  Uns16 i;
  char *p;

  for (i = 0; i < n; i++) {
    int match;

    if (!strcmp(val, Defaults[ix])) {
      match = 0;
    }
    else {
      p = strtok(val, ", \t");
      val = 0;

      if (p == 0) {
        fprintf(stderr, "Not enough elements in assignment\n");
        return 0;
      }

      match = ListMatch(p, Defaults[ix]);

      if (match < 0) {
        fprintf(stderr, "Illegal value: '%s'\n", p);
        return 0;
      }
    }
    *(Boolean *) (Data + Pos[ix] + i * sizeof(Boolean)) = (Boolean) match;
  }
  return 1;
}

static int
readStringType(Uns16 ix, char *val)
{
  Uns16 n = Elements[ix] - 1;

  if (strlen(val) > n) {
    fprintf(stderr, "Warning: Assignment to '%s' exceeds string length\n",
          Names[ix]);
  }
  n = min(n, strlen(val));

  memset(Data + Pos[ix], 0, Elements[ix]);
  strncpy(Data + Pos[ix], val, n);

  return 1;
}

static char *
skipWS(char *p)
{
  while (*p && isspace(*p))
    p++;

  if (*p == 0)
    return 0;
  return p;
}

static void
trimTrailingWS(char *p)
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
ReadMConfigFile(const char *lFileName)
{
  FILE *InFile = OpenInputFile(lFileName, GAME_OR_ROOT_DIR | TEXT_MODE);
  Uns16 line = 0;
  char inbuf[256];
  Boolean lInSection = True;

  while (!feof(InFile)) {
    char *p,
     *ep;

    line++;
    if (0 == fgets(inbuf, 255, InFile)) {
      if (!feof(InFile)) {
        fprintf(stderr, "Error reading input file at line %u\n", line);
        exit(1);
      }
      break;
    }

    p = skipWS(inbuf);
    if (p == 0)
      continue;

    if (*p == '#')
      continue;

    if (*p == '%') {
      char *lSection;

      lSection = strtok(p + 1, " \t\n\r");
      if (lSection EQ 0)
        continue;

      if (stricmp(lSection, "pmaster") EQ 0) {
        /* We're starting our target section */
        lInSection = True;
      }
      else {
        /* Some other section */
        lInSection = False;
      }
      continue;
    }

    if (!lInSection)
      continue;

    /* We have a name on the LHS */
    ep = strchr(p, '=');
    if (ep == 0) {
      fprintf(stderr, "Expected assignment at line %u\n", line);
      exit(1);
    }

    *ep++ = 0;
    ep = skipWS(ep);
    if (ep == 0) {
      fprintf(stderr, "Expecting value to assign at line %u\n", line);
      exit(1);
    }
    trimTrailingWS(ep);

    trimTrailingWS(p);
    if (*p == 0) {
      fprintf(stderr, "Expecting parameter name to assign to at line %u\n",
            line);
      exit(1);
    }

    if (!DoAssignment(p, ep)) {
      fprintf(stderr, "Parameter error at line %u\n", line);
      exit(1);
    }
  }

  PrintDefaultSets();
}

/******************************************************************************
 *                                                                            *
 *                              M A S T E R I N G                             *
 *                                                                            *
 ******************************************************************************/

#define cfg ((MConfig_Struct *)Data)

static void doPlanetRandomize(void);
static void doHomeworlds(void);
static void doFreeShips(void);

static void doClassicScenario(void);
static void doAshesScenario(void);
static void doDisunitedScenario(void);

static const char *gConfigFile = "pmaster.cfg";
static Uns16 gNumRaces = 0;

static void
usage(char *argv[])
{
  fprintf(stderr,
        "Usage: %s [options] [GameDirectory [RootDirectory]]\n" "\n"
        "Files in the game directory will be overwritten so be careful! There must\n"
        "be a configuration file (by default, named 'pmaster.cfg') in either the\n"
        "game or root directory specifying the universe parameters. The documentation\n"
        "for this program is essentially contained in the 'pmaster.cfg' file that\n"
        "comes with the program. Use it as a template for creating your own\n"
        "configuration.\n" "\n" "options:\n"
        "        -f file   -- use 'file' as input rather than 'pmaster.cfg'\n"
        "        -h        -- prints this help summary and exits\n"
        "        -v        -- prints program version and exits\n", argv[0]);
  exit(1);
}

static void
processOptions(int argc, char *argv[])
{
  int c;

  while ((c = pgetopt(argc, argv, "hHf:v")) NEQ EOF) {
    switch (c) {
    case 'h':
    case 'H':
    default:
      usage(argv);

    case 'f':
      gConfigFile = poptarg;
      break;

    case 'v':
      printf("Portable MASTER version %u.%u\n", PMASTER_VERSION_MAJOR,
            PMASTER_VERSION_MINOR);
      exit(0);
    }
  }

  if (poptind < argc) {
    gGameDirectory = argv[poptind++];
  }
  if (poptind < argc) {
    gRootDirectory = argv[poptind++];
  }
  if (poptind < argc) {
    fprintf(stderr,
          "Too many parameters. First extraneous parameter is '%s'\n"
          "Type '%s -h' for a usage summary.\n", argv[poptind], argv[0]);
    exit(1);
  }
}

int
main(int argc, char *argv[])
{
  Uns16 race,
    cargo;
  Refdata_Struct lRefData;
  static char *lCargoNames[4] = {
    "Neutronium",
    "Tritanium",
    "Duranium",
    "Molybdenum"
  };

  processOptions(argc, argv);

  /* Initialize the PDK */
  InitPHOSTLib();

  gLogFile = OpenOutputFile("pmaster.log", GAME_DIR_ONLY | TEXT_MODE);

  /* Read our config file */
  DoDefaultAssignments();
  ReadMConfigFile(gConfigFile);

  /* We need to read in the global data files so we can inspect hull types,
     cargo capacities, etc. */
  if (!ReadGlobalData())
    exit(1);

  /* These are the files we must create: PDATA.HST SHIP.HST BDATA.HST
     MINES.HST NEXTTURN.HST GEN.HST LASTTURN.HST

     Call the Initialize...() routines to initialize these data structures.
     This is the only way we can call the Write_...() routines without first
     calling the Read_...() routines. */

  InitializePlanets();
  InitializeShips();
  InitializeBases();
  InitializeMines();
  InitializeHostGen();

  /* Set a random seed based on the number of seconds since Jan. 1, 1970 just 
     so we don't get the same universe every time. */
  SetRandomSeed((Uns16) time(0));

  /* Count the number of races we have */
  for (race = 1, gNumRaces = 0; race <= 11; race++) {
    if (cfg->mRaces[race])
      gNumRaces++;
  }

  memset(&lRefData, 0, sizeof(lRefData));

  Info("\nMastering a new game in directory: '%s'", gGameDirectory);
  Info("Planet and ship list data is in directory: '%s'\n", gRootDirectory);

  Info("Active races:");
  for (race = 1; race <= 11; race++) {
    if (cfg->mRaces[race]) {
      Info("    %2u: %s", race, RaceLongName(race, 0));
    }
    else {
      Info("    %2u: INACTIVE", race);
    }
  }

  Info("Surface mineral distributions on non-homeworld planets:");
  for (cargo = 0; cargo < 4; cargo++) {
    Info("    %10s: %u-%u KT", lCargoNames[cargo],
          cfg->mPlanetSurfaceRanges[cargo],
          cfg->mPlanetSurfaceRanges[cargo + 4]);
  }

  Info("Planet density distributions on non-homeworld planets:");
  for (cargo = 0; cargo < 4; cargo++) {
    Info("    %10s: %u-%u percent", lCargoNames[cargo],
          cfg->mPlanetDensityRanges[cargo],
          cfg->mPlanetDensityRanges[cargo + 4]);
  }

  Info("Planet core mineral distributions on non-homeworld planets:");
  for (cargo = 0; cargo < 4; cargo++) {
    Info("    %10s: %u-%u KT in %u%% of the planets\n"
          "            %u-%u KT in %u%% of the planets", lCargoNames[cargo],
          cfg->mPlanetCoreRangesUsual[cargo],
          cfg->mPlanetCoreRangesUsual[cargo + 4],
          cfg->mPlanetCoreUsualFrequency[cargo],
          cfg->mPlanetCoreRangesAlternate[cargo],
          cfg->mPlanetCoreRangesAlternate[cargo + 4],
          100 - cfg->mPlanetCoreUsualFrequency[cargo]);
  }

  Info("Chosen scenario:");

  /* Handle the rest differently based on scenario */
  switch (cfg->mScenario) {
  case Classic:
    Info("    Classic");
    doClassicScenario();
    lRefData.mScenario = 1;
    break;

  case Ashes:
    Info("    Ashes of the Old Empire");
    doAshesScenario();
    lRefData.mScenario = 6;
    lRefData.mOldEmpireRace = cfg->mOldEmpireRace;
    lRefData.mGameGoal = 6;
    break;

  case Disunited:
    Info("    Disunited Kingdoms");
    doDisunitedScenario();
    lRefData.mScenario = 8;
    break;

  default:
    passert(0);
    break;
  }

  /* Mark all players as active */
  for (race = 1; race <= 11; race++) {
    PutPlayerIsActive(race, cfg->mRaces[race]);
    if (cfg->mRaces[race]) {
      lRefData.mHomeworlds[race] = cfg->mHomeworlds[race];
    }
  }

  /* Write out their initial passwords */
  for (race = 1; race <= 11; race++) {
    if (!cfg->mRaces[race])
      continue;
    if (!strcmp(cfg->mPasswords[race], "none"))
      continue;

    PutPlayerPassword(race, cfg->mPasswords[race]);
  }

  /* Write out our universe. */
  if (!Write_Planets_File(PLANET_NR))
    exit(1);
  if (!Write_Ships_File(SHIP_NR))
    exit(1);
  if (!Write_Bases_File(PLANET_NR, 0))
    exit(1);
  if (!Write_Mines_File())
    exit(1);
  if (!Write_HostGen_File())
    exit(1);

  /* The turntime file NEXTTURN.HST is of 0 length after mastering */
  (void) OpenOutputFile("nextturn.hst", GAME_DIR_ONLY);

  /* The turntime file LASTTURN.HST is of length 20 after mastering and
     contains junk. */
  {
    FILE *lFile = OpenOutputFile("lastturn.hst", GAME_DIR_ONLY);
    char buf[20];

    memset(buf, 0, 20);
    (void) fwrite(buf, 20, 1, lFile);
    fclose(lFile);
  }

  /* Write out the REF.HST file */
  {
    FILE *lFile = OpenOutputFile("refdata.hst", GAME_DIR_ONLY);

    (void) fwrite(&lRefData, sizeof(lRefData), 1, lFile);
    fclose(lFile);
  }

  /* Write out the UFO.HST file */
  {
    FILE *lFile = OpenOutputFile("ufo.hst", GAME_DIR_ONLY);
    UFO_Struct *lUFO;

    lUFO = (UFO_Struct *) MemCalloc(100, sizeof(UFO_Struct));
    (void) fwrite(lUFO, 100, sizeof(UFO_Struct), lFile);
    MemFree(lUFO);
    fclose(lFile);
  }

  /* Close down. */
  FreePHOSTLib();

  return 0;
}

static void
doPlanetRandomize(void)
{
  Uns16 planet;
  Uns16 cargo;
  char FC[4];
  float lNatTypeCDF[9];
  float lNatGovCDF[9];
  Uns32 lNatTypeSum;
  Uns32 lNatGovSum;
  Uns16 lCount;
  static char *lNatTypeName[9] = {
    "Humanoid",
    "Bovinoid",
    "Reptilian",
    "Avian",
    "Amorphous",
    "Insectoid",
    "Amphibian",
    "Ghipsoldal",
    "Siliconoid"
  };
  static char *lNatGovName[9] = {
    "Anarchy",
    "Pre-Tribal",
    "Early-Tribal",
    "Tribal",
    "Feudal",
    "Monarchy",
    "Representative",
    "Participatory",
    "Unity"
  };

  /* First, change the relative ratios of native types and governments into a 
     CDF */
  for (lNatTypeSum = 0, lNatGovSum = 0, lCount = 0; lCount < 9; lCount++) {
    lNatTypeSum += cfg->mNativeTypeFrequencies[lCount];
    lNatGovSum += cfg->mNativeGovFrequencies[lCount];
  }

  passert(lNatTypeSum NEQ 0);
  passert(lNatGovSum NEQ 0);

  for (lCount = 0; lCount < 9; lCount++) {
    lNatTypeCDF[lCount] =
          cfg->mNativeTypeFrequencies[lCount] / (float) lNatTypeSum;
    lNatGovCDF[lCount] =
          cfg->mNativeGovFrequencies[lCount] / (float) lNatGovSum;
    if (lCount > 0) {
      lNatTypeCDF[lCount] += lNatTypeCDF[lCount - 1];
      lNatGovCDF[lCount] += lNatGovCDF[lCount - 1];
    }
  }
  lNatTypeCDF[8] = lNatGovCDF[8] = 1.0;

  Info("Natives will appear on %u%% of the planets.",
        cfg->mNativesOnPlanetFrequency);

  Info("Percentage of native types and governments:");
  for (lCount = 0; lCount < 9; lCount++) {
    Info("    %10s: %5.1f%%    %14s: %5.1f%%", lNatTypeName[lCount],
          100 * (lNatTypeCDF[lCount] - (lCount EQ 0 ? 0 : lNatTypeCDF[lCount -
                            1])), lNatGovName[lCount],
          100 * (lNatGovCDF[lCount] - (lCount EQ 0 ? 0 : lNatGovCDF[lCount -
                            1])));
  }

  Info("Natives will range from %lu to %lu clans.", cfg->mNativeClansRange[0],
        cfg->mNativeClansRange[1]);

  for (planet = 1; planet <= PLANET_NR; planet++) {
    if (!IsPlanetExist(planet))
      continue;

    /* Start setting attributes */
    PutPlanetOwner(planet, NoOwner);

    sprintf(FC, "%03u", RandomRange(1000)); /* random friendly code */
    PutPlanetFC(planet, FC);

    PutPlanetMines(planet, 0);
    PutPlanetFactories(planet, 0);
    PutPlanetDefense(planet, 0);

    PutPlanetColTax(planet, 0);
    PutPlanetNatTax(planet, 0);
    PutPlanetColHappy(planet, 80);
    PutPlanetNatHappy(planet, 80);

    /* Decide if this planet is to have natives. */
    if (RandomRange(100) < cfg->mNativesOnPlanetFrequency) {
      Uns32 lClans,
        lRange;
      float lRand;
      Uns16 lIndex;

      lRange = cfg->mNativeClansRange[1] - cfg->mNativeClansRange[0];
      lRange /= 10;
      passert(lRange < RAND_MAX);

      lClans =
            (Uns32) RandomRange((Uns16) lRange) * 10UL +
            cfg->mNativeClansRange[0];

      PutPlanetNativePopulation(planet, 100 * lClans);

      /* Determine native type */
      lRand = (float) RandomRange(10000) / 10000.0;
      for (lIndex = 0; lNatTypeCDF[lIndex] < lRand; lIndex++) /* NULL */
        ;

      PutPlanetNativeType(planet, 1 + lIndex);

      /* Determine native government */
      lRand = (float) RandomRange(10000) / 10000.0;
      for (lIndex = 0; lNatGovCDF[lIndex] < lRand; lIndex++) /* NULL */
        ;

      PutPlanetNatGovm(planet, 1 + lIndex);
    }
    else {
      PutPlanetNativePopulation(planet, 0);
    }

    /* Temperature */
    PutPlanetTemp(planet, RandomRange(101));

    /* Densities */
    for (cargo = 0; cargo < 4; cargo++) {
      Uns16 lRange;

      lRange = cfg->mPlanetDensityRanges[cargo + 4]
            - cfg->mPlanetDensityRanges[cargo];
      PutPlanetDensity(planet, cargo,
            RandomRange(lRange + 1) + cfg->mPlanetDensityRanges[cargo]);
    }

    /* Surface minerals */
    for (cargo = 0; cargo < 4; cargo++) {
      Uns16 lRange;

      lRange = cfg->mPlanetSurfaceRanges[cargo + 4]
            - cfg->mPlanetSurfaceRanges[cargo];
      PutPlanetCargo(planet, cargo,
            RandomRange(lRange + 1) + cfg->mPlanetSurfaceRanges[cargo]);
    }

    /* Core minerals */
    for (cargo = 0; cargo < 4; cargo++) {
      Uns16 lLow,
        lHigh,
        lRange;

      if (RandomRange(100) < cfg->mPlanetCoreUsualFrequency[cargo]) {
        lLow = cfg->mPlanetCoreRangesUsual[cargo];
        lHigh = cfg->mPlanetCoreRangesUsual[cargo + 4];
      }
      else {
        lLow = cfg->mPlanetCoreRangesAlternate[cargo];
        lHigh = cfg->mPlanetCoreRangesAlternate[cargo + 4];
      }
      lRange = lHigh - lLow;
      PutPlanetCore(planet, cargo, RandomRange(lRange + 1) + lLow);
    }
  }
}

static void
doHomeworlds(void)
{
  Uns16 index,
    bestPlanet;
  Int16 X,
    Y,
    DX,
    DY;
  Uns32 bestDistance,
    distance;
  float angle;
  Uns16 planet;
  Uns16 assigned[12],
    totalAssigned;
  Boolean planetUsed[PLANET_NR + 1];
  Uns16 race;

  /* Start off with a random angle from 0 to 2*PI */
  angle = RandomReal() * 2 * M_PI;

  /* Indicate that all races have not been assigned homeworlds */
  memset(assigned, 0, sizeof(assigned));

  /* Indicate that all planets are available */
  memset(planetUsed, 0, sizeof(planetUsed));

  totalAssigned = 0;
  for (index = 1; index <= 11 AND totalAssigned < gNumRaces;
        index++, angle += (2 * M_PI / gNumRaces)) {
    if (cfg->mHomeworldsRadius EQ 0) {
      if (!cfg->mRaces[index])
        continue;

      bestPlanet = cfg->mHomeworlds[index];

      X = PlanetLocationX(bestPlanet);
      Y = PlanetLocationY(bestPlanet);
      race = index;
    }
    else {
      /* Determine the (X,Y) co-ordinates that are gHomeworldRadius LY away
         from (2000,2000) at the current angle. */
      X = (Int16) (cfg->mHomeworldsRadius * cos(angle)) + 2000;
      Y = (Int16) (cfg->mHomeworldsRadius * sin(angle)) + 2000;

      do {
        race = RandomRange(11) + 1;
      } while (assigned[race] OR ! cfg->mRaces[race]);
    }

    /* Find the planet nearest to these co-ordinates. */
    bestPlanet = 0;
    bestDistance = 0x7FFFFFFFUL;
    for (planet = 1; planet <= PLANET_NR; planet++) {
      if (!IsPlanetExist(planet))
        continue;
      if (planetUsed[planet])
        continue;
      if ((PlanetOwner(planet) NEQ NoOwner)
            AND(PlanetOwner(planet) NEQ race)
            )
        continue;

      DX = PlanetLocationX(planet) - X;
      DY = PlanetLocationY(planet) - Y;
      distance = (Uns32) ((Int32) DX * DX + (Int32) DY * DY);
      if (distance < bestDistance) {
        bestDistance = distance;
        bestPlanet = planet;
        if (bestDistance EQ 0)
          break;
      }
    }
    passert(bestPlanet != 0);

    planetUsed[bestPlanet] = True;

    assigned[race] = True;
    totalAssigned++;
    PutPlanetOwner(bestPlanet, race);

    /* Save the homeworld */
    cfg->mHomeworlds[race] = bestPlanet;

    /* Now set up minerals and stuff for this homeworld. */
    PutPlanetMines(bestPlanet, 100);
    PutPlanetFactories(bestPlanet, 150);
    PutPlanetDefense(bestPlanet, 100);

    PutPlanetCargo(bestPlanet, NEUTRONIUM, cfg->mHomeworldSurfaceN[race]);
    PutPlanetCore(bestPlanet, NEUTRONIUM, cfg->mHomeworldCoreN[race]);
    PutPlanetCargo(bestPlanet, TRITANIUM, cfg->mHomeworldSurfaceT[race]);
    PutPlanetCore(bestPlanet, TRITANIUM, cfg->mHomeworldCoreT[race]);
    PutPlanetCargo(bestPlanet, DURANIUM, cfg->mHomeworldSurfaceD[race]);
    PutPlanetCore(bestPlanet, DURANIUM, cfg->mHomeworldCoreD[race]);
    PutPlanetCargo(bestPlanet, MOLYBDENUM, cfg->mHomeworldSurfaceM[race]);
    PutPlanetCore(bestPlanet, MOLYBDENUM, cfg->mHomeworldCoreM[race]);

    PutPlanetDensity(bestPlanet, NEUTRONIUM, 20);
    PutPlanetDensity(bestPlanet, TRITANIUM, 20);
    PutPlanetDensity(bestPlanet, DURANIUM, 15);
    PutPlanetDensity(bestPlanet, MOLYBDENUM, 95);

    /* Give them some colonists, supplies, and money */
    PutPlanetCargo(bestPlanet, COLONISTS, cfg->mHomeworldClans[race] * 100);
    PutPlanetCargo(bestPlanet, SUPPLIES, 400);
    PutPlanetCargo(bestPlanet, CREDITS, cfg->mHomeworldCash[race]);

    PutPlanetColTax(bestPlanet, 6);
    PutPlanetNatTax(bestPlanet, 0);
    PutPlanetColHappy(bestPlanet, 80);

    PutPlanetNativePopulation(bestPlanet, 0);
    PutPlanetNativeType(bestPlanet, NoNatives);

    /* Temperature */
    if (race == 7 AND cfg->mDesertCrystalHomeworld) {
      PutPlanetTemp(bestPlanet, 100);
    }
    else {
      PutPlanetTemp(bestPlanet, 50);
    }

    /* Now give this planet a starbase */
    if (cfg->mHomeworldBases[race]) {
      CreateBase(bestPlanet);   /* automatically sets owner from planet */
      PutBaseDefense(bestPlanet, 10);
      PutBaseFighters(bestPlanet, 20);
      PutBaseOrder(bestPlanet, Refuel);
      PutBaseTech(bestPlanet, ENGINE_TECH, cfg->mBaseEngineTech[race]);

      /* Put some components in storage. */
      PutBaseHulls(bestPlanet, 1, 1); /* One free ship in slot 1 */
      PutBaseHulls(bestPlanet, 2, 1); /* One free ship in slot 2 */
      PutBaseEngines(bestPlanet, 1, 4); /* Four free StarDrive 1 engines */
      PutBaseEngines(bestPlanet, 3, 2); /* Two free StarDrive 3 engines */
      PutBaseBeams(bestPlanet, 2, 8); /* Eight free X-Rays */
      PutBaseTubes(bestPlanet, 2, 3); /* Three free Proton torpedo tubes */
      PutBaseTorps(bestPlanet, 2, 20); /* 20 free Proton torpedoes */
    }
  }
}

static void
doFreeShips(void)
{
  Uns16 race;
  Uns16 ship;
  Uns16 hullNumber;
  char FC[4];
  Uns16 ammo;

  /* Give 2 free ships to each race. These ships are the first two that they
     can build with Blasters, Mark 5 torps, and up to 20 fish. */

  for (hullNumber = 1; hullNumber <= 2; hullNumber++) {
    for (race = 1; race <= 11; race++) {
      if (!cfg->mRaces[race])
        continue;
      if (!cfg->mTwoFreeShips[race])
        continue;

      ship = CreateShip(race);

      /* Give it a hull type */
      PutShipHull(ship, TrueHull(race, hullNumber));

      /* Give it a name */
      PutShipName(ship, HullName(TrueHull(race, hullNumber), 0));

      /* Give it a friendly code */
      sprintf(FC, "%03u", RandomRange(1000));
      PutShipFC(ship, FC);

      /* Fuel it */
      PutShipCargo(ship, NEUTRONIUM, HullFuelCapacity(ShipHull(ship)));

      /* Give it HyperDrive 6 engines */
      PutShipEngine(ship, 6);

      /* Give it crew */
      PutShipCrew(ship, HullCrewComplement(ShipHull(ship)));

      /* Give it beam weapons */
      if (HullBeamNumber(ShipHull(ship)) > 0) {
        PutShipBeamNumber(ship, HullBeamNumber(ShipHull(ship)));
        PutShipBeamType(ship, 4); /* blasters */
      }

      /* Give it tubes or bays */
      if (HullBays(ShipHull(ship)) > 0) {
        PutShipBays(ship, HullBays(ShipHull(ship)));
        ammo = 20;
      }
      else if (HullTubeNumber(ShipHull(ship)) > 0) {
        PutShipTubeNumber(ship, HullTubeNumber(ShipHull(ship)));
        PutShipTorpType(ship, 7); /* Mark 5 */
        ammo = 20;
      }
      else {
        ammo = 0;
      }

      /* Give it ammo */
      ammo = min(ammo, HullCargoCapacity(ShipHull(ship)));
      PutShipAmmunition(ship, ammo);

      /* Place it at the homeworld. */
      PutShipLocationX(ship, PlanetLocationX(cfg->mHomeworlds[race]));
      PutShipLocationY(ship, PlanetLocationY(cfg->mHomeworlds[race]));

      /* Clear its waypoint */
      PutShipRelWaypointX(ship, 0);
      PutShipRelWaypointY(ship, 0);
    }
  }
}

typedef struct {
  Uns16 mPlanet;
  Int32 mDistSqr;
} PlanetDist_Struct;

static int
nearestPlanetCompare(const void *p1, const void *p2)
{
  const PlanetDist_Struct *l1 = (const PlanetDist_Struct *) p1;
  const PlanetDist_Struct *l2 = (const PlanetDist_Struct *) p2;

  return sgn((l1->mDistSqr - l2->mDistSqr));
}

static Uns16 *
findNearestPlanets(Uns16 pPlanet)
{
  static Uns16 *lPlanets = 0;
  PlanetDist_Struct *lDists = 0;
  Uns16 lX,
    lY,
    lPlanet,
    lTotalPlanets;
  Int16 lDX,
    lDY;

  if (lPlanets EQ 0) {
    lPlanets = MemCalloc(PLANET_NR + 1, sizeof(lPlanets[0]));
  }
  lDists = MemCalloc(PLANET_NR + 1, sizeof(lDists[0]));

  lX = PlanetLocationX(pPlanet);
  lY = PlanetLocationY(pPlanet);

  for (lPlanet = 1, lTotalPlanets = 0; lPlanet <= PLANET_NR; lPlanet++) {
    if (!IsPlanetExist(lPlanet))
      continue;

    lDX = (Int16) PlanetLocationX(lPlanet) - (Int16) lX;
    lDY = (Int16) PlanetLocationY(lPlanet) - (Int16) lY;

    lDists[lTotalPlanets].mPlanet = lPlanet;
    lDists[lTotalPlanets++].mDistSqr = (Int32) lDX *lDX + (Int32) lDY *lDY;
  }

  qsort(lDists, lTotalPlanets, sizeof(lDists[0]), nearestPlanetCompare);

  for (lPlanet = 0; lPlanet < lTotalPlanets; lPlanet++) {
    lPlanets[lPlanet] = lDists[lPlanet].mPlanet;
  }
  lPlanets[lPlanet] = 0;

  MemFree(lDists);

  return lPlanets;
}

static void
doClassicScenario(void)
{
  /* Now, put some random minerals on planets, add natives, set temperature,
     etc. */
  doPlanetRandomize();

  /* Make some planets into homeworlds */
  doHomeworlds();

  /* Give each race two free ships */
  doFreeShips();
}

static void
doAshesScenario(void)
{
  Uns16 *lClosePlanets;
  Uns16 lPlanets;

  /* Put some random minerals on planets, add natives, set temperature, etc. */
  doPlanetRandomize();

  /* Assign planets to old empire */
  lClosePlanets = findNearestPlanets(cfg->mHomeworlds[cfg->mOldEmpireRace]);

  for (lPlanets = 0; *lClosePlanets AND lPlanets <= cfg->mOldEmpirePlanets;
        lPlanets++, lClosePlanets++) {
    PutPlanetOwner(*lClosePlanets, cfg->mOldEmpireRace);
    PutPlanetCargo(*lClosePlanets, COLONISTS, 100000UL);
    PutPlanetMines(*lClosePlanets, 10);
    PutPlanetFactories(*lClosePlanets, 100);
    PutPlanetDefense(*lClosePlanets, 5);
    PutPlanetColTax(*lClosePlanets, 5);
    PutPlanetNatTax(*lClosePlanets, 5);

    /* lPlanets==0 is the homeworld, lPlanets==1 is the next closest planet. */
    if (lPlanets EQ 1) {
      PutPlanetCargo(*lClosePlanets, NEUTRONIUM, 10000);
      PutPlanetCore(*lClosePlanets, NEUTRONIUM, 1000000UL);
      PutPlanetDensity(*lClosePlanets, NEUTRONIUM, 90);
    }
    else {
      PutPlanetCargo(*lClosePlanets, NEUTRONIUM, 0);
      PutPlanetCore(*lClosePlanets, NEUTRONIUM, 0);
    }
  }

  /* Make some planets into homeworlds */
  doHomeworlds();

  /* Give each race two free ships */
  doFreeShips();
}

static void
copyPlanetToPlanet(Uns16 pSrc, Uns16 pDst)
{
#define PUT1(x) PutPlanet ## x (pDst, Planet ## x(pSrc))
#define PUT2(x,y) PutPlanet ## x (pDst, y, Planet ## x(pSrc, y))

  PUT2(Cargo, NEUTRONIUM);
  PUT2(Cargo, TRITANIUM);
  PUT2(Cargo, DURANIUM);
  PUT2(Cargo, MOLYBDENUM);
  PUT2(Core, NEUTRONIUM);
  PUT2(Core, TRITANIUM);
  PUT2(Core, DURANIUM);
  PUT2(Core, MOLYBDENUM);
  PUT2(Density, NEUTRONIUM);
  PUT2(Density, TRITANIUM);
  PUT2(Density, DURANIUM);
  PUT2(Density, MOLYBDENUM);
  PUT1(NatGovm);
  PUT1(NativePopulation);
  PUT1(NatType);
  PUT1(Temp);

#undef PUT1
#undef PUT2
}

static void
doDisunitedScenario(void)
{
  Uns16 *lClosePlanets;
  Uns16 lPlanets,
    lRace;
  Uns16 *lPlanetList = 0;

  /* Put some random minerals on planets, add natives, set temperature, etc. */
  doPlanetRandomize();

  /* Make some planets into homeworlds */
  doHomeworlds();

  /* Now compute the maximum number of owned planets any one race has. Only
     need to do this if we're setting each race's initial conditions to be
     the same. */
  if (cfg->mAllOwnedPlanetsTheSame) {
    /* Allocate our array list */
    lPlanetList = (Uns16 *) MemCalloc(PLANET_NR + 1, sizeof(Uns16));
  }

  /* Start finding owned planets */
  for (lRace = 1; lRace <= 11; lRace++) {
    if (!cfg->mRaces[lRace])
      continue;

    /* Find the N nearest planets */
    lClosePlanets = findNearestPlanets(cfg->mHomeworlds[lRace]);

    for (lPlanets = 0; lPlanets < cfg->mOwnedPlanets[lRace] - 1;
          lPlanets++, lClosePlanets++) {
      while (*lClosePlanets AND(PlanetOwner(*lClosePlanets) NEQ NoOwner))
        lClosePlanets++;

      if (*lClosePlanets EQ 0) {
        Error("Unable to assign owned planets");
        exit(1);
      }

      PutPlanetOwner(*lClosePlanets, lRace);
      if (lPlanetList NEQ 0) {
        if (lPlanetList[lPlanets] EQ 0) {
          lPlanetList[lPlanets] = *lClosePlanets;
        }
        else {
          /* Copy the stored planet into this one. */
          copyPlanetToPlanet(lPlanetList[lPlanets], *lClosePlanets);
        }
      }

      PutPlanetCargo(*lClosePlanets, COLONISTS, 100000UL);
      PutPlanetMines(*lClosePlanets, 10);
      PutPlanetFactories(*lClosePlanets, 100);
      PutPlanetDefense(*lClosePlanets, 5);
      PutPlanetColTax(*lClosePlanets, 5);
      PutPlanetNatTax(*lClosePlanets, 0);
    }
  }

  /* Give each race two free ships */
  doFreeShips();

  MemFree(lPlanetList);
}

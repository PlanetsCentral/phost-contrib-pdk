
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

static const char *CONFIG_FILE = "pconfig.src";

/* 'Data' is used to mirror gConfigInfo, but it is char * so we can
   index into it based upon byte position. */
static char *Data = 0;

static void DoDefaultAssignments(void);
static Boolean DoAssignment(const char *name, char *val,

      const char *pInputLine);
static const char *languageName(Language_Def pLanguage);

/* ---------------------------------------------------------------------- */

IO_Def
ConfigFileReader(FILE * pInFile, const char *pFileName, const char *pSection,
      Boolean pUseDefaultSection, configAssignment_Func pAssignFunc)
{
  char inbuf[256];
  char lInputLine[256];
  unsigned line;
  Boolean lInSection = pUseDefaultSection;

  line = 0;

  while (!feof(pInFile)) {
    char *p,
     *ep;

    line++;
    if (0 EQ fgets(inbuf, 255, pInFile)) {
      if (!feof(pInFile)) {
        /* ErrorExit("%s: unable to read file at line %u", pFileName, line); */
        Error("%s: unable to read file at line %u", pFileName, line);
        return IO_FAILURE;
      }
      break;
    }

    p = SkipWS(inbuf);
    if ((p EQ 0)
          OR(*p EQ '#')
          ) {
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
      /* ErrorExit("%s: expected assignment at line %u", pFileName, line); */
      Error("%s: expected assignment at line %u", pFileName, line);
      return IO_FAILURE;
    }

    *ep++ = 0;
    ep = SkipWS(ep);
    if (ep EQ 0) {
      /* ErrorExit("%s: expecting value to assign at line %u", pFileName,
         line); */
      Error("%s: expecting value to assign at line %u", pFileName, line);
      return IO_FAILURE;
    }
    TrimTrailingWS(ep);

    TrimTrailingWS(p);
    if (*p EQ 0) {
      /* ErrorExit("%s: expecting parameter name to assign to at line %u",
         pFileName, line); */
      Error("%s: expecting parameter name to assign to at line %u", pFileName,
            line);
      return IO_FAILURE;
    }

    if (!(*pAssignFunc) (p, ep, lInputLine)) {
      /* ErrorExit("%s: parameter error at line %u", pFileName, line); */
      Error("%s: parameter error at line %u", pFileName, line);
      return IO_FAILURE;
    }
  }
  return IO_SUCCESS;
}

IO_Def
Read_HConfig_File(void)
{
  FILE *lConfigFile;
  IO_Def lRes;

  lConfigFile = OpenInputFile(CONFIG_FILE, GAME_DIR_ONLY | NO_MISSING_ERROR);
  if (lConfigFile == NULL)
    return IO_FAILURE;

  Data = (char *) gConfigInfo;

  DoDefaultAssignments();

  lRes =
        ConfigFileReader(lConfigFile, CONFIG_FILE, "phost", True,
        DoAssignment);

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

#define CFDefine(name, member, num, type, def, MA) name,
static const char *Names[] = {
#include "config.hi"
  0
};

#define CF_NumItems ((sizeof(Names)/sizeof(Names[0]))-1)

#define CFDefine(name, member, num, type, def, MA) offsetof(Hconfig_Struct, member),
static size_t Pos[] = {
#include "config.hi"
  0
};

#define CFDefine(name, member, num, type, def, MA) num,
static Uns16 Elements[] = {
#include "config.hi"
  0
};

#define CFDefine(name, member, num, type, def, MA) CFType_ ## type ,
static CFType Types[] = {
#include "config.hi"
  CFType_NoType
};

#define CFDefine(name, member, num, type, def, MA) def,
static const char *Defaults[] = {
#include "config.hi"
  0
};

#define CFDefine(name, member, num, type, def, MA) MA,
static Boolean gAllowMA[] = {
#include "config.hi"
  0
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

#if 0
static const char *
scoreMethodName(ScoreMethod_Def pMethod)
{
  static const char *lMethod[] = {
    "None",
    "Compatible"
  };

  return lMethod[pMethod];
}
#endif

/* Check each assignment for a valid parameter value. Boolean-type assignments
   need not be checked since they've already been verified by ListMatch */

static int
checkValue(Uns16 ix, long val)
{
  static struct {
    long minval;
    long maxval;
  } sCheck[CF_NumItems] = {
    {
    0, 100},                    /* RecycleRate */
    {
    0, 100},                    /* RandomMeteorRate */
    {
    0, 1},                      /* AllowMinefields */
    {
    0, 1},                      /* AllowAlchemy */
    {
    0, 1},                      /* DeleteOldMessages */
    {
    0, 1},                      /* DisablePasswords */
    {
    1, 32767},                  /* GroundKillFactor */
    {
    1, 32767},                  /* GroundDefenseFactor */
    {
    0, 60},                     /* FreeFighters */
    {
    0, 1638},                   /* RaceMiningRate */
    {
    0, 10000},                  /* RaceTaxRate */
    {
    0, 1},                      /* RebelsBuildFighters */
    {
    0, 1},                      /* ColoniesBuildFighters */
    {
    0, 1},                      /* RobotsBuildFighters */
    {
    0, 100},                    /* CloakFailureRate */
    {
    0, 1},                      /* RobCloakedShips */
    {
    0, 32767},                  /* ScanRange */
    {
    0, 32767},                  /* DarkSenseRange */
    {
    0, 1},                      /* AllowHiss */
    {
    0, 1},                      /* AllowRebelGroundAttack */
    {
    0, 1},                      /* AllowSuperRefit */
    {
    0, 1},                      /* AllowWebMines */
    {
    0, 1000},                   /* CloakFuelBurn */
    {
    0, 32767},                  /* SensorRange */
    {
    0, 1},                      /* AllowNewNatives */
    {
    0, 1},                      /* AllowPlanetAttacks */
    {
    0, 100},                    /* BorgAssimilationRate */
    {
    0, 100},                    /* WebMineDecayRate */
    {
    0, 100},                    /* MineDecayRate */
    {
    0, 32767},                  /* MaximumMinefieldRadius */
    {
    0, 100},                    /* TransuraniumDecayRate */
    {
    0, 1000},                   /* StructureDecayPerTurn */
    {
    0, 1},                      /* AllowEatingSupplies */
    {
    0, 1},                      /* AllowNoFuelMovement */
    {
    0, 100},                    /* MineHitOdds */
    {
    0, 100},                    /* WebMineHitOdds */
    {
    0, 32767},                  /* MineScanRange */
    {
    0, 1},                      /* AllowMinesDestroyMines */
    {
    0, 1},                      /* AllowEngineShieldBonus */
    {
    0, 100},                    /* EngineShieldBonusRate */
    {
    0, 32767},                  /* ColonialFighterSweepRate */
    {
    0, 1},                      /* AllowColoniesSweepWebs */
    {
    0, 32767},                  /* MineSweepRate */
    {
    0, 32767},                  /* WebMineSweepRate */
    {
    0, 100},                    /* HissEffectRate */
    {
    0, 100},                    /* RobFailureOdds */
    {
    0, 1},                      /* PlanetsAttackRebels */
    {
    0, 1},                      /* PlanetsAttackKlingons */
    {
    0, 32767},                  /* MineSweepRange */
    {
    0, 32767},                  /* WebMineSweepRange */
    {
    0, 1},                      /* AllowScienceMissions */
    {
    0, 1000},                   /* MineHitOddsWhenCloakedX10 */
    {
    0, 100},                    /* DamageLevelForCloakFail */
    {
    0, 1},                      /* AllowFedCombatBonus */
    {
    0, 100},                    /* MeteorShowerOdds */
    {
    0, 32767},                  /* MeteorShowerOreRanges */
    {
    0, 500},                    /* LargeMeteorsImpacting */
    {
    0, 32767},                  /* LargeMeteorOreRanges */
    {
    0, 1},                      /* AllowMeteorMessages */
    {
    0, 1},                      /* AllowOneEngineTowing */
    {
    0, 1},                      /* AllowHyperWarps */
    {
    0, 100},                    /* ClimateDeathRate */
    {
    0, 1},                      /* AllowGravityWells */
    {
    0, 1},                      /* CrystalsPreferDeserts */
    {
    0, 1},                      /* AllowMinesDestroyWebs */
    {
    0, 1},                      /* ClimateLimitsPopulation */
    {
    0, 0x7FFFFFFFUL},           /* MaxPlanetaryIncome */
    {
    0, 100},                    /* IonStormActivity */
    {
    0, 1},                      /* AllowChunneling */
    {
    0, 1},                      /* AllowDeluxeSuperSpy */
    {
    0, 1},                      /* IonStormsHideMines */
    {
    0, 1},                      /* AllowGloryDevice */
    {
    0, 1},                      /* AllowAntiCloakShips */
    {
    0, 1},                      /* AllowGamblingShips */
    {
    0, 1},                      /* AllowCloakedShipAttack */
    {
    0, 1},                      /* AllowShipCloning */
    {
    0, 1},                      /* AllowBoardingParties */
    {
    0, 1},                      /* AllowImperialAssault */
    {
    0, 100},                    /* RamScoopFuelPerLY */
    {
    0, 1},                      /* AllowAdvancedRefinery */
    {
    0, 1},                      /* AllowBioscanners */
    {
    0, 11},                     /* HullTechNotSlowedByMines */
    {
    0, 1},                      /* UseAccurateFuelModel */
    {
    0, 32767},                  /* DefenseForUndetectable */
    {
    0, 32767},                  /* FactoriesForDetectable */
    {
    0, 32767},                  /* MinesForDetectable */
    {
    0, 32767},                  /* ColonialFighterSweepRange */
    {
    0, 327},                    /* MineHitDamageFor100KT */
    {
    0, 327},                    /* WebHitDamageFor100KT */
    {
    0, 1},                      /* AllowRegisteredFunctions */
    {
    0, 32767},                  /* GravityWellRange */
    {
    0, 0},                      /* Language */
    {
    0, 1},                      /* AllowPlayerMessages */
    {
    0, 1},                      /* ScoringMethod */
    {
    0, 1},                      /* TowedShipsBreakFree */
    {
    0, 100},                    /* NativeClimateDeathRate */
    {
    0, 1},                      /* AllowMoreThan50Targets */
    {
    0, 1},                      /* CrystalSinTempBehavior */
    {
    0, 1},                      /* RGANeedsBeams */
    {
    0, 1},                      /* AllowRGAOnUnowned */
    {
    0, 1},                      /* CPEnableLanguage */
    {
    0, 1},                      /* CPEnableBigTargets */
    {
    0, 1},                      /* CPEnableRaceName */
    {
    0, 1},                      /* CPEnableAllies */
    {
    0, 1},                      /* CPEnableMessage */
    {
    0, 1},                      /* CPEnableRumor */
    {
    0, 1},                      /* DelayAllianceCommands */
    {
    1, 100},                    /* TerraformRate */
    {
    0, 32767},                  /* MaxColTempSlope */
    {
    0, 32767},                  /* WebDrainFuelLoss */
    {
    0, 1},                      /* AllowWormholes */
    {
    0, 2000},                   /* WrmDisplacement */
    {
    0, 2000},                   /* WrmRandDisplacement */
    {
    -1000, 1000},               /* WrmStabilityAddX10 */
    {
    0, 100},                    /* WrmRandStability */
    {
    -10000, 10000},             /* WrmMassAdd */
    {
    0, 10000},                  /* WrmRandMass */
    {
    0, 1},                      /* WrmVoluntaryTravel */
    {
    1, 32767},                  /* WrmTravelDistDivisor */
    {
    0, 9},                      /* WrmTravelWarpSpeed */
    {
    0, 1},                      /* WrmTravelCloaked */
    {
    0, 100},                    /* WrmEntryPowerX100 */
    {
    0, 1},                      /* CPEnableGive */
    {
    0, 1},                      /* AllowTowCloakedShips */
    {
    0, 100},                    /* RobCloakedChance */
    {
    0, 100},                    /* PlanetaryTorpsPerTube */
    {
    0, 32767},                  /* UnitsPerTorpRate */
    {
    0, 1},                      /* AllowESBonusAgainstPlanets */
    {
    0, 32767},                  /* ShipCloneCostRate */
    {
    0, 1},                      /* AllowHyperjumpGravWells */
    {
    0, 100},                    /* NativeCombatSurvivalRate */
    {
    0, 1},                      /* AllowPrivTowCapture */
    {
    0, 1},                      /* AllowCrystalTowCapture */
    {
    0, 0},                      /* GameName */
    {
    0, 1},                      /* RoundWarpWells */
    {
    0, 1},                      /* CPEnableSend */
    {
    0, 1},                      /* CPEnableJettison */
    {
    0, 1},                      /* CumulativePillaging */
    {
    0, 1},                      /* AllowInterceptAttack */
    {
    0, 32767},                  /* RaceGrowthRate */
    {
    1, 12},                     /* PlayerRace */
    {
    0, 32767},                  /* ProductionRate */
    {
    0, 10000},                  /* MineOddsWarpBonus */
    {
    0, 10000},                  /* CloakMineOddsWarpBonus */
    {
    0, 10000},                  /* WebMineOddsWarpBonus */
    {
    0, 9},                      /* MineTravelSafeWarp */
    {
    0, 9},                      /* CloakTravelSafeWarp */
    {
    0, 9},                      /* WebMineTravelSafeWarp */
    {
    0, 1},                      /* CloakFailMessages */
    {
    0, 1},                      /* TonsScoreCountsPlanets */
    {
    20, 900},                   /* ExtMissionsStartAt */
    {
    1, 801},                    /* WormholeUFOsStartAt */
    {
    1, 500},                    /* MaxShipsHissing */
    {
    0, 1},                      /* AllowExtendedMissions */
    {
    0, 100},                    /* SpyDetectionChance */
    {
    0, 1},                      /* MapTruehullByPlayerRace */
    {
    0, 1},                      /* AllowWraparoundMap */
    {
    0, 10000},                  /* WraparoundRectangle */
    {
    0, 32767},                  /* Dummy1 (old ColFighterSweepRate) */
    {
    0, 32767},                  /* Dummy2 (old ColFighterSweepRange) */
    {
    0, 1},                      /* CPEnableRemote */
    {
    0, 1},                      /* AllowAlliedChunneling */
    {
    0, 10000},                  /* ColTaxRate */
    {
    0, 10000},                  /* NatTaxRate */
    {
    0, 1},                      /* AllowAlternativeTowing */
    {
    0, 1},                      /* AllowBeamUpClans */
    {
    0, 1},                      /* AllowBeamUpMultiple */
    {
    0, 100},                    /* SBQBuildPALBoost */
    {
    0, 100},                    /* SBQNewBuildPALBoost */
    {
    0, 10000},                  /* SBQPointsForAging */
    {
    -2147483647L, 2147483647L}, /* SBQBuildChangePenalty */
    {
    0, 100},                    /* PALDecayPerTurn */
    {
    0, 100},                    /* PALPlayerRate */
    {
    0, 100},                    /* PALCombatAggressor */
    {
    0, 100},                    /* PALAggressorPointsPer10KT */
    {
    0, 100},                    /* PALOpponentPointsPer10KT */
    {
    0, 100},                    /* PALAggressorKillPointsPer10KT */
    {
    0, 100},                    /* PALOpponentKillPointsPer10KT */
    {
    0, 200},                    /* PALCombatPlanetScaling */
    {
    0, 200},                    /* PALCombatBaseScaling */
    {
    0, 100},                    /* PALShipCapturePer10Crew */
    {
    0, 100},                    /* PALRecyclingPer10KT */
    {
    0, 100},                    /* PALBoardingPartyPer10Crew */
    {
    0, 100},                    /* PALGroundAttackPer100Clans */
    {
    0, 100},                    /* PALGloryDevice */
    {
    0, 100},                    /* PALGloryDamagePer10KT */
    {
    0, 100},                    /* PALImperialAssault */
    {
    0, 100},                    /* PALRGA */
    {
    0, 100},                    /* PALPillage */
    {
    0, 1},                      /* FilterPlayerMessages */
    {
    0, 1},                      /* AlternativeAntiCloak */
    {
    0, 1},                      /* AntiCloakImmunity */
    {
    0, 1},                      /* AllowMoreThan500Minefields */
    {
    0, 10000},                  /* CPNumMinefields */
    {
    0, 100},                    /* PALShipMinekillPer10KT */
    {
    0, 16384},                  /* BayRechargeRate */
    {
    -500, 500},                 /* BayRechargeBonus */
    {
    0, 16384},                  /* BeamRechargeRate */
    {
    -4095, 4095},               /* BeamRechargeBonus */
    {
    0, 16384},                  /* TubeRechargeRate */
    {
    0, 1000},                   /* BeamHitFighterCharge */
    {
    0, 1000},                   /* BeamHitShipCharge */
    {
    0, 100000L},                /* TorpFiringRange */
    {
    0, 100000L},                /* BeamFiringRange */
    {
    0, 100},                    /* TorpHitOdds */
    {
    0, 100},                    /* BeamHitOdds */
    {
    -4095, 4095},               /* BeamHitBonus */
    {
    1, 100},                    /* StrikesPerFighter */
    {
    0, 50},                     /* FighterKillOdds */
    {
    1, 1000},                   /* FighterBeamExplosive */
    {
    1, 1000},                   /* FighterBeamKill */
    {
    1, 10000},                  /* ShipMovementSpeed */
    {
    1, 10000},                  /* FighterMovementSpeed */
    {
    1, 10000},                  /* BayLaunchInterval */
    {
    0, MAX_FIGHTER_LIMIT},      /* MaxFightersLaunched */
    {
    0, 1},                      /* AllowAlternativeCombat */
    {
    3000, 60000L},              /* StandoffDistance */
    {
    0, 1},                      /* PlanetsHaveTubes */
    {
    0, 1},                      /* FireOnAttackFighters */
    {
    -4095, 4095},               /* TorpHitBonus */
    {
    -4095, 4095},               /* TubeRechargeBonus */
    {
    0, 32767},                  /* ShieldDamageScaling */
    {
    0, 32767},                  /* HullDamageScaling */
    {
    0, 32767},                  /* CrewKillScaling */
    {
    -20, 20},                   /* ExtraFighterBays */
    {
    0, 100000UL},               /* BeamHitFighterRange */
    {
    0, 100000UL}                /* FighterFiringRange */
  };
  passert(ix < CF_NumItems);
  return (val >= sCheck[ix].minval AND val <= sCheck[ix].maxval);
}

/* When this variable is true, we don't print out any errors w.r.t.
   assigning to obsolete variables */
static Boolean gAssigningDefaults = False;

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
    Error("%s: Parameter '%s' is not recognized", CONFIG_FILE, name);
    return False;
  }

  if (!gAssigningDefaults AND UserSet[ix]) {
    Error("%s: Parameter '%s' is being assigned twice", CONFIG_FILE,
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
      Error("%s: the '%s' parameter is obsolete", CONFIG_FILE, Names[ix]);
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

  Error("%s: extraneous data beyond assignment", CONFIG_FILE);
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
      Error("%s: not enough elements in assignment", CONFIG_FILE);
    }
    return 0;
  }

  *retval = strtol(p, &endptr, 0);
  if (*endptr NEQ 0) {
    Error("%s: illegal numeric value '%s'", CONFIG_FILE, str);
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
        Error("%s: illegal 8-bit quantity '%ld'", CONFIG_FILE, uval);
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
        Error("%s: illegal 16-bit unsigned quantity '%ld'", CONFIG_FILE,
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
        Error("%s: illegal 16-bit signed quantity '%ld'", CONFIG_FILE, uval);
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
        }
        else {
          Error("%s: not enough elements in assignment", CONFIG_FILE);
          return 0;
        }
      }

      if (!lDoMA) {
        match = ListMatch(p, "False True No Yes"); /* order is important */
        if (match < 0) {
          Error("%s: illegal boolean parameter '%s'", CONFIG_FILE, p);
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
        }
        else {
          Error("%s: not enough elements in assignment", CONFIG_FILE);
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
          Error("%s: illegal language '%s'", CONFIG_FILE, p);
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
      Error("%s: not enough elements in assignment", CONFIG_FILE);
      return 0;
    }

    /* The order of methods in the following list is important and must match 
       the order of definitions in auxdata.c */
    match = ListMatch(p, "None Compatible");

    if (match < 0) {
      Error("%s: illegal scoring method '%s'", CONFIG_FILE, p);
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
    Warning("%s: assignment to '%s' exceeds string length", CONFIG_FILE,
          Names[ix]);
  }
  n = min(n, strlen(val));

  memset(Data + Pos[ix], 0, Elements[ix]);
  strncpy(Data + Pos[ix], val, n);

  return 1;
}

/*************************************************************
  $HISTORY:$
**************************************************************/

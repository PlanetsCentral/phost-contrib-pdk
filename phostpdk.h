/*!
 *  \file phostpdk.h
 *  \brief Header file for the Portable Host Development Kit (PDK)
 *  Version 3.1
 *
 *   Copyright 1995-1998 by Andrew Sterian, Thomas Voigt, and Steffen Pietsch
 */

#ifndef _PHOSTPDK_H_
#define _PHOSTPDK_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define   RACE_NR  11           /*!< Number of races */
#define   SHIP_NR    500        /*!< Max number of ships */
#define   PLANET_NR    500      /*!< Number of planets */
#define   BASE_NR    500        /*!< Max number of bases */
#define   MINE_NR  10000        /*!< Max number of mines */
#define   HULL_NR    105        /*!< Number of hulls in HULLSPEC.DAT */
#define   ENGINE_NR   9         /*!< Number of engines in ENGSPEC.DAT */
#define   BEAM_NR  10           /*!< Number of beams in BEAMSPEC.DAT */
#define   TORP_NR  10           /*!< Number of torps in TORPSPEC.DAT */

#define   LONGNAME_SIZE     30  /*!< Length of full race name */
#define   SHORTNAME_SIZE    20  /*!< Length of short race name */
#define   ADJECTIVE_SIZE    12  /*!< Length of race adjective */
#define   RACENAME_RACE_SIZE   (LONGNAME_SIZE+SHORTNAME_SIZE+ADJECTIVE_SIZE)
#define   RACENAME_SIZE     (RACENAME_RACE_SIZE*11)
  /*!< Size of RACE.NM file */
#define   PLANETNAME_SIZE    20 /*!< size of a record in PLANET.NM */

#define RACEHULLS    20         /*!< Max number of hulls per race */
#define MAX_SPEED     9         /*!< Max warp speed on a ship */
#define MAX_BEAMS    20         /*!< Max number of beams on a ship */
#define MAX_TUBES    20         /*!< Max number of tubes on a ship */
#define MAX_BAYS    20          /*!< Max number of bays on a ship */
#define MAX_PLANET_BAYS   50    /*!< max number of bays on a planet/SB */
#define MAX_BASE_DEFENSE 200    /*!< Max defences for a base */
#define MAX_BASE_FIGHTERS  60   /*!< Max fighters for a base */
#define MAX_TECH    10          /*!< Max tech level */
#define MAX_DENSITY   100       /*!< Max mineral density in a planet */
#define MAX_HAPPY   100         /*!< Max happiness for colonists and natives 
                                 */
#define MIN_HAPPY   (-300)      /*!< Min happiness for colonists and natives 
                                 */
#define MAX_COORDINATE   10000  /*!< coordinate limits given by planets.exe */
#define MIN_COORDINATE    0
#define MAX_SHIP_CREDITS  10000 /*!< max credits allowed on a ship */
#define MAX_WORMHOLES  100      /*!< maximum supported wormholes */
#define MAX_MINES   700         /*!< sqrt(249800) + 200 */
#define MAX_FACTORIES  600      /*!< sqrt(249900) + 100 */
#define MAX_DEFENSE   550       /*!< sqrt(249950) +  50 */
#define MAX_POPULATION 25000000UL /*!< absolute maximum population on a
                                     planet */
#define MAX_TAX    100          /*!< max tax rate on a planet */
#define MAX_TEMP   100          /*!< max planet temperature */

  typedef short Int16;
  typedef unsigned short Uns16;

#ifdef __alpha                  /* Alpha AXP defines long as 64 bits */
  typedef int Int32;
  typedef unsigned int Uns32;
#else
  typedef long Int32;
  typedef unsigned long Uns32;
#endif

  typedef enum { False, True } Boolean;
  typedef Boolean IO_Def;
#define IO_SUCCESS True
#define IO_FAILURE False

  typedef Uns16 RandType_Def;

  typedef enum {
    NEUTRONIUM = 0,
    TRITANIUM,
    DURANIUM,
    MOLYBDENUM,
    COLONISTS,
    SUPPLIES,
    CREDITS,
    NumCargoTypes
  } CargoType_Def;

  typedef enum {
    NoOwner = 0, NoRace = 0,
    Federation = 1,
    Gorn = 2, Lizards = 2,
    Romulans = 3, Birdmen = 3,
    Klingons = 4,
    Orions = 5, Privateers = 5,
    Borg = 6, Cyborg = 6,
    Tholians = 7, Crystals = 7,
    Empire = 8,
    Cylons = 9, Robots = 9,
    Rebels = 10,
    Colonies = 11
  } RaceType_Def;

  typedef enum {
    NoNatives = 0,
    Humanoid = 1,
    Bovinoid = 2,
    Reptilian = 3,
    Avian = 4,
    Amorphous = 5,
    Insectoid = 6,
    Amphibian = 7,
    Ghipsoldal = 8,
    Siliconoid = 9,
    NumNativeTypes = 9
  } NativeType_Def;

  typedef enum {
    NoGovm = 0,
    Anarchy = 1,
    PreTribal = 2,
    EarlyTribal = 3,
    Tribal = 4,
    Feudal = 5,
    Monarchy = 6,
    Representative = 7,
    Participatory = 8,
    Unity = 9,
    NumGovmTypes = 9
  } NativeGovm_Def;

  typedef enum {
    NoMission = 0,
    Exploration = 1,
    MineSweep = 2,
    LayMines = 3,
    Kill = 4,
    SensorSweep = 5,
    Colonize = 6,
    Tow = 7,
    Intercept = 8,
    SpecialMission = 9,
    Cloak = 10,
    GatherNeutronium = 11,
    GatherDuranium = 12,
    GatherTritanium = 13,
    GatherMolybdenum = 14,
    GatherSupplies = 15,
    NumShipMissions = 15
  } ShipMission_Def;

  typedef enum {
    NoBaseMission = 0,
    Refuel = 1,
    MaxDefense = 2,
    LoadTorps = 3,
    UnloadFreighters = 4,
    RepairBase = 5,
    ForceSurrender = 6
  } BaseMission_Def;

  typedef enum {
    FIX_MISSION = 0,
    RECYCLE_MISSION = 1
  } BaseFixMission_Def;

  typedef enum {
    ENGINE_TECH = 0,
    HULL_TECH = 1,
    BEAM_TECH = 2,
    TORP_TECH = 3
  } BaseTech_Def;

  typedef struct {
    Uns16 mHull;
    Uns16 mEngineType;
    Uns16 mBeamType;
    Uns16 mNumBeams;
    Uns16 mTubeType;
    Uns16 mNumTubes;
  } BuildOrder_Struct;

  typedef struct {
    Uns16 mBase;
    BuildOrder_Struct mOrder;

    Boolean mByCloning;
    RaceType_Def mShipOwner;    /*!< In case of cloning */
    Uns32 mPriority;
    char mReserved[4];          /*!< DO NOT USE */
  } BaseOrder_Struct;

  typedef enum {
    ALLY_SHIPS = 0,
    ALLY_PLANETS = 1,
    ALLY_MINES = 2,
    ALLY_COMBAT = 3,
    ALLY_VISION = 4
  } AllianceLevel_Def;

  typedef enum {
    ALLY_STATE_OFF,
    ALLY_STATE_ON,
    ALLY_STATE_CONDITIONAL
  } AllianceState_Def;

  typedef enum {
    GAME_DIR_ONLY = 0x01,       /*!< search or write only in game
                                   subdirectory */
    ROOT_DIR_ONLY = 0x02,       /*!< search or write only in root
                                   subdirectory */
    GAME_OR_ROOT_DIR = 0x04,    /*!< search first in game dir, then in root
                                   dir */
    /*!< This flag only applies to input files */
    REWRITE_MODE = 0x10,        /*!< rewrite mode for update files */
    APPEND_MODE = 0x20,         /*!< append mode rather than write for
                                   output files */
    TEXT_MODE = 0x40,           /*!< open the file in text mode instead of
                                   binary */
    NO_MISSING_ERROR = 0x80     /*!< don't complain if input file is missing 
                                 */
  } FileLocation_Def;

  typedef enum {
    COMBAT_VICTOR,              /*!< The ship won the battle (i.e.,
                                   survived) */
    COMBAT_CAPTURED,            /*!< The ship was captured by the opponent */
    /*!< Doesn't apply to planets */
    COMBAT_DESTROYED,           /*!< The ship was destroyed */
    /*!< Applies to planets */
    COMBAT_NOAMMO               /*!< Ship has no beams, no torps, no
                                   fighters but has not been destroyed or
                                   captured */
  } CombatResult_Def;

  typedef enum {
    LANG_English,
    LANG_German,
    LANG_French,
    LANG_Spanish,
    LANG_Italian,
    LANG_Dutch,
    LANG_Russian,
    LANG_Estonian,
    NumLanguages
  } Language_Def;

  typedef enum {
    BQ_PAL,                     /* Good ol' PAL system */
    BQ_Fifo,                    /* Even older FIFO system */
    BQ_PBP,                     /* Degi's PBP system */
    BQ_LAST_ITEM    /* must be one bigger than last one */
  } BuildQueue_Def;

  typedef enum {
    UTIL_Ext,
    UTIL_Dat,
    UTIL_Tmp
  } UtilMode_Def;

  typedef enum {
    USED_POINTS = 0,
    UNUSED_POINTS,
    ALL_POINTS,
    THOST_POINTS
  } PointsType_Def;

  typedef enum {
    WAR_SHIPS = 0,
    FREIGHTER_SHIPS,
    ALL_SHIPS
  } ShipsType_Def;

  typedef enum {
    KILL_Ships = 0,
    KILL_Planets,
    KILL_Bases,
    KILL_Minefields,
    KILL_Structures,
    KILL_Cash_Supplies,
    NumKILL
  } PlayerKillType_Def;


#define GAME_NAME_SIZE  32  /* Maximum size of game names */
    
/** Host configuration. This structure contains all host configuration
    options (from the %PCONFIG section in pconfig.src). This structure
    is shared between PHost and PDK.

    Fields with 12 array members are generally indexed by player number,
    so index 0 is unused. */
typedef struct
{
    /* Host config data */
#define FirstConfigOption RecycleRate
    Uns16    RecycleRate,           /*!< % of ore recovered from colonizing ships */
             MeteorRate;            /*!< % chance of one large meteor */
    Boolean  Minefields,            /*!< allow minefields */
             Alchemy,               /*!< allow alchemy */
             DeleteOldMessages,     /*!< delete old messages -- NOT USED */
             DisablePasswd;         /*!< disable passwords */
    Uns16    GroundKill[12],        /*!< tim uses only 1..11, 0 ignored */
             GroundDefense[12],     /*!< ...ground attack, defense ratios */
             FreeFighters[12],      /*!< free fighters per starbase */
             MiningRate[12],        /*!< mineral mining rate */
             TaxRate[12];           /*!< taxation rate -- NO LONGER USED */
    Boolean  RebelsBuildFighters,   /*!< Rebels allowed to build fighters in space */
             ColoniesBuildFighters, /*!< Colonies allowed to build fighters in space */
             RobotsBuildFighters;   /*!< Robots allowed to build fighters in space */
    Uns16    CloakFailure;          /*!< % chance of cloaking failure */
    Boolean  RobCloakedShips;       /*!< Privateers can rob cloaked ships */
    Uns16    ScanRange[12],         /*!< Enemy ship scan range */
             DarksenseRange;        /*!< Dark sense range */
    Boolean  Hiss,                  /*!< Allow hiss mission */
             RebelGroundAttack,     /*!< Allow RGA */
             SuperRefit,            /*!< Allow super refit */
             WebMines;              /*!< Allow web mines */
    Uns16    CloakFuelBurn,         /*!< kt of fuel burn required to stay cloaked */
             SensorRange[12];       /*!< range of the sensor sweep mission */
    Boolean  NewNatives,            /*!< Allow new natives to appear */
             PlanetsAttack;         /*!< Allow planets to attack ships */
    Uns16    AssimilationRate,      /*!< assimilation percentage */
             WebDecay,              /*!< web decay rate percentage */
             MineDecay[12],         /*!< mines decay rate percentage */
             MaxMineRadius[12],     /*!< maximum minefield radius */
             TransuraniumRate,      /*!< new mineral formation rate */
             StructureDecay[12];    /*!< structure decay rate */
    Boolean  EatSupplies,           /*!< overpopulated planets eat supplies */
             NoFuelMove;            /*!< ships without fuel can move */
    Uns16    MineOdds[12],          /*!< percentage odds of hitting a mine */
             WebOdds[12],           /*!< percentage odds of hitting a web mine */
             MineScanRange[12];     /*!< range in which mines are visible */
    Boolean  MinesDestroyMines,     /*!< allow mines to destroy mines */
             EngShieldBonus;        /*!< allow engine bonus */
    Uns16    ShieldBonusRate,       /*!< engine tech bonus rate */
             FighterSweepRate[12];  /*!< Fighter sweep rate */
    Boolean  ColSweepWebs;          /*!< Allow Colonies to sweep web mines */
    Uns16    MineSweepRate[12],     /*!< rate of mine sweeping with beam weapons */
             WebSweepRate[12],      /*!< rate of web sweeping with beam weapons */
             HissEffect,            /*!< hiss effect per ship */
             RobFailRate;           /*!< percentage change that robbing fails */
    Boolean  PlanetRebelAttack,     /*!< planets can attack Rebels */
             PlanetKlingonAttack;   /*!< planets can attack Klingons */
    Uns16    MineSweepRange[12],    /*!< mine sweep range */
             WebSweepRange[12];     /*!< web sweep range */
    Boolean  ScienceShips;          /*!< allow science ship mission */
    Uns16    CloakMineOdds[12],     /*!< 10X percentage of cloaked hitting mine (13 = 1.3%) */
             NoCloakDamage;         /*!< amount of damage that prevents cloaking */
    Boolean  FedCrewBonus;          /*!< allow Fed combat bonus */

    Uns16    SmallMeteorOdds;       /*!< percentage chance per planet of small showers */
    Uns32    SmallOre[8];           /*!< Nmin, Dmin, Tmin, Mmin, Nmax, Dmax, Tmax, Mmax */
    Uns16    LargeMeteorNumber;     /*!< number of large meteors to impact */
    Uns32    LargeOre[8];           /*!< Nmin, Dmin, Tmin, Mmin, Nmax, Dmax, Tmax, Mmax */
    Boolean  MeteorMessages;        /*!< send meteor messages when they happen */

    Boolean  OneTow,                /*!< allow one engine ships to tow */
             HyperDrives;           /*!< allow hyperdrive */
    Uns16    ClimateDeathRate;      /*!< climate death rate */
    Boolean  GravityWells,          /*!< allow gravity wells */
             CrystalsLikeDesert;    /*!< Crystals grow better on hot planets */
    Boolean  DestroyWebs,           /*!< allow normal mines to destroy web mines */
             ClimateLimitsPopulation; /*!< allow climate to limit population */
    Uns32    MaxPlanetaryIncome;    /*!< max number of credits to earn per planet */
    Uns16    IonStormActivity;      /*!< chance of storms as a percentage */
    Boolean  AllowChunneling;       /*!< allow Firecloud chunneling */
    Boolean  AllowDeluxeSuperSpy;   /*!< allow Deluxe Super Spy mission */
    Boolean  IonStormsHideMines;    /*!< Ion storms hide minefields */
    Boolean  AllowGloryDevice;      /*!< allow Klingon glory device */
    Boolean  AllowAntiCloakShips;   /*!< allow Loki to uncloak ships */
    Boolean  AllowGamblingShips;    /*!< allow Lady Royale to generate MC */
    Boolean  AllowCloakedShipAttack;/*!< allow cloaked ships to attack based on PE */
    Boolean  AllowShipCloning;      /*!< allow ships to be cloned */
    Boolean  AllowBoardingParties;  /*!< allow Priv/Crystals to tow capture */
    Boolean  AllowImperialAssault;  /*!< allow SSD to perform imperial assault */
    Uns16    RamScoopFuelPerLY;     /*!< amount of fuel to gather per LY travelled */
    Boolean  AllowAdvancedRefinery; /*!< allow Aries to do refinery */
    Boolean  AllowBioscanners;      /*!< allow bioscanning */
    Uns16    HullTechNotSlowedByMines; /*!< min hull tech for ships not to be slowed */

    /* NEW OPTIONS */
#define FirstNewConfigOption UseAccurateFuelModel
    Boolean  UseAccurateFuelModel;  /*!< mass of ship decreases as fuel burns */
    Uns16    DefenseForUndetectable; /*!< min defenses to make planet undetectable */
    Uns16    FactoriesForDetectable; /*!< min factories to make planet detectable */
    Uns16    MinesForDetectable;     /*!< min mines to make planet detectable */
    Uns16    FighterSweepRange[12];  /*!< range at which fighters can sweep */
    Uns16    MineHitDamage;         /*!< damage per 100 KT of mass */
    Uns16    WebHitDamage;          /*!< damage per 100 KT of mass */
    Boolean  AllowRegisteredFunctions; /*!< allow registered-only functions to be performed */
    Uns16    GravityWellRange;      /*!< distance from planets at which ships fall in */
    Language_Def Language[12];      /*!< preferred language. Element 0 is for host */
    Boolean  AllowPlayerMessages;   /*!< allow player-to-player messages */
    Uns16    ScoringMethod;         /*!< the method of reporting scores */
    Boolean  TowedShipsBreakFree;   /*!< allow towed ships to break free */
    Uns16    NativeClimateDeathRate;/*!< climate death rate for natives */
    Boolean  AllowMoreThan50Targets[12]; /*!< allow target.dat part of .RST file to have >50 targets */
    Boolean  CrystalSinTempBehavior; /*!< growth and max pop for Crystals is sinusoidal */
    Boolean  RGANeedsBeams;         /*!< Need beam weapons to do RGA */
    Boolean  AllowRGAOnUnowned;     /*!< allow RGA on unowned planets */
    Boolean  CPEnableLanguage;      /*!< Enable 'language' command processor command */
    Boolean  CPEnableBigTargets;    /*!< Enable 'bigtargets' command */
    Boolean  CPEnableRaceName;      /*!< Enable 'racename' command */
    Boolean  CPEnableAllies;        /*!< Enable 'allies' command */
    Boolean  CPEnableMessage;       /*!< Enable 'message' command */
    Boolean  CPEnableRumor;         /*!< Enable 'rumor' command */
    Boolean  DelayAllianceCommands; /*!< Alliance management only after combat */
    Uns16    TerraformRate[12];     /*!< number of degrees of terraforming per turn */
    Uns16    MaxColTempSlope;       /*!< max colonists slope on arctic/desert planets */
    Uns16    WebDrainFuelLoss;      /*!< amount of fuel to drain from ships in web mines */
    Boolean  AllowWormholes;        /*!< allow wormholes */
    Uns16    WrmDisplacement;       /*!< magnitude of predictable endpoint travel */
    Uns16    WrmRandDisplacement;   /*!< magnitude of random endpoint travel */
    Int16    WrmStabilityAddX10;    /*!< amount of stability increase per turn X10 */
    Uns16    WrmRandStability;      /*!< magnitude of random stability to add */
    Int16    WrmMassAdd;            /*!< amount of mass to add per turn */
    Uns16    WrmRandMass;           /*!< magnitude of random mass to add */
    Boolean  WrmVoluntaryTravel;    /*!< require WRT friendly code for travel */
    Uns16    WrmTravelDistDivisor;  /*!< distance scaling for wormhole travel */
    Uns16    WrmTravelWarpSpeed;    /*!< speed at which ships must travel thru wormholes */
    Boolean  WrmTravelCloaked;      /*!< allow ships to remain cloaked thru wormholes */
    Uns16    WrmEntryPowerX100;     /*!< entry radius power of wormhole mass */
    Boolean  CPEnableGive;          /*!< allow the 'give' CP command */
    Boolean  AllowTowCloakedShips;  /*!< allow cloaked ships to be towed */
    Uns16    RobCloakedChance;      /*!< percent chance that rob cloaked succeeds */
    Uns16    PlanetaryTorpsPerTube; /*!< number of free torps to give per tube */
    Uns16    UnitsPerTorpRate[12];  /*!< percent of normal units-per-torp rate */
    Boolean  ESBonusAgainstPlanets; /*!< E-S bonus applies when fighting planets */
    Uns16    ShipCloneCostRate[12]; /*!< percentage cost of original ship */
    Boolean  HyperjumpGravityWells; /*!< allow hyperwarp ships to get sucked in */
    Uns16    NativeCombatSurvRate;  /*!< percent natives dying when planet loses combat */
    Boolean  AllowPrivTowCapture;    /*!< Allow Privs to do tow capture */
    Boolean  AllowCrystalTowCapture; /*!< Allow Crystals to do tow capture */
    char     GameName[GAME_NAME_SIZE]; /*!< The name of the game */
    Boolean  RoundWarpWells;        /*!< Warp wells are round, not square */
    Boolean  CPEnableSend;          /*!< Enable the 'send' command */
    Boolean  CPEnableJettison;      /*!< Enable the 'jettison' command */
    Boolean  CumulativePillaging;   /*!< Allow multiple ships to pillage */
    Boolean  AllowInterceptAttack;  /*!< Allow XA attack */
    Uns16    RaceGrowthRate[12];    /*!< Colonist growth factor */
    Uns16    PlayerRace[12];        /*!< Player-to-race map */
    Uns16    ProductionRate[12];    /*!< Supply production factor */
    Uns16    MineOddsWarpBonus[12]; /*!< Percent derate X100 for normal mines */
    Uns16    CloakMineOddsWarpBonus[12];/*!< Percent derate X100 for cloaked travel */
    Uns16    WebMineOddsWarpBonus[12];  /*!< Percent derate X100 for web mines */
    Uns16    MineTravelSafeWarp[12];/*!< Max. speed at which mine travel is safe */
    Uns16    CloakTravelSafeWarp[12];/*!< Max. speed at which cloak travel is safe */
    Uns16    WebMineTravelSafeWarp[12];/*!< Max. speed at which web mine travel is safe */
    Boolean  CloakFailMessages;     /*!< Allow sending of cloak fail messages */
    Boolean  TonsScoreCountsPlanets;/*!< Allow planets/bases to count in TONS.HST file */
    Uns16    ExtMissionsStartAt;    /*!< Starting value for extended missions */
    Uns16    WormholeUFOsStartAt;   /*!< Starting value for wormholes in UFO.DAT */
    Uns16    MaxShipsHissing;       /*!< Max ships Hissing at 1 planet */
    Boolean  AllowExtendedMissions; /*!< Allow access to extended missions */
    Uns16    SpyDetectionChance;    /*!< % chance of discovering Super Spy mission */
    Boolean  MapTruehullByPlayerRace; /*!< Use PlayerRace to interpret TRUEHULL.DAT */
    Boolean  AllowWraparoundMap;    /*!< Use wraparound */
    Uns16    WraparoundRectangle[4]; /*!< Vertices of wraparound rectangle */
    Uns16    Dummy1;                /*!< Old colonial fighter sweep rate */
    Uns16    Dummy2;                /*!< Old colonial fighter sweep range */
    Boolean  CPEnableRemote;        /*!< Allow the CP 'remote' command */
    Boolean  AllowAlliedChunneling; /*!< allow chunnel to +s allies */
    Uns16    ColTaxRate[12],        /*!< tax rate for colonists */
             NatTaxRate[12];        /*!< tax rate for natives */
    Boolean  AllowAlternativeTowing; /*!< Use alternative tow model */
    Boolean  AllowBeamUpClans;      /*!< Allow gathering clans from planets */
    Boolean  AllowBeamUpMultiple;   /*!< Allow beam up multiple extmission */
    Uns16    MaximumMinefieldsPerPlayer[12]; /*!< max # minefields per race */
    Boolean  MineIdNeedsPermission; /*!< True: need minefield alliance to lay minefield in other's Id */
    Uns16    DamageLevelForAntiCloakFail; /*!< Damage level at which Loki can't decloak */
    Uns16    DamageLevelForChunnelFail;   /*!< Damage level at which Firecloud chunnel no longer works */
    Uns16    DamageLevelForTerraformFail; /*!< Damage level at which terraforming fails */

    /* Build queue */
    BuildQueue_Def BuildQueue;       /*!< Build Queue Mode (BQ_xxx) */
    Boolean  AllowPriorityBuild;     /*!< Allow "PBx" Fcodes */
    Uns16    SBQBuildPALBoost[12];   /*!< Multiplier for existing build orders */
    Uns16    SBQNewBuildPALBoost[12];/*!< Multiplier for new build orders */
    Uns32    SBQPointsForAging[12];  /*!< Constant offset for old build orders */
    Uns32    SBQBuildChangePenalty[12]; /*!< Constant offset for build changes */
    Uns16    SBQBoostExpX100[12];    /*!< Exponent for build order boosts X 100 */
    Uns16    PBPCostPer100KT[12];    /*!< Cost for building a 100 kt hull */
    Uns16    PBPMinimumCost[12];     /*!< Minimum cost for building a ship */
    Uns16    PBPCloneCostRate[12];   /*!< Cost scaling for clone orders (%) */
    Uns16    PALDecayPerTurn[12];    /*!< Exponential decay term */
    Uns16    PALPlayerRate[12];      /*!< Player-dependent scaling */
    Uns16    PALCombatAggressor[12]; /*!< Points for being aggressor in combat */
    Uns16    PALAggressorPointsPer10KT[12]; /*!< Points for mass destroyed in combat */
    Uns16    PALOpponentPointsPer10KT[12];  /*!< As above, for opponent */
    Uns16    PALAggressorKillPointsPer10KT[12]; /*!< Bonus for complete destruction */
    Uns16    PALOpponentKillPointsPer10KT[12]; /*!< As above, for opponent */
    Uns16    PALCombatPlanetScaling[12];    /*!< Scale factor for above */
    Uns16    PALCombatBaseScaling[12];      /*!< Scale factor for above */
    Uns16    PALShipCapturePer10Crew[12];   /*!< Points for killing crew */
    Uns16    PALRecyclingPer10KT[12];       /*!< Points for recycling/colonizing */
    Uns16    PALBoardingPartyPer10Crew[12];   /*!< Priv/Crystal boarding party */
    Uns16    PALGroundAttackPer100Clans[12]; /*!< As it says */
    Uns16    PALGloryDevice[12];            /*!< Points for exploding a G.D. */
    Uns16    PALGloryDamagePer10KT[12];     /*!< Points for enemy ships */
    Uns16    PALImperialAssault[12];        /*!< Points for performing one */
    Uns16    PALRGA[12];                    /*!< Points for performing one */
    Uns16    PALPillage[12];                /*!< Points for performing one */

    Boolean  FilterPlayerMessages[12];      /*!< Elide messages for which there are
                                               UTIL.DAT equivalents? */
    Boolean  AlternativeAntiCloak;          /*!< Loki's don't decloak own race */
    Boolean  AntiCloakImmunity[12];         /*!< New HOST 3.22.022 option */
    Boolean  AllowMoreThan500Minefields[12]; /*!< allow receipt of more than 500 minefields */
    Uns16    CPNumMinefields;                /*!< total number of minefields */
    Uns16    PALShipMinekillPer10KT[12];     /*!< PAL for destroying ship by minehit */

    /* Combat-related Options -- always leave these at the end */

#define FirstCombatConfigOption BayRechargeRate
    /* Player-specific parameters */
    Uns16    BayRechargeRate[12];       /*!< recharge rate X10 of fighter bays per bay */
    Int16    BayRechargeBonus[12];      /*!< bay number bonus in recharging */
    Uns16    BeamRechargeRate[12];      /*!< recharge rate X10 of beams */
    Int16    BeamRechargeBonus[12];     /*!< beam tech bonus in recharging */
    Uns16    TubeRechargeRate[12];      /*!< recharge rate X10 of tubes */
    Uns16    BeamHitFighterCharge[12];  /*!< minimum charge X10 for beams to fire at fighters */
    Uns16    BeamHitShipCharge[12];     /*!< minimum charge X10 for beams to fire at ships */
    Uns32    TorpFiringRange[12];       /*!< maximum distance at which to fire torps */
    Uns32    BeamFiringRange[12];       /*!< maximum distance at which to fire beams */
    Uns16    TorpHitOdds[12];           /*!< percentage chance of torps hitting */
    Uns16    BeamHitOdds[12];           /*!< percentage chance of beams hitting */
    Int16    BeamHitBonus[12];          /*!< bonus factor for beam hit odds */
    Uns16    StrikesPerFighter[12];     /*!< number of strikes per fighter */
    Uns16    FighterKillOdds[12];       /*!< percent chance that a fighter will destroy another fighter */
    Uns16    FighterBeamExplosive[12];  /*!< damage done by each fighter */
    Uns16    FighterBeamKill[12];       /*!< kill power of each fighter */
    Uns16    ShipMovementSpeed[12];     /*!< ship closing speed in meters/tick */
    Uns16    FighterMovementSpeed[12];  /*!< fighter closing speed in meters/tick */
    Uns16    BayLaunchInterval[12];     /*!< minimum number of ticks between launches */
    Uns16    MaxFightersLaunched[12];   /*!< max number of fighters out of ship */
    Int16    TorpHitBonus[12];          /*!< bonus factor for torp hit odds */
    Int16    TubeRechargeBonus[12];     /*!< tube tech bonus in recharging */
    Uns16    ShieldDamageScaling[12];   /*!< relative damage to shields */
    Uns16    HullDamageScaling[12];     /*!< relative damage to hull */
    Uns16    CrewKillScaling[12];       /*!< relative damage to crew */
    Int16    ExtraFighterBays[12];      /*!< extra bays for carriers */
    Uns32    BeamHitFighterRange[12];   /*!< distance at which beams can hit fighters */
    Uns32    FighterFiringRange[12];    /*!< distance from which fighters can strike */

    /* Common combat parameters. Note that changing a common parameter into
       an arrayized one (or vice-versa) has implications in PVCR.C and
       BATTLE.C */
    Boolean  AlternativeCombat;     /*!< allow fractional damage, etc. */
    Uns32    StandoffDistance;      /*!< minimum inter-ship distance */
    Boolean  PlanetsHaveTubes;      /*!< allow planets to fire torps */
    Boolean  FireOnAttackFighters;  /*!< fire first on attack fighters, then retreaters */
}
Pconfig_Struct;

/* Historically, the names of the Pconfig_Struct members differed
   from the ones used in pconfig.src. The following #defines
   make it possible to use the pconfig.src name everywhere (recommended!). */
    
/*      new name (valid in pconfig.src) old name (valid in PDK)  */
#define AllowAlchemy                    Alchemy
#define AllowAlternativeCombat          AlternativeCombat
#define AllowCloakedShipsAttack         AllowCloakedShipAttack
#define AllowCloakFailMessages          CloakFailMessages
#define AllowColoniesSweepWebs          ColSweepWebs
#define AllowEatingSupplies             EatSupplies
#define AllowEngineShieldBonus          EngShieldBonus
#define AllowESBonusAgainstPlanets      ESBonusAgainstPlanets
#define AllowFedCombatBonus             FedCrewBonus
#define AllowGravityWells               GravityWells
#define AllowHiss                       Hiss
#define AllowHyperjumpGravWells         HyperjumpGravityWells
#define AllowHyperWarps                 HyperDrives
#define AllowMeteorMessages             MeteorMessages
#define AllowMinefields                 Minefields
#define AllowMinesDestroyMines          MinesDestroyMines
#define AllowMinesDestroyWebs           DestroyWebs
#define AllowNewNatives                 NewNatives
#define AllowNoFuelMovement             NoFuelMove
#define AllowOneEngineTowing            OneTow
#define AllowPlanetAttacks              PlanetsAttack
#define AllowPrivateerTowCapture        AllowPrivTowCapture
#define AllowRebelGroundAttack          RebelGroundAttack
#define AllowScienceMissions            ScienceShips
#define AllowSuperRefit                 SuperRefit
#define AllowWebMines                   WebMines
#define BorgAssimilationRate            AssimilationRate
#define CloakFailureRate                CloakFailure
#define LargeMeteorOreRanges            LargeOre
#define ColonistTaxRate                 ColTaxRate
#define CrystalsPreferDeserts           CrystalsLikeDesert
#define DarkSenseRange                  DarksenseRange
#define DisablePasswords                DisablePasswd
#define GroundDefenseFactor             GroundDefense
#define GroundKillFactor                GroundKill
#define HissEffectRate                  HissEffect
#define MaximumMinefieldRadius          MaxMineRadius
#define MineHitDamageFor100KT           MineHitDamage
#define MineHitOdds                     MineOdds
#define PlanetsAttackKlingons           PlanetKlingonAttack
#define PlanetsAttackRebels             PlanetRebelAttack
#define MineOddsWarpBonusX100           MineOddsWarpBonus
#define NativeCombatSurvivalRate        NativeCombatSurvRate
#define NativeTaxRate                   NatTaxRate
#define RobFailureOdds                  RobFailRate
#define RoundGravityWells               RoundWarpWells
#define TransuraniumDecayRate           TransuraniumRate
#define WebHitDamageFor100KT            WebHitDamage
#define WebMineDecayRate                WebDecay
#define WebMineHitOdds                  WebOdds
#define WebMineOddsWarpBonusX100        WebMineOddsWarpBonus
#define WebMineSweepRange               WebSweepRange
#define WebMineSweepRate                WebSweepRate
#define StructureDecayPerTurn           StructureDecay
#define MineDecayRate                   MineDecay
#define DamageLevelForCloakFail         NoCloakDamage
#define EngineShieldBonusRate           ShieldBonusRate
#define RaceMiningRate                  MiningRate
#define CloakedMineTravelSafeWarp       CloakTravelSafeWarp
#define LargeMeteorsImpacting           LargeMeteorNumber
#define MeteorShowerOdds                SmallMeteorOdds
#define MeteorShowerOreRanges           SmallOre
#define RandomMeteorRate                MeteorRate
#define CloakMineOddsWarpBonusX100      CloakMineOddsWarpBonus
#define MineHitOddsWhenCloakedX10       CloakMineOdds

/*! This record describes an entry in the UFO.HST file */

  typedef struct {
    Uns16 mColor;               /*!< 0==no object, 1-15 == color */
    char mName[20];
    char mInfo1[20];
    char mInfo2[20];
    Uns16 mX;
    Uns16 mY;
    Uns16 mSpeed;               /*!< range is 0 to 15 */
    Int16 mHeading;             /*!< 0 to 360, and possibly -1 */
    Uns16 mPRange;              /*!< range at which UFO can be seen from a
                                   planet */
    Uns16 mSRange;              /*!< range at which UFO can be seen from a
                                   ship */
    Uns16 mRadius;              /*!< the radius of the UFO object */
    Uns16 mParentID;            /*!< ID of the program that maintains this
                                   UFO */
  } UFO_Struct;

/*
 * Enumeration functions
 */

  extern Uns16 *EnumerateShipsWithinRadius(Int16 pX, Int16 pY,
        double pRadius);
  extern Uns16 *EnumerateShipsAt(Uns16 pX, Uns16 pY);
  extern Uns16 *EnumerateMinesWithinRadius(Int16 pX, Int16 pY, Uns16 pRadius);
  extern Uns16 FindPlanetAtShip(Uns16 lShip);
  extern Uns16 FindGravityPlanet(Int16 pX, Int16 pY);
  extern Uns16 *EnumeratePlanetsWithin(Int16 pX, Int16 pY, Uns16 pRadius);
  extern Uns16 *EnumerateShipsAtPlanet(Uns16 pPlanet);
  extern Uns16 *EnumerateMinesCovering(Int16 pX, Int16 pY);
  extern Uns16 *EnumerateOverlappingMines(Uns16 pMinefield);

/*
 * Interface to host messaging
 */

  extern Boolean WriteExternalMessage(RaceType_Def pRace,
        const char *pMessage);
  extern Boolean WriteAUXHOSTMessage(RaceType_Def pRace,
        const char *pMessage);
  extern Boolean WriteMessageToRST(RaceType_Def pRace, int pArgc,
        const char *pArgv[]);

/*
 *  Reading and writing of PHOST data files
 */

/*! Multiple-read routines (single-read routines all in a row) */
  extern Boolean ReadGlobalData(void);
  extern Boolean ReadHostData(void);

/*! Multiple-write routine (single-write routines all in a row) */
  extern Boolean WriteHostData(void);

  extern Boolean IsCLOAKCFound(void);

/*! Single-read routines */
  extern IO_Def Read_AuxData_File(void);
  extern IO_Def Read_Turntime_File(void);
  extern IO_Def Read_HConfig_File(void);
  extern IO_Def Read_THost_HConfig_File(void);
  extern IO_Def Read_Planets_File(Int16 * pControl);
  extern IO_Def Read_Ships_File(Int16 * pControl);
  extern IO_Def Read_Bases_File(Int16 * pControl1, Int16 * pControl2);
  extern IO_Def Read_Mines_File(void);
  extern IO_Def Read_Hullspec_File(void);
  extern IO_Def Read_Truehull_File(void);
  extern IO_Def Read_Engspec_File(void);
  extern IO_Def Read_Torpspec_File(void);
  extern IO_Def Read_Beamspec_File(void);
  extern IO_Def Read_Planetname_File(void);
  extern IO_Def Read_Racenames_File(void);
  extern IO_Def Read_Xyplan_File(void);
  extern IO_Def Read_HostGen_File(void);

/*! Single-write routines */
  extern Boolean Write_Planets_File(Int16 pControl);
  extern Boolean Write_Ships_File(Int16 pControl);
  extern Boolean Write_Bases_File(Int16 pControl1, Int16 pControl2);
  extern Boolean Write_Mines_File(void);
  extern Boolean Write_HostGen_File(void);
  extern Boolean Write_AuxData_File(void);
  extern Boolean Write_Racenames_File(void);
  extern Boolean WriteWormholeFile(void);
  extern Boolean WriteCLOAKCFile(void);
  extern Boolean Write_Hullspec_File(void);
  extern Boolean Write_Truehull_File(void);
  extern Boolean Write_Engspec_File(void);
  extern Boolean Write_Torpspec_File(void);
  extern Boolean Write_Beamspec_File(void);
  extern Boolean Write_Planetname_File(void);
  extern Boolean Write_Xyplan_File(void);
  
  
/*
 *  Initializing dynamic files (optional routines in place of reading from disk)
 */

  extern void InitializePlanets(void);
  extern void InitializeShips(void);
  extern void InitializeBases(void);
  extern void InitializeMines(void);
  extern void InitializeHostGen(void);

/*
 *  Player status and kill race functions
 */

  extern Boolean PlayerIsActive(RaceType_Def pPlayer);
  extern void PutPlayerIsActive(RaceType_Def pPlayer, Boolean pActive);
  extern const char *PlayerPassword(RaceType_Def pPlayer);
  extern void PutPlayerPassword(RaceType_Def pPlayer, const char *pPasswd);
  extern Boolean KillPlayerConfigure(PlayerKillType_Def lKillConf);
  extern void PutKillPlayerConfigure(PlayerKillType_Def lKillConf, Boolean lValue);
  extern void KillPlayerPlanets(RaceType_Def Race);
  extern void KillPlayerShips(RaceType_Def Race);
  extern void KillPlayer(RaceType_Def Race);
  extern void KillPlayerBases(RaceType_Def Race);
  extern void KillPlayerMinefields(RaceType_Def Race);
  extern Uns16 EffRace(Uns16 pPlayer);

/*
 *  Build Queue Interface
 */

  extern Uns16 BuildQueuePeek(BaseOrder_Struct * pOrder);
  extern BaseOrder_Struct * BuildQueueAccess(Uns16 pMember);
  extern void BuildQueuePush(BaseOrder_Struct * pOrderPtr);
  extern void BuildQueueInvalidate(Uns16 pBase);
  extern Uns16 ShipsInBuildQueue(void);
  extern void SortBuildQueue(void);
  extern Uns16 BuildQueueBaseID(Uns16 pPosition);
  extern Uns32 BuildQueuePriority(Uns16 pPosition);
  extern Boolean BuildQueueIsCloning(Uns16 pPosition);
  extern RaceType_Def BuildQueueShipOwner(Uns16 pPosition);
  extern RaceType_Def BuildQueueOwner(Uns16 pPosition);
  extern Uns16 BuildQueueHullType(Uns16 pPosition);
  extern Uns16 BuildQueueEngineType(Uns16 pPosition);
  extern Uns16 BuildQueueBeamType(Uns16 pPosition);
  extern Uns16 BuildQueueTorpType(Uns16 pPosition);
  extern Uns16 BuildQueueBeamNumber(Uns16 pPosition);
  extern Uns16 BuildQueueTorpNumber(Uns16 pPosition);
  extern void PutBuildQueuePriority(Uns16 pPosition, Uns32 pPriority);

/*
 *       Activity Level
 */
  extern Uns32 GetActivityLevel(Uns16 pRace);
  extern void SetActivityLevel(Uns16 pRace, Uns32 pNewLevel);

/*
 *   Alliance interface
 */

  extern void AllyAddRequest(RaceType_Def pPlayer, RaceType_Def pRace);
  extern void AllyDropRequest(RaceType_Def pPlayer, RaceType_Def pRace);
  extern Boolean PlayerAllowsAlly(RaceType_Def pPlayer, RaceType_Def pRace,
        AllianceLevel_Def pLevel);
  extern void PutPlayerAllowsAlly(RaceType_Def pPlayer, RaceType_Def pRace,
        AllianceLevel_Def pLevel, AllianceState_Def pState);
  extern Boolean PlayerOffersAlliance(RaceType_Def pPlayer,
        RaceType_Def pRace);
  extern Boolean PlayersAreAllies(RaceType_Def pPlayer, RaceType_Def pRace);
  extern Boolean IsAllyOfferConditional(RaceType_Def pPlayer,
        RaceType_Def pRace, AllianceLevel_Def pLevel);
  extern Boolean IsAllyOfferAny(RaceType_Def pPlayer, RaceType_Def pRace,
        AllianceLevel_Def pLevel);

/*
 *   Combat Interface
 */

/*! This structure defines the data required by the combat module for each
   ship/planet battle. It consists of 3 sections. The first section
   contains variables which must be set before calling Combat() to perform
   the battle. These variable will not be changed by Combat(). The second
   section contains variables that must be set before calling Combat() and
   may also be changed during the course of battle. For example, NumTubes
   must be known by Combat() but it is not something that changes. The
   Shields variable, however, not only needs to be known at the start of
   combat, but may also change during combat. The third section contains
   variables that are written to by Combat() but their initial state is
   ignored. That is, the code calling Combat() need not fill in the
   values of the variables in this section. */

  typedef struct {
    /*! The following fields will not be changed by Combat() They must be
       filled in with appropriate values by the code calling Combat(). */

    Boolean IsPlanet;           /*!< True if this is a planet, not a ship */
    Uns16 Mass;                 /*!< effective combat mass (must include all
                                   bonuses, base contribution, etc.) */
    RaceType_Def Race;          /*!< ship's owner */
    const char *Name;           /*!< Ship/Planet name */
    Uns16 BeamTech;             /*!< tech level of beams from 0 to 10 */
    Uns16 NumBeams;             /*!< number of beams */
    Uns16 TorpTech;             /*!< tech level of torps from 0 to 10 */
    Uns16 NumTubes;             /*!< number of torp launchers */
    Uns16 NumBays;              /*!< number of fighter bays */

    /*! The following fields WILL be changed by Combat(). Initial values for 
       these fields must be filled in before calling Combat(). */

    Uns16 Crew;                 /*!< Number of crew members (irrelevant for
                                   planets) */
    Uns16 Shields;              /*!< Shield strength from 0 to 100 */
    Uns16 Damage;               /*!< Ship's damage from 0 to 100 */
    Uns16 NumFighters;          /*!< Number of fighters */
    Uns16 NumTorps;             /*!< Number of torps */

    /*! The following fields will be filled in by Combat(). It is not
       necessary to fill them in prior to calling Combat(). */

    CombatResult_Def Outcome;   /*!< What happened to this ship/planet */

  } Combat_Struct;

  extern void Combat(Combat_Struct * pShip1, Combat_Struct * pShip2);

/*! If the Combat() routine is called by user code, then the following
   functions must also be defined in the user's code.

   NOTE: THESE ROUTINES ARE NOT DEFINED BY THE PDK!!! YOU MUST WRITE
         THEM YOURSELF!
*/

  extern void DrawInitBeams(Uns16 pShip, Uns16 pNumBeams, Uns16 pCharge);
  extern void DrawInitTubes(Uns16 pShip, Uns16 pNumTubes, Uns16 pCharge);
  extern void DrawBeamRecharge(Uns16 pShip, Uns16 pBeam, Uns16 pCharge);
  extern void DrawTubeRecharge(Uns16 pShip, Uns16 pTube, Uns16 pCharge);
  extern void DrawBayRecharge(Uns16 pShip, Uns16 pBay, Uns16 pCharge);
  extern void DrawFighterAttack(Uns16 pShip, Uns16 pFighter, Int32 pPos);
  extern void DrawEraseFighterAttack(Uns16 pShip, Uns16 pFighter, Int32 pPos);
  extern void DrawFighterReturn(Uns16 pShip, Uns16 pFighter, Int32 pPos);
  extern void DrawEraseFighterReturn(Uns16 pShip, Uns16 pFighter, Int32 pPos);
  extern void DrawShipShields(Uns16 pShip, Int32 pPos, double pShields);
  extern void DrawShipDamage(Uns16 pShip, double pDamage);
  extern void DrawShipCaptured(Uns16 pShip);
  extern void DrawShipDestroyed(Uns16 pShip, Int32 pPos, Boolean pIsPlanet);
  extern void DrawStalemateResult(Boolean pIsPlanet2);
  extern void DrawShipCrew(Uns16 pShip, double pCrew);
  extern void DrawTorpNumber(Uns16 pShip, Uns16 pTorps);
  extern void DrawFighterNumber(Uns16 pShip, Uns16 pFighters);
  extern void DrawFighterHit(Uns16 pShip, Uns16 pFighter, Int32 pPos,
        Int32 pEnemyPos);
  extern void DrawFighterHitFighter(Int32 pPos, Uns16 pFighter1,
        Uns16 pFighter2);
  extern void DrawTorpHit(Uns16 pShip, Int32 pPos, Int32 pEnemyPos);
  extern void DrawTorpMiss(Uns16 pShip, Int32 pPos, Int32 pEnemyPos);
  extern void DrawBeamHitFighter(Uns16 pShip, Uns16 pBeam, Int32 pPos,
        Uns16 pFighter, Int32 pEnemyFighterPos);
  extern void DrawBeamMissedFighter(Uns16 pShip, Uns16 pBeam, Int32 pPos,
        Uns16 pFighter, Int32 pEnemyFighterPos);
  extern void DrawBeamHitShip(Uns16 pShip, Int32 pPos, Int32 pEnemyPos);
  extern void DrawBeamMissedShip(Uns16 pShip, Int32 pPos, Int32 pEnemyPos);
  extern void DrawEraseShip(Uns16 pShip, Int32 pPos);
  extern void DrawShip(Uns16 pShip, Int32 pPos);
  extern void DrawDistance(Int32 pDist);
  extern void DrawBackground(void);
  extern void DrawBeamName(Uns16 pShip, const char *pName);
  extern void DrawTubeName(Uns16 pShip, const char *pName);
  extern void DrawShipOwner(Uns16 pShip, const char *pRaceName,
        Boolean pIsPlanet);
  extern void DrawShipName(Uns16 pShip, const char *pName);

/*
 *   Utility functions
 */

/*! File handling */
  extern FILE *OpenInputFile(const char *pName, int pLocation);
  extern FILE *OpenOutputFile(const char *pName, int pLocation);
  extern FILE *OpenUpdateFile(const char *pName, int pLocation);
  extern Boolean RemoveGameFile(const char *pName);
  extern Boolean CopyFileToFile(FILE * pSrc, FILE * pDst, Uns32 pSize);
  extern Boolean CopyGameFile(const char *pSrcName, const char *pDstName);
  extern Boolean RenameGameFile(const char *pSrcName, const char *pDstName);
  extern Uns32 FileLength(FILE * lFile);

/*! Memory allocation */
  extern void *MemAlloc(size_t pAmount);
  extern void *MemCalloc(size_t pNumber, size_t pSize);
  extern void *MemRealloc(void *pPtr, size_t pAmount);
  extern void MemFree(void *pPtr);

/*! Reporting */
  extern void Error(const char *pStr, ...);
  extern void VError(const char *pStr, va_list pAP);
  extern void ErrorExit(const char *pStr, ...);
  extern void VErrorExit(const char *pStr, va_list pAP);
  extern void Warning(const char *pStr, ...);
  extern void VWarning(const char *pStr, va_list pAP);
  extern void Info(const char *pStr, ...);
  extern void VInfo(const char *pStr, va_list pAP);
  extern void AssertFail(const char *pExpr, const char *pFile, int pLine);
#ifdef NDEBUG
#define passert(p) ((void)0)
#else
#define passert(p) ((p) ? ((void)0) : AssertFail(0, __FILE__, __LINE__))
#endif

/*! Random numbers */
  extern void SetRandomSeed(Uns16 seed);
  extern RandType_Def RandomRange(RandType_Def pRange);
  extern double RandomReal(void);

/*! Cross-platform portability */
  extern void WordSwapLong(void *pData, Uns16 pNumWords);
  extern void WordSwapShort(void *pData, Uns16 pNumWords);
  extern Boolean DOSRead16(Uns16 * pData, Uns16 pNum, FILE * pInFile);
  extern Boolean DOSRead32(Uns32 * pData, Uns16 pNum, FILE * pInFile);
  extern Boolean DOSWrite16(const Uns16 * pData, Uns16 pNum, FILE * pOutFile);
  extern Boolean DOSWrite32(const Uns32 * pData, Uns16 pNum, FILE * pOutFile);

/*! Command-line option processing */
  extern int pgetopt(int argc, char *argv[], char *optstring);

/*! Configuration file processing */
  typedef Boolean (*configAssignment_Func) (const char *lhs, char *rhs,
        const char *lInputLine);
  typedef void (*configError_Func) (const char*, ...);
    
  extern IO_Def ConfigFileReader(FILE * pInFile, const char *pFileName,
        const char *pSection, Boolean pUseDefaultSection,
        configAssignment_Func pAssignFunc);
  extern IO_Def ConfigFileReaderEx(FILE * pInFile, const char *pFileName, const char *pSection,
        Boolean pUseDefaultSection, configAssignment_Func pAssignFunc,
        configError_Func pError, Boolean pContinue);

/*! Wraparound Maps */
  extern Int32 DistanceSquared(Int16 pX1, Int16 pY1, Int16 pX2, Int16 pY2);
  extern double Distance(Int16 pX1, Int16 pY1, Int16 pX2, Int16 pY2);
  extern Boolean IsDistanceLTRadius(Int16 pX1, Int16 pY1, Int16 pX2,
        Int16 pY2, Uns32 pRadius);
  extern Int16 WrapMapX(Int16 pX);
  extern Int16 WrapMapY(Int16 pY);
  extern Int16 WrapDistX(Int16 pX);
  extern Int16 WrapDistY(Int16 pY);
  extern Boolean IsPointOnWrapMap(Int16 pX, Int16 pY);
  extern void RewrapShipsAndMines(void);

/*
 *     Remote control
 */

  extern Boolean BeginShipRemoteControl(Uns16 pShipID, Uns16 pRemoteOwner);
  extern Boolean EndShipRemoteControl(Uns16 pShipID, Uns16 pRemoteOwner);
  extern Boolean IsShipRemoteControlled(Uns16 pShip);
  extern Boolean IsShipRemoteForbidden(Uns16 pShip);
  extern void ResetShipRemoteControl(Uns16 pShip);
  extern Boolean AllowShipRemoteControl(Uns16 pShipID, Uns16 pOrigOwner);
  extern Boolean ForbidShipRemoteControl(Uns16 pShipID, Uns16 pOrigOwner);
  extern Uns16 ShipRemoteController(Uns16 pShipID);
  extern Uns16 ShipRemoteOwner(Uns16 pShipID);

/*
 *  Global variables
 */

  extern const char *gRootDirectory;
  extern const char *gGameDirectory;
  extern FILE *gLogFile;
  extern Boolean gNewlyMastered;
  extern Boolean gNonPHOSTWarnings;
  extern Pconfig_Struct *gPconfigInfo;
  extern char *poptarg;
  extern int poptind;
  extern Boolean gUsingTHost;

/*
 *  Initialization and cleanup
 */

  extern void InitPHOSTLib(void);
  extern void FreePHOSTLib(void);

/*
 *  Ship functions
 */

  extern Boolean IsShipExist(Uns16 pID);
  extern void DeleteShip(Uns16 pID);
  extern RaceType_Def ShipOwner(Uns16 pID);
  extern Uns16 ShipSpeed(Uns16 pID);
  extern Uns16 ShipLocationX(Uns16 pID);
  extern Uns16 ShipLocationY(Uns16 pID);
  extern Uns16 ShipWaypointX(Uns16 pID);
  extern Uns16 ShipWaypointY(Uns16 pID);
  extern Int16 ShipRelWaypointX(Uns16 pID);
  extern Int16 ShipRelWaypointY(Uns16 pID);
  extern Uns16 ShipHull(Uns16 pID);
  extern Uns16 ShipEngine(Uns16 pID);
  extern Uns16 ShipBeamType(Uns16 pID);
  extern Uns16 ShipBeamNumber(Uns16 pID);
  extern Uns16 ShipTorpType(Uns16 pID);
  extern Uns16 ShipTubeNumber(Uns16 pID);
  extern Uns16 ShipBays(Uns16 pID);
  extern Uns16 ShipAmmunition(Uns16 pID);
  extern ShipMission_Def ShipMission(Uns16 pID);
  extern RaceType_Def ShipEnemy(Uns16 pID);
  extern Uns16 ShipTowTarget(Uns16 pID);
  extern Uns16 ShipInterceptTarget(Uns16 pID);
  extern Uns16 ShipDamage(Uns16 pID);
  extern Uns16 ShipCrew(Uns16 pID);
  extern Uns16 ShipDump(Uns16 pID, CargoType_Def pType);
  extern Uns16 ShipDumpPlanet(Uns16 pID);
  extern Uns16 ShipTransfer(Uns16 pID, CargoType_Def pType);
  extern Uns16 ShipTransferShip(Uns16 pID);
  extern char *ShipName(Uns16 pID, char *pName);
  extern const char *ShipFC(Uns16 pID, char *pFCode);
#define ShipFCode ShipFC
  extern Uns16 ShipCargo(Uns16 pID, CargoType_Def pType);
  extern Uns16 CreateShip(RaceType_Def pOwner);
  extern Uns16 ShipTotalMass(Uns16 sID);
  extern Uns16 ShipTravelMass(Uns16 sID);
  extern Uns16 ShipCargoMass(Uns16 sID);
  extern char *ShipMissionString(Uns16 sID, char *pBuffer);
  /* extern Boolean IsShipFCSpecial(Uns16 sID); */
#define IsShipFCSpecial IsShipFCodeSpecial
  extern void PutShipCargo(Uns16 pID, CargoType_Def pType, Uns16 pCargo);
  extern void PutShipName(Uns16 pID, const char *pName);
  extern void PutShipOwner(Uns16 pID, RaceType_Def pOwner);
  extern void PutShipFC(Uns16 pID, char *pFCode);
#define PutShipFCode PutShipFC
  extern void PutShipSpeed(Uns16 pID, Uns16 pSpeed);
  extern void PutShipWaypointX(Uns16 pID, Uns16 pWaypoint);
  extern void PutShipWaypointY(Uns16 pID, Uns16 pWaypoint);
  extern void PutShipRelWaypointX(Uns16 pID, Int16 pRel);
  extern void PutShipRelWaypointY(Uns16 pID, Int16 pRel);
  extern void PutShipLocationX(Uns16 pID, Int16 pLocation);
  extern void PutShipLocationY(Uns16 pID, Int16 pLocation);
  extern void PutShipHull(Uns16 pID, Uns16 pHull);
  extern void PutShipEngine(Uns16 pID, Uns16 pEngine);
  extern void PutShipBeamType(Uns16 pID, Uns16 pBeamType);
  extern void PutShipBeamNumber(Uns16 pID, Uns16 pBeamNumber);
  extern void PutShipTorpType(Uns16 pID, Uns16 pTorpType);
  extern void PutShipTubeNumber(Uns16 pID, Uns16 pTubeNumber);
  extern void PutShipBays(Uns16 pID, Uns16 pBays);
  extern void PutShipAmmunition(Uns16 pID, Uns16 pAmmunition);
  extern void PutShipMission(Uns16 pID, ShipMission_Def pMission);
  extern void PutShipEnemy(Uns16 pID, RaceType_Def pEnemy);
  extern void PutShipTowTarget(Uns16 pID, Uns16 pTarget);
  extern void PutShipInterceptTarget(Uns16 pID, Uns16 pTarget);
  extern void PutShipDamage(Uns16 pID, Uns16 pDamage);
  extern void PutShipCrew(Uns16 pID, Uns16 pCrew);
  extern void PutShipDump(Uns16 pID, CargoType_Def pType, Uns16 pCargo);
  extern void PutShipDumpPlanet(Uns16 pID, Uns16 pPlanet);
  extern void PutShipTransfer(Uns16 pID, CargoType_Def pType, Uns16 pCargo);
  extern void PutShipTransferShip(Uns16 pID, Uns16 pShip);
  extern Boolean IsShipCloaked(Uns16 pID);
  extern void MarkShipCloaking(Uns16 pID, Boolean pCloaked);
  extern Uns16 ShipCombatShieldLevel(Uns16 pID);
  extern void SetShipCombatShieldLevel(Uns16 pID, Uns16 pShields);

/*
 *   Planet functions
 */

  extern Boolean IsPlanetExist(Uns16 pID);
  extern Uns16 PlanetLocationX(Uns16 pID);
  extern Uns16 PlanetLocationY(Uns16 pID);
  extern RaceType_Def PlanetOwner(Uns16 pID);
  extern Uns16 PlanetMines(Uns16 pID);
  extern Uns16 PlanetFactories(Uns16 pID);
  extern Uns16 PlanetDefense(Uns16 pID);
  extern Uns32 PlanetCargo(Uns16 pID, CargoType_Def pType);
  extern Uns32 PlanetCore(Uns16 pID, CargoType_Def pType);
  extern Uns16 PlanetDensity(Uns16 pID, CargoType_Def pType);
  extern Uns16 PlanetColTax(Uns16 pID);
  extern Uns16 PlanetNatTax(Uns16 pID);
  extern Int16 PlanetColHappy(Uns16 pID);
  extern Int16 PlanetNatHappy(Uns16 pID);
  extern NativeType_Def PlanetNatType(Uns16 pID);
  extern NativeGovm_Def PlanetNatGovm(Uns16 pID);
  extern Uns32 PlanetNativePopulation(Uns16 pID);
  extern Uns16 PlanetTemp(Uns16 pID);
  extern Boolean PlanetBuildBase(Uns16 pID);
  extern Boolean PlanetHasNatives(Uns16 pPlanet);
  /* extern Boolean IsPlanetFCSpecial(Uns16 pID); */
#define IsPlanetFCSpecial IsPlanetFCodeSpecial
  extern char *PlanetNatString(Uns16 pID, char *pBuffer);
  extern char *PlanetNatGovmString(int pID, char *pBuffer);
  extern Uns16 PlanetMaxFactories(Uns16 pID);
  extern Uns16 PlanetMaxDefense(Uns16 pID);
  extern Uns16 PlanetMaxMines(Uns16 pID);
  extern Uns16 PlanetMining(Uns16 pID, Uns16 Mineral);
  extern Uns16 PlanetGovTaxRate(Uns16 pID);
  extern Uns16 NumberOfShipsHissingPlanet(Uns16 pID);
  extern int PlanetNatHappyChange(Uns16 pID);
  extern int PlanetColHappyChange(Uns16 pID);
  extern Uns16 PlanetColIncome(Uns16 pID);
  extern Uns16 PlanetNatIncome(Uns16 pID);
  extern Uns16 PlanetSuppIncome(Uns16 pID);
  extern const char *PlanetName(Uns16 pID, char *pBuffer);
  extern const char *PlanetFCode(Uns16 pID, char *pFCode);
  extern const char *PlanetTempString(Uns16 pPlanet);
  extern void PutPlanetOwner(Uns16 pID, RaceType_Def pOwner);
  extern void PutPlanetFC(Uns16 pID, char *pFCode);
#define PutPlanetFCode PutPlanetFC
  extern void PutPlanetMines(Uns16 pID, Uns16 pMines);
  extern void PutPlanetFactories(Uns16 pID, Uns16 pFactories);
  extern void PutPlanetDefense(Uns16 pID, Uns16 pDefense);
  extern void PutPlanetCargo(Uns16 pID, CargoType_Def pType, Uns32 pAmount);
  extern void PutPlanetCore(Uns16 pID, CargoType_Def pType, Uns32 pAmount);
  extern void PutPlanetDensity(Uns16 pID, CargoType_Def pType,
        Uns16 pDensity);
  extern void PutPlanetColTax(Uns16 pID, Uns16 pTax);
  extern void PutPlanetNatTax(Uns16 pID, Uns16 pTax);
  extern void PutPlanetColHappy(Uns16 pID, Int16 pHappy);
  extern void PutPlanetNatHappy(Uns16 pID, Int16 pHappy);
  extern void PutPlanetNatGovm(Uns16 pID, NativeGovm_Def pGovm);
  extern void PutPlanetNativePopulation(Uns16 pID, Uns32 pPopulation);
  extern void PutPlanetNatType(Uns16 pID, NativeType_Def pType);
#define PutPlanetNativeType PutPlanetNatType
  extern void PutPlanetTemp(Uns16 pID, Uns16 pTemp);
  extern void PutPlanetBuildBase(Uns16 pID, Boolean pBuild);

  extern void PutPlanetName(Uns16 pID, char *pName);
  extern void PutPlanetLocationX(Uns16 pID, Uns16 pX);
  extern void PutPlanetLocationY(Uns16 pID, Uns16 pY);

/*
 *   Base functions
 */

  extern void DeleteBase(Uns16 pID);
  extern Boolean IsBaseExist(Uns16 pID);
  extern Boolean IsBasePresent(Uns16 pID);
  extern RaceType_Def BaseOwner(Uns16 pID);
  extern Uns16 BaseDefense(Uns16 pID);
  extern Uns16 BaseDamage(Uns16 pID);
  extern Uns16 BaseFighters(Uns16 pID);
  extern BaseMission_Def BaseOrder(Uns16 pID);
  extern BaseFixMission_Def BaseFixOrder(Uns16 pID);
  extern Uns16 BaseFixingShip(Uns16 pID);
  extern Uns16 BaseTech(Uns16 pID, BaseTech_Def pType);
  extern Uns16 BaseHulls(Uns16 pID, Uns16 pHullType);
  extern Uns16 BaseEngines(Uns16 pID, Uns16 pEngineType);
  extern Uns16 BaseBeams(Uns16 pID, Uns16 pBeamType);
  extern Uns16 BaseTubes(Uns16 pID, Uns16 pTorpType);
  extern Uns16 BaseTorps(Uns16 pID, Uns16 pTorpType);
  extern Boolean BaseBuildOrder(Uns16 pID, BuildOrder_Struct * pOrder);
  extern char *BaseOrderString(int bID, char *pBuffer);
  extern void CreateBase(Uns16 pPlanet);
  extern void PutBaseOwner(Uns16 pID, RaceType_Def pOwner);
  extern void PutBaseDefense(Uns16 pID, Uns16 pDefense);
  extern void PutBaseDamage(Uns16 pID, Uns16 pDamage);
  extern void PutBaseFighters(Uns16 pID, Uns16 pFighters);
  extern void PutBaseOrder(Uns16 pID, BaseMission_Def pOrder);
  extern void PutBaseFixOrder(Uns16 pID, BaseFixMission_Def pOrder);
  extern void PutBaseFixingShip(Uns16 pID, Uns16 pShip);
  extern void PutBaseTech(Uns16 pID, BaseTech_Def pType, Uns16 pTech);
  extern void PutBaseHulls(Uns16 pID, Uns16 pHullType, Uns16 pNumber);
  extern void PutBaseEngines(Uns16 pID, Uns16 pEngineType, Uns16 pNumber);
  extern void PutBaseBeams(Uns16 pID, Uns16 pBeamType, Uns16 pNumber);
  extern void PutBaseTubes(Uns16 pID, Uns16 pTorpType, Uns16 pNumber);
  extern void PutBaseTorps(Uns16 pID, Uns16 pTorpType, Uns16 pNumber);
  extern void PutBaseBuildOrder(Uns16 pID, BuildOrder_Struct * pOrderPtr);
  extern void ClearBaseBuildOrder(Uns16 pID);
  extern void ClearBaseHulls(Uns16 pID);

/*
 *   Minefield functions
 */

  extern Uns16 GetNumMinefields(void); /*!< return the maximum number of
                                          minefields */
  extern Boolean IsMinefieldExist(Uns16 pID);
  extern double MinefieldRadius(Uns16 pID);
  extern Uns16 MinefieldPositionX(Uns16 pID);
  extern Uns16 MinefieldPositionY(Uns16 pID);
  extern RaceType_Def MinefieldOwner(Uns16 pID);
  extern Uns32 MinefieldUnits(Uns16 pID);
  extern Boolean IsMinefieldWeb(Uns16 pID);
  extern void PutMinefieldUnits(Uns16 pID, Uns32 pUnits);
  extern void PutMinefieldOwner(Uns16 pID, RaceType_Def pRace);
  extern Uns16 CreateMinefield(Uns16 pXloc, Uns16 pYloc, RaceType_Def pRace,
        Uns32 pUnits, Boolean pIsWeb);

/*
 *   Hull functions
 */

  extern Uns16 HullPicnumber(Uns16 pHullNr);
  extern Uns16 HullTritCost(Uns16 pHullNr);
  extern Uns16 HullDurCost(Uns16 pHullNr);
  extern Uns16 HullMolyCost(Uns16 pHullNr);
  extern Uns16 HullFuelCapacity(Uns16 pHullNr);
  extern Uns16 HullCrewComplement(Uns16 pHullNr);
  extern Uns16 HullEngineNumber(Uns16 pHullNr);
  extern Uns16 HullMass(Uns16 pHullNr);
  extern Uns16 HullTechLevel(Uns16 pHullNr);
  extern Uns16 HullCargoCapacity(Uns16 pHullNr);
  extern Uns16 HullBays(Uns16 pHullNr);
  extern Uns16 HullTubeNumber(Uns16 pHullNr);
  extern Uns16 HullBeamNumber(Uns16 pHullNr);
  extern Uns16 HullMoneyCost(Uns16 pHullNr);
  extern void PutHullPicnumber(Uns16 pHullNr, Uns16 pHullPic);
  extern void PutHullTritCost(Uns16 pHullNr, Uns16 pTritCost);
  extern void PutHullDurCost(Uns16 pHullNr, Uns16 pDurCost);
  extern void PutHullMolyCost(Uns16 pHullNr, Uns16 pMolyCost);
  extern void PutHullFuelCapacity(Uns16 pHullNr, Uns16 pFuelCapacity);
  extern void PutHullCrewComplement(Uns16 pHullNr, Uns16 pCrewComplement);
  extern void PutHullEngineNumber(Uns16 pHullNr, Uns16 pEngineNumber);
  extern void PutHullMass(Uns16 pHullNr, Uns16 pMass);
  extern void PutHullTechLevel(Uns16 pHullNr, Uns16 pTechLevel);
  extern void PutHullCargoCapacity(Uns16 pHullNr, Uns16 pCargoCapacity);
  extern void PutHullBays(Uns16 pHullNr, Uns16 pBays);
  extern void PutHullTubeNumber(Uns16 pHullNr, Uns16 pTubeNumber);
  extern void PutHullBeamNumber(Uns16 pHullNr, Uns16 pBeamNumber);
  extern void PutHullMoneyCost(Uns16 pHullNr, Uns16 pMoneyCost);
  extern void PutTrueHull(RaceType_Def pRace, Uns16 pHullIndex, Uns16 pHullNr);
  extern void PutHullName(Uns16 pHullNr, const char *pName);
  extern Boolean ShipDoesAlchemy(Uns16 pShip);
  extern Boolean ShipDoesRefinery(Uns16 pShip);
  extern Boolean ShipDoesAdvancedRefinery(Uns16 pShip);
  extern Boolean ShipHeatsTo50(Uns16 pShip);
  extern Boolean ShipCoolsTo50(Uns16 pShip);
  extern Boolean ShipHeatsTo100(Uns16 pShip);
  extern Boolean ShipCanHyperwarp(Uns16 pShip);
  extern Boolean ShipIsGravitonic(Uns16 pShip);
  extern Boolean ShipScansAllWormholes(Uns16 pShip);
  extern Boolean ShipIsGamblingShip(Uns16 pShip);
  extern Boolean ShipIsAntiCloaking(Uns16 pShip);
  extern Boolean ShipDoesImperialAssault(Uns16 pShip);
  extern Boolean ShipDoesChunneling(Uns16 pShip);
  extern Boolean ShipHasRamScoop(Uns16 pShip);
  extern Boolean ShipDoesFullBioscan(Uns16 pShip);
  extern Boolean ShipHasAdvancedCloak(Uns16 pShip);
  extern Boolean ShipCanCloak(Uns16 pShip);
  extern Boolean ShipDoesBioscan(Uns16 pShip);
  extern Boolean ShipHasGloryDevice(Uns16 pShip);
  extern Boolean ShipHasHEGloryDevice(Uns16 pShip);
  extern Uns16 TrueHull(RaceType_Def pRace, Uns16 pHullIndex);
  extern const char *HullName(Uns16 pHullNr, char *pBuffer);

/*
 *   Engine functions
 */

  extern char *EngineName(Uns16 pEngNr, char *pBuffer);
  extern Uns16 EngMoneyCost(Uns16 pEngNr);
  extern Uns16 EngTritCost(Uns16 pEngNr);
  extern Uns16 EngDurCost(Uns16 pEngNr);
  extern Uns16 EngMolyCost(Uns16 pEngNr);
  extern Uns16 EngTechLevel(Uns16 pEngNr);
  extern Uns32 EngFuelConsumption(Uns16 pEngNr, Uns16 pSpeed);
  extern void PutEngineName(Uns16 pEngNr, const char *pName);
  extern void PutEngMoneyCost(Uns16 pEngNr, Uns16 pMoneyCost);
  extern void PutEngTritCost(Uns16 pEngNr, Uns16 pTritCost);
  extern void PutEngDurCost(Uns16 pEngNr, Uns16 pDurCost);
  extern void PutEngMolyCost(Uns16 pEngNr, Uns16 pMolyCost);
  extern void PutEngTechLevel(Uns16 pEngNr, Uns16 pTechLevel);
  extern void PutEngFuelConsumption(Uns16 pEngNr, Uns16 pSpeed, Uns32 pFuelConsumption);

/*
 *  Torpedo/tube functions
 */

  extern char *TorpName(Uns16 pTorpNr, char *pBuffer);
  extern Uns16 TorpTorpCost(Uns16 pTorpNr);
  extern Uns16 TorpTubeCost(Uns16 pTorpNr);
  extern Uns16 TorpTritCost(Uns16 pTorpNr);
  extern Uns16 TorpDurCost(Uns16 pTorpNr);
  extern Uns16 TorpMolyCost(Uns16 pTorpNr);
  extern Uns16 TorpTubeMass(Uns16 pTorpNr);
  extern Uns16 TorpTechLevel(Uns16 pTorpNr);
  extern Uns16 TorpKillPower(Uns16 pTorpNr);
  extern Uns16 TorpDestructivePower(Uns16 pTorpNr);
  extern void PutTorpName(Uns16 pTorpNr, char *pName);
  extern void PutTorpTorpCost(Uns16 pTorpNr, Uns16 pTorpCost);
  extern void PutTorpTubeCost(Uns16 pTorpNr, Uns16 pTubeCost);
  extern void PutTorpTritCost(Uns16 pTorpNr, Uns16 pTubeTritCost);
  extern void PutTorpDurCost(Uns16 pTorpNr, Uns16 pTubeDurCost);
  extern void PutTorpMolyCost(Uns16 pTorpNr, Uns16 pTubeMolyCost);
  extern void PutTorpTubeMass(Uns16 pTorpNr, Uns16 pTubeMass);
  extern void PutTorpTechLevel(Uns16 pTorpNr, Uns16 pTechLevel);
  extern void PutTorpKillPower(Uns16 pTorpNr, Uns16 pKillPower);
  extern void PutTorpDestructivePower(Uns16 pTorpNr, Uns16 pDestructivePower);

/*
 *  Beam weapon functions
 */

  extern char *BeamName(Uns16 pBeamNr, char *pBuffer);
  extern Uns16 BeamMoneyCost(Uns16 pBeamNr);
  extern Uns16 BeamTritCost(Uns16 pBeamNr);
  extern Uns16 BeamDurCost(Uns16 pBeamNr);
  extern Uns16 BeamMolyCost(Uns16 pBeamNr);
  extern Uns16 BeamMass(Uns16 pBeamNr);
  extern Uns16 BeamTechLevel(Uns16 pBeamNr);
  extern Uns16 BeamKillPower(Uns16 pBeamNr);
  extern Uns16 BeamDestructivePower(Uns16 pBeamNr);
  extern void PutBeamName(Uns16 pBeamNr, char *pName);
  extern void PutBeamMoneyCost(Uns16 pBeamNr, Uns16 pMoneyCost);
  extern void PutBeamTritCost(Uns16 pBeamNr, Uns16 pTritCost);
  extern void PutBeamDurCost(Uns16 pBeamNr, Uns16 pDurCost);
  extern void PutBeamMolyCost(Uns16 pBeamNr, Uns16 pMolyCost);
  extern void PutBeamMass(Uns16 pBeamNr, Uns16 pMass);
  extern void PutBeamTechLevel(Uns16 pBeamNr, Uns16 pTechLevel);
  extern void PutBeamKillPower(Uns16 pBeamNr, Uns16 pKillPower);
  extern void PutBeamDestructivePower(Uns16 pBeamNr, Uns16 pDestructivePower);

/*
 *   Race name functions
 */

  extern const char *RaceLongName(RaceType_Def pRace, char *pBuffer);
  extern const char *RaceShortName(RaceType_Def pRace, char *pBuffer);
  extern const char *RaceNameAdjective(RaceType_Def pRace, char *pBuffer);
  extern void PutRaceLongName(RaceType_Def pRace, const char *pBuffer);
  extern void PutRaceShortName(RaceType_Def pRace, const char *pBuffer);
  extern void PutRaceNameAdjective(RaceType_Def pRace, const char *pBuffer);

/*
 *   Host run time functions
 */

  extern char *HostTimeStamp(char *pTimeStamp);
  extern Uns16 TurnNumber(void);
  extern Boolean GameFilesVersion(Uns16 * pMajor, Uns16 * pMinor);

/*
 *   Wormhole functions
 */
  extern Uns16 NumWormholes(void);
  extern Uns16 CreateWormhole(void);
  extern void DeleteWormhole(Uns16 pID);
  extern Uns16 WormholeStabilityCode(Uns16 pID);
  extern Uns16 WormholeStartX(Uns16 pID);
  extern Uns16 WormholeStartY(Uns16 pID);
  extern Uns16 WormholeEndX(Uns16 pID);
  extern Uns16 WormholeEndY(Uns16 pID);
  extern Int16 WormholeMass(Uns16 pID);
  extern float WormholeInstability(Uns16 pID);
  extern Uns16 WormholeWaypointStartX(Uns16 pID);
  extern Uns16 WormholeWaypointStartY(Uns16 pID);
  extern Uns16 WormholeWaypointEndX(Uns16 pID);
  extern Uns16 WormholeWaypointEndY(Uns16 pID);
  extern void PutWormholeStartX(Uns16 pID, Uns16 pX);
  extern void PutWormholeStartY(Uns16 pID, Uns16 pY);
  extern void PutWormholeEndX(Uns16 pID, Uns16 pX);
  extern void PutWormholeEndY(Uns16 pID, Uns16 pY);
  extern void PutWormholeMass(Uns16 pID, Int16 pMass);
  extern void PutWormholeInstability(Uns16 pID, float pInstability);
  extern void PutWormholeWaypointStartX(Uns16 pID, Uns16 pX);
  extern void PutWormholeWaypointStartY(Uns16 pID, Uns16 pY);
  extern void PutWormholeWaypointEndX(Uns16 pID, Uns16 pX);
  extern void PutWormholeWaypointEndY(Uns16 pID, Uns16 pY);

/*
 *  util.dat writing
 */
  struct DOSConvertElement;
  extern void SetUtilMode(UtilMode_Def pDat);
  extern Boolean PutUtilRecordSimple(RaceType_Def pRace, Uns16 pRecordId,
                                     Uns16 pSize, void* pData);
  extern Boolean PutUtilRecordStruct(RaceType_Def pRace, Uns16 pRecordId,
                                     void* pData, const struct DOSConvertElement *pStruct,
                                     Uns16 pNumElements);
  extern Boolean PutUtilRecord(RaceType_Def pRace, Uns16 pRecordId, Uns16 pCount,
                               Uns16* pSizes, void** pData);
  extern Boolean PutUtilFileTransfer(RaceType_Def pRace, const char* pName,
                                     Boolean pText, Uns16 pSize, void* pData);

/*
 *  points
 */
  extern Uns32 PointsFor(CargoType_Def lCargo);
  extern void PutPointsFor(CargoType_Def lCargo, Uns16 lPoints);
  extern Uns32 PointsForFighter(void);
  extern Uns32 PointsForBaseDefensePost(void);
  extern Uns32 PointsForMineralMine(void);
  extern Uns32 PointsForFactory(void);
  extern Uns32 PointsForDefencePost(void);
  extern Uns32 PointsForBaseOnPlanet(void);

  extern Uns32 PointsForHull(Uns16 lHullNr);
  extern Uns32 PointsForEngine(Uns16 lEngNr);
  extern Uns32 PointsForBeam(Uns16 lBeamNr);
  extern Uns32 PointsForTube(Uns16 lTorpNr);
  extern Uns32 PointsForTorpedo(Uns16 lTorpNr);
  extern Uns32 PointsForShipParts(Uns16 sID);
  extern Uns32 PointsForShipCargo(Uns16 sID);
  extern Uns32 PointsForShipAmmo(Uns16 sID);
  extern Uns32 PointsForPlanetStructures(Uns16 pID);
  extern Uns32 PointsForPlanetMinerals(Uns16 pID);
  extern Uns32 PointsForBaseTechLevels(Uns16 bID);
  extern Uns32 PointsForBaseDefense(Uns16 bID);
  extern Uns32 PointsForBaseStorage(Uns16 bID);
  extern Uns32 PointsForBaseTorpedos(Uns16 bID);
  extern Uns32 PointsForShip(Uns16 sID, PointsType_Def CountType);
  extern Uns32 PointsForPlanet(Uns16 pID, PointsType_Def CountType);
  extern Uns32 PointsForBase(Uns16 bID, PointsType_Def CountType);
  extern Uns32 PointsForMinefield(Uns16 mID, PointsType_Def CountType);
  extern Boolean IsShipType(Uns16 sID,ShipsType_Def ShipsType);
  extern Uns32 RaceScoreForShips( RaceType_Def Race, PointsType_Def CountType, ShipsType_Def ShipsType);
  extern Uns32 RaceScoreForPlanets( RaceType_Def Race, PointsType_Def CountType);
  extern Uns32 RaceScoreForBases( RaceType_Def Race, PointsType_Def CountType);
  extern Uns32 RaceScoreForMinefields( RaceType_Def Race, PointsType_Def CountType, Boolean RaceModify);
  extern Uns32 RaceShipsNumber( RaceType_Def Race, ShipsType_Def ShipsType);
  extern Uns32 RacePlanetsNumber(RaceType_Def Race);
  extern Uns32 RaceBasesNumber( RaceType_Def Race);
  extern Uns32 RaceFightersNumber(RaceType_Def Race);
  extern Uns32 RaceTorpedosNumber(RaceType_Def Race, Uns16 TorpType);
  extern Uns32 RaceFactoriesNumber(RaceType_Def Race);
  extern Uns32 RaceMineralMinesNumber(RaceType_Def Race);
  extern Uns32 RaceDefencePostsNumber(RaceType_Def Race);
  extern Uns32 RaceBaseDefenceNumber(RaceType_Def Race);
  extern Uns32 RaceCargoNumber(RaceType_Def Race,CargoType_Def CargoType);
  extern Uns32 RaceMinefieldUnitsNumber(RaceType_Def Race);

  /* friendly codes */
  extern void InitFCodes(void);
  extern void ShutdownFCodes(void);
  extern void DumpExtraFCodes(FILE *pFile);
  extern Boolean DefineSpecialFCode(const char* pCode);
  extern Boolean IsFCodeSpecial(const char* pFCode);
  extern Boolean IsShipFCodeSpecial(Uns16 pShip);
  extern Boolean IsPlanetFCodeSpecial(Uns16 pPlanet);
  extern Boolean IsFCodeMatch(const char* p1, const char* p2);


#ifdef __cplusplus
}
#endif
#endif                          /*!< _PHOSTPDK_H_ */

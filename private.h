#ifndef _PRIVATE_H_
#define _PRIVATE_H_

#include "shrouds.h"
#include "version.h"
#include "includes.h"
#include "doscvt.h"

#undef RACE_NR
#define RACE_NR 12
#define OLD_RACE_NR 11

static const Uns16 EXT_MESSAGE_SIZE = 8; /* size of ExtMessage_Struct */

enum {
    aux_BaseNatives      = 1,
    aux_AllianceInfo     = 2,
    aux_ShipScanInfo     = 3,
    aux_BuildQueue       = 4,
    aux_PAL              = 5,
    aux_Remote           = 6,
    aux_ShipSpecial      = 7,
    aux_ShipVisible      = 8,
    aux_ShipExperience   = 9,
    aux_PlanetExperience = 10,
    aux_Enemies          = 11,
    aux_ShipExtraSpecial = 12,
    aux_ShipExtraDef     = 13,

    /* 100+ are transient records */
    aux_ShipFlags        = 101,
    aux_PlanetFlags      = 102,
    aux_ShipNewExp       = 103,
    aux_PlanetNewExp     = 104,
    aux_TAL              = 105,
    aux_SpecialInhibited = 106,
    aux_Bangs            = 107
};

typedef unsigned char Byte;
typedef unsigned char Uns8;
typedef Uns16 CRCType;

#define  AND   &&
#define  OR    ||
#define  EQ    ==
#define  NEQ   !=
#define  GT    >
#define  GE    >=
#define  LT  <
#define  LE    =<

/* We use our own random number generator */
#ifdef RAND_MAX
#undef RAND_MAX
#endif
#define RAND_MAX 65535U

#ifdef __cplusplus
#  define HAVE_STATIC_INLINE
#  define STATIC_INLINE static inline
#  define C_LINKAGE "C"
#else
#  define C_LINKAGE
#endif

#ifdef __GNUC__
#  define HAVE_STATIC_INLINE
#  define STATIC_INLINE static __inline__
#endif

#ifndef HAVE_STATIC_INLINE
#define STATIC_INLINE
#endif

#ifndef _MAX_PATH
#  ifdef _POSIX_PATH_MAX
#  define _MAX_PATH _POSIX_PATH_MAX
#  else
#  define _MAX_PATH 1024
#  endif
#endif

#ifndef _MAX_FNAME
#  ifdef _POSIX_NAME_MAX
#  define _MAX_FNAME _POSIX_NAME_MAX
#  else
#  define _MAX_FNAME 256
#  endif
#endif

/* Manifests for X and Y components of a location */
typedef enum {
  X_Part = 0,                   /* Also used as indices into arrays */
  Y_Part = 1
} XYComponent_Def;

typedef struct {
  RaceType_Def Owner;
  Uns16 Id;
  char FriendlyCode[3];
  Uns16 Mines,
    Factories,
    Defense;
  Uns32 Cargo[7],               /* N, T, D, M, CLANS!!!, S, MC */

    Core[4];
  Uns16 Density[4],
    ColTax,
    NatTax;
  Int16 ColHappy,
    NatHappy;
  NativeGovm_Def Govm;
  Uns32 NativePopulation;       /* CLANS!!! */
  NativeType_Def NativeType;
  Uns16 Temp;                   /* Really, 100-temperature */
  Boolean BuildBase;
} Planet_Struct;

typedef struct {
  Uns16 Id;
  RaceType_Def Owner;
  char FriendlyCode[3];
  Uns16 Speed;
  Int16 Waypoint[2];            /* OFFSET from location */
  Uns16 Location[2];            /* x,y */
  Uns16 Engine,
    Hull,
    BeamType,
    BeamNumber,
    Bays,
    TorpType,
    Ammunition,
    TubeNumber;
  ShipMission_Def Mission;
  RaceType_Def Enemy;           /* primary enemy */
  Uns16 TowTarget,
    Damage,
    Crew,
    Colonists;
  char Name[20];                /* NO closing 0 !!! */
  Uns16 Cargo[5],               /* N, T, D, M, S *//* NO CLANS!!! */

    Dump[6],                    /* N, T, D, M, C, S */

    DumpPlanet,                 /* 0 for jettison */

    Transfer[6],                /* N, T, D, M, C, S */

    TransferShip,
    InterceptTarget,
    Credits;
} Ship_Struct;

typedef struct {
  Uns16 Id;
  RaceType_Def Owner;
  Uns16 Defense,
    Damage,
    TechLevel[4],
    Engines[9],
    Hulls[20],
    Beams[10],
    Tubes[10],
    Torps[10],
    Fighters,
    FixingShip;
  BaseFixMission_Def FixOrder;
  BaseMission_Def Order;
  Uns16 ToBuild[7];             /* hull,eng,beam,#beams,torp,#torp,bla */
} Base_Struct;

/* This enumeration defines the various positions in the ToBuild member
   of Base_Struct */

typedef enum {
  BuildHullType,
  BuildEngineType,
  BuildBeamType,
  BuildBeamNumber,
  BuildTorpType,
  BuildTorpNumber
} ToBuild_Def;

typedef struct {
  Uns16 X;
  Uns16 Y;
  RaceType_Def Owner;
} XYData_Struct;

/* xyplan.hst, xyplan.dat : Xydata_Struct[500] ; xyplan.dat : owner always 0 */

typedef struct {
  char Name[30];
  Uns16 Picnumber,
    Bla,                        /* unused */

    TritCost,
    DurCost,
    MolyCost,
    FuelCapacity,
    CrewComplement,
    EngineNumber,
    Mass,
    TechLevel,
    CargoCapacity,
    Bays,
    TubeNumber,
    BeamNumber,
    MoneyCost;
} Hullspec_Struct;

typedef struct {
  char Name[20];
  Uns16 MoneyCost,
    TritCost,
    DurCost,
    MolyCost,
    TechLevel;
  Uns32 FuelConsumption[9];
} Engspec_Struct;

typedef struct {
  char Name[20];
  Uns16 TorpCost,
    TubeCost,
    TubeTritCost,
    TubeDurCost,
    TubeMolyCost,
    TubeMass,
    TechLevel,
    KillPower,
    DestructivePower;
} Torpspec_Struct;

typedef struct {
  char Name[20];
  Uns16 MoneyCost,
    TritCost,
    DurCost,
    MolyCost,
    Mass,
    TechLevel,
    KillPower,
    DestructivePower;
} Beamspec_Struct;

/* hullspec.dat : Hullspec_Struct[105]
   engspec.dat : Engspec_Struct[9]   + byte unused[4]
   beamspec.dat : Beamspec_Struct[10]
   torpspec.dat : Torpspe_Struct[10]  + byte unused[90] */

typedef Uns16 Truehull_Def[12][21];

/* Every race can build up to 20 different hulls
   the numbers in truehull.dat refer to the hull slot in hullspec.dat
   ID's range from 1 to 105 (=HULLNR)
   truehull contains zeros if the race can build less than 20 hulls
*/

typedef struct {
  Uns16 Position[2];
  RaceType_Def Race;
  Uns32 Units;
  Boolean Web;
} MineField_Struct;

/* mines.hst : MineField_Struct[500] */

typedef char *RaceName_Def[3][12];

typedef struct {
  Uns16 Position[2];
  RaceType_Def Owner;
  Uns16 Mass;
} Shipxy_Struct;

/* shipxy.hst : shipxy[500] */

typedef struct {
  char Name[20];                /* No NULL character! */
} Planetname_Struct;

/* planet.nm : Planetname_Struct[500] */

typedef struct {
  char HostTime[18];
  Uns16 TurnNumber;
} Turntime_Struct;

/* nextturn.hst, lastturn.hst, both identical ? */

typedef struct {
  char Name[20];
  Uns16 Damage,
    Crew,
    Id;
  RaceType_Def Race;
  Uns8 Picnumber,               /* picture in hullspec (picnumber) */

    Hullnumber;                 /* For ships only: hull # from HULLSPEC */
  Uns16 Beamtype,
    Beamnumber,
    Bays,
    Torps,                      /* Tube tech */

    Ammunition,                 /* torps or fighters */

    Tubes;
} ShipVcr_Struct;

typedef struct {
  Int16 Random1;
  Boolean Dummy;                /* never true */
  Uns16 Planetpic;              /* only !=0 if is_planet2 is true */
  Boolean Is_planet2;
  Uns16 Mass1,
    Mass2;
  ShipVcr_Struct Ship1,
    Ship2;
  Uns16 Shield1,
    Shield2;
} Vcr_Struct;

/*
structure of vcr??.dat/vcr.hst :

Int16   number_of_battles;
Vcr_Struct  vcrdat[number_of_battles];
byte   signature[10];  (player data only)
*/

/* in gen.hst are a lot bytes with unknown use, which our host
   can use for his own infos */

typedef struct {
  Byte Bla[24];
  Byte WasUsingWinPlan[OLD_RACE_NR];
  Byte WasRegistered[OLD_RACE_NR];
  Boolean Players[OLD_RACE_NR]; /* active players, identical to init.tmp */
  char Passwd1[OLD_RACE_NR][10],
    Passwd2[OLD_RACE_NR][10];

/* this is all fromm address 0000-011F after master (= 288 Bytes)
   after first hosting it will be 450 Bytes
    +byte  bla2[162];
   mostly bla and bla2 have only 00
*/
} HostGen_Struct;

typedef struct {
  RaceType_Def mRace;           /* Destination race */
  Uns32 mFilePos;               /* Starting position in MESS.EXT */
  Uns16 mLength;                /* Message length */
} ExtMessage_Struct;

/* Structure of MESSPNT.EXT file:
  Uns16 NumMessages
  ExtMessage_Struct Messages[NumMessages]
*/

/* The structure of an entry in the target.dat file and the UTIL.DAT file */

typedef struct {
  Uns16 Id;
  RaceType_Def Owner;
  Uns16 Speed;
  Int16 Location[2];
  Uns16 Hull;
  Int16 Heading;                /* -1 for unknown */
  char Name[20];
} TargetShip_Struct;

typedef struct {
  Uns32 mTotalTons[OLD_RACE_NR]; /* 0-based arrays -- be careful */
  Uns32 mCurrentTons[OLD_RACE_NR];
} Tons_Struct;

#define GAME_NAME_SIZE  32      /* Maximum size of game names */

#define Hconfig_Struct Pconfig_Struct
#define gConfigInfo gPconfigInfo

/* These are flags that can be passed to the ReadConfigFile/WriteConfigFile
   routines. */
typedef enum {
  CFG_NO_FLAGS = 0,
  CFG_ACCEPT_NON_PHOST = 0x0001, /* Accept a non-PHOST-generated config file */
  CFG_WRITE_INCLUDE = 0x0002,   /* Write ASCII include file rather than
                                   binary config file */
  CFG_NO_REPORTING = 0x0004,    /* Print no diagnostics */
  CFG_WRITE_OLD_FORMAT = 0x0008 /* Write in HCONFIG format */
} CFGFlags_Def;

/* GREY.HST */

typedef struct {
    Int16         X, Y;
    Uns16         Radius;
    Uns16         Voltage;
    Uns16         Heading;
    Uns16         GrowthFlag;
    Uns32         Unused;
} IonStorm_Struct;

#ifdef __cplusplus
extern "C" {
#endif

  extern void ChangePlanetOwner(Uns16 pID, RaceType_Def pNewOwner);

  extern void InitCRC(void);
  extern CRCType GenerateCRC(void *base, Uns32 size);
  extern CRCType IncrementalCRC(unsigned char byte, CRCType crc);
  extern CRCType GenerateCRCContinue(void *pData, Uns32 pSize, CRCType pCrc);

  extern Boolean ReadWormholeFile(void);
  extern Boolean WriteWormholeFile(void);

  extern const char *SystemName(int syscode);

  extern Planet_Struct *GetPlanet(Uns16 pID);
  extern Ship_Struct *GetShip(Uns16 pID);
  extern Base_Struct *GetBase(Uns16 pID);
  extern MineField_Struct *GetMinefield(Uns16 pID);

/* functions store requested structure and generate them new if necessary */

  extern void PutPlanet(Uns16 pID, Planet_Struct * pPlanet);
  extern void PutShip(Uns16 pID, Ship_Struct * pShip);
  extern void PutBase(Uns16 pID, Base_Struct * pBase);

  extern const Turntime_Struct *RawTurnTime(void), *RawAuxTime(void);

  extern void SquishError(const char *str);

  typedef void (*CleanupFunction_T) (void);

  extern void RegisterCleanupFunction(CleanupFunction_T pFunc);

  extern Hconfig_Struct *gConfigInfo;

  extern char *SkipWS(const char *p);
  extern void TrimTrailingWS(char *p);
  extern void ZeroTrailingWS(char *pStr, Uns16 pLen);

/* Single-read routines */
  extern Boolean Read_Hullspec_File(void);
  extern Boolean Read_Engspec_File(void);
  extern Boolean Read_Beamspec_File(void);
  extern Boolean Read_Torpspec_File(void);
  extern Boolean Read_Truehull_File(void);
  extern Boolean Read_Planetname_File(void);
  extern Boolean Read_Planets_File(Int16 * pControl);
  extern Boolean Read_Ships_File(Int16 * pControl);
  extern Boolean Read_Bases_File(Int16 * pControl1, Int16 * pControl2);
  extern Boolean Read_Mines_File(void);
  extern Boolean Read_HostGen_File(void);
  extern Boolean Read_Racenames_File(void);
  extern Boolean Read_Xyplan_File(void);
  extern Boolean Read_HConfig_File(void);
  extern Boolean Read_AuxData_File(void);
  extern Boolean Read_Turntime_File(void);
  extern Boolean ReadWormholeFile(void);
  extern void ReadCLOAKCFile(void);
  extern void HandleMissingAuxdata(void);

  extern void ReadHullfunc(void);

  extern void WrapShipLocation(Uns16 pShip);
  extern void WrapMineLocation(Uns16 pMine);
  extern void ReinitWraparound(void);

  extern void InitConfig(void);

  extern int RND(double value);

#ifdef __cplusplus
}
#endif
#endif                          /* _PRIVATE_H_ */

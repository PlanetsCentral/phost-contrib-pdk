/**
  *  \file ion.c
  *  \brief Ion Storms
  *  \author Stefan Reuther
  *
  *  This module is mostly cut&paste from PHost.
  */

#include "phostpdk.h"
#include "private.h"

/** Data about a storm. */
typedef struct {
    IonStorm_Struct  mData;         /**< Data as stored in host data file. */
    /* Uns16 mSpeed; The speed is computed internally by the host and
       not available to add-ons. */
} RealIonStorm_Struct;

static RealIonStorm_Struct  gStorms[STORM_NR];
static char                 gStormNames[STORM_NR][20];

static const char gStormNameFile[] = "storm.nm";
static const char gGreyFile[] = "grey.hst";

/** Get pointer to storm by Id. */
static RealIonStorm_Struct*
GetStorm(Uns16 pStorm)
{
    passert(pStorm > 0 && pStorm <= STORM_NR);
    return &gStorms[pStorm-1];
}

/** Check whether a storm exists. */
Boolean
IsStormExist(Uns16 pStorm)
{
    return (pStorm > 0) && (pStorm <= STORM_NR) && (GetStorm(pStorm)->mData.Voltage != 0);
}

/** Get storm X location. */
Int16
StormLocationX(Uns16 pStorm)
{
    return GetStorm(pStorm)->mData.X;
}

/** Get storm Y location. */
Int16
StormLocationY(Uns16 pStorm)
{
    return GetStorm(pStorm)->mData.Y;
}

/** Get storm radius. */
Uns16
StormRadius(Uns16 pStorm)
{
    return GetStorm(pStorm)->mData.Radius;
}

/** Get storm heading. */
Uns16
StormHeading(Uns16 pStorm)
{
    return GetStorm(pStorm)->mData.Heading;
}

/** Check whether storm is growing. */
Boolean
IsStormGrowing(Uns16 pStorm)
{
    return GetStorm(pStorm)->mData.GrowthFlag != 0;
}

/** Get storm voltage. */
Uns16
StormVoltage(Uns16 pStorm)
{
    return GetStorm(pStorm)->mData.Voltage;
}

/** Get storm name. */
char*
StormName(Uns16 pStorm, char* pBuffer)
{
    static char lBuffer[21];
    passert(pStorm > 0 && pStorm <= STORM_NR);
    if (!pBuffer)
        pBuffer = lBuffer;
    memcpy(pBuffer, gStormNames[pStorm-1], 20);
    pBuffer[20] = 0;
    TrimTrailingWS(pBuffer);

    if (pBuffer[0] == 0) {
        /* fallback, in case file is empty */
        sprintf(pBuffer, "Storm %d", (int) pStorm);
    }
    return pBuffer;
}

/** Get storm class. */
Uns16
StormClass(Uns16 pStorm)
{
    int v = (StormVoltage(pStorm) + 1) / 50;
    if (v <= 1)
        return 1;
    if (v <= 5)
        return v;
    return 5;
}

void
PutStormLocationX(Uns16 pStorm, Int16 pX)
{
    GetStorm(pStorm)->mData.X = WrapMapX(pX);
}

void
PutStormLocationY(Uns16 pStorm, Int16 pY)
{
    GetStorm(pStorm)->mData.Y = WrapMapY(pY);
}

void
PutStormRadius(Uns16 pStorm, Uns16 pRadius)
{
    if (pRadius > MAX_STORM_RADIUS)
        pRadius = MAX_STORM_RADIUS;
    GetStorm(pStorm)->mData.Radius = pRadius;
}

void
PutStormHeading(Uns16 pStorm, Uns16 pHeading)
{
    GetStorm(pStorm)->mData.Heading = pHeading;
}

void
PutStormVoltage(Uns16 pStorm, Uns16 pVoltage)
{
    RealIonStorm_Struct* lP = GetStorm(pStorm);
    if (pVoltage > MAX_STORM_VOLTAGE)
        pVoltage = MAX_STORM_VOLTAGE;
    lP->mData.Voltage    = pVoltage;
    lP->mData.GrowthFlag = pVoltage & 1;
}

void
CreateStorm(Uns16 pStorm, Uns16 pX, Uns16 pY, Uns16 pRadius, Uns16 pVoltage, Uns16 pHeading)
{
    PutStormVoltage(pStorm, pVoltage);
    PutStormLocationX(pStorm, pX);
    PutStormLocationY(pStorm, pY);
    PutStormRadius(pStorm, pRadius);
    PutStormHeading(pStorm, pHeading);
}

/** Delete a storm. */
void
DeleteStorm(Uns16 pStorm)
{
    PutStormVoltage(pStorm, 0);
}

/** Count active ion storms. */
int
GetStormCount(void)
{
    int lStorm, lCount;
    lCount = 0;
    for (lStorm = 1; lStorm <= STORM_NR; ++lStorm)
        if (IsStormExist(lStorm))
            ++lCount;
    return lCount;
}

/** Load ion storm names. Note that if ion storms are disabled, we
    consider it a success even if the file does not exist, to permit
    graceful update for running games.

    \return True on success */
static Boolean
InitStormNames(void)
{
    FILE* lFile = OpenInputFile(gStormNameFile, GAME_OR_ROOT_DIR | NO_MISSING_ERROR);
    if (!lFile) {
        return False;
    } else {
        fread(gStormNames, 1, sizeof(gStormNames), lFile);
        fclose(lFile);
        return True;
    }
}

/** Delete all ion storms. */
static void
ClearStorms(void)
{
    int i;
    for (i = 1; i <= STORM_NR; ++i) {
        DeleteStorm(i);
    }
}

/** Load ion storms from host data file.
    \param pFile  host data file, file pointer placed correctly
    \return True on success */
static Boolean
LoadStorms(FILE* pFile)
{
    int i;
    for (i = 1; i <= STORM_NR; ++i) {
        IonStorm_Struct lTmp;
        if (!DOSReadStruct(IonStormStruct_Convert, NumIonStormStruct_Convert, &lTmp, pFile))
            return False;

        /* validate it */
        if (lTmp.X <= MIN_COORDINATE || lTmp.X > MAX_COORDINATE
            || lTmp.Y <= MIN_COORDINATE || lTmp.Y > MAX_COORDINATE
            || lTmp.Radius > MAX_STORM_RADIUS || lTmp.Voltage > MAX_STORM_VOLTAGE
            || lTmp.Radius <= 0)
        {
            /* storm is invalid, ignore it. */
            /* FIXME: message */
            DeleteStorm(i);
        } else {
            RealIonStorm_Struct* pS = GetStorm(i);
            lTmp.Heading %= 360;
            lTmp.GrowthFlag = lTmp.Voltage & 1;
            lTmp.X = WrapMapX(lTmp.X);
            lTmp.Y = WrapMapX(lTmp.Y);
            pS->mData = lTmp;
            /* do not modify pS->mSpeed, to carry it over from previous run. */
        }
    }
    return True;
}

/** Save ion storms to host data file.
    \param pFile  host data file, file pointer placed correctly
    \return True on success */
static Boolean
SaveStorms(FILE* pFile)
{
    int i;
    for (i = 1; i <= STORM_NR; ++i) {
        if (!DOSWriteStruct(IonStormStruct_Convert, NumIonStormStruct_Convert, &GetStorm(i)->mData, pFile))
            return False;
    }
    return True;
}

/** Load ion storms. This pretends to be always successful, because it
    must cope with non-present or partial files left by previous host
    versions. */
IO_Def
Read_Ion_File(void)
{
    FILE* lFP;

    lFP = OpenInputFile(gGreyFile, GAME_DIR_ONLY | NO_MISSING_ERROR);
    if (lFP == 0) {
        ClearStorms();
    } else {
        fseek(lFP, 1000, SEEK_SET);
        LoadStorms(lFP);
        fclose(lFP);
    }
    InitStormNames();
    return IO_SUCCESS;
}

/** Write ion storms. */
IO_Def
Write_Ion_File(void)
{
    FILE* lFP;
    Boolean lSuccess;
    int i;

    lFP = OpenUpdateFile(gGreyFile, GAME_DIR_ONLY | REWRITE_MODE | NO_MISSING_ERROR);
    if (lFP == 0) {
        if (GetStormCount() == 0)
            return IO_SUCCESS;

        /* create file only if needed */
        lFP = OpenOutputFile(gGreyFile, GAME_DIR_ONLY);
        for (i = 0; i < 1000; ++i)
            fputc(0, lFP);
    } else {
        fseek(lFP, 1000, SEEK_SET);
    }
    lSuccess = SaveStorms(lFP);
    fclose(lFP);
    return lSuccess ? IO_SUCCESS : IO_FAILURE;
}

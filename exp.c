/**
  *  \file exp.c
  *  \brief Experience Access
  *  \author Stefan Reuther
  *
  *  This module defines the functions to access PHost 4.0 experience.
  */

#include "phostpdk.h"
#include "doscvt.h"
#include "private.h"

/** Saturated addition. Makes sure we don't accidentially hit some
    overflow (PHost has the same restriction). */
static void
SaturatedAdd(void* pPtr, Uns32 pValue)
{
    pValue += ReadDOSUns32(pPtr);
    if (pValue > 1000000000U)
        pValue = 1000000000U;
    WriteDOSUns32(pPtr, pValue);
}

Exp_Handle
ShipExperience(Uns16 pShipId)
{
    struct Auxdata_Chunk* lExpChunk = GetAuxdataChunkById(aux_ShipExperience),
        *lNewChunk = GetAuxdataChunkById(aux_ShipNewExp);

    Exp_Handle rv;
    rv.mOldPoints = (pShipId > 0 && 4*pShipId <= AuxdataChunkSize(lExpChunk))
        ? (char*) AuxdataChunkData(lExpChunk) + 4 * (pShipId-1) : 0;
    rv.mNewPoints = (pShipId > 0 && 4*pShipId <= AuxdataChunkSize(lNewChunk))
        ? (char*) AuxdataChunkData(lNewChunk) + 4 * (pShipId-1) : 0;
    return rv;
}

Exp_Handle
PlanetExperience(Uns16 pPlanetId)
{
    struct Auxdata_Chunk* lExpChunk = GetAuxdataChunkById(aux_PlanetExperience),
        *lNewChunk = GetAuxdataChunkById(aux_PlanetNewExp);

    Exp_Handle rv;
    rv.mOldPoints = (pPlanetId > 0 && 4*pPlanetId <= AuxdataChunkSize(lExpChunk))
        ? (char*) AuxdataChunkData(lExpChunk) + 4 * (pPlanetId-1) : 0;
    rv.mNewPoints = (pPlanetId > 0 && 4*pPlanetId <= AuxdataChunkSize(lNewChunk))
        ? (char*) AuxdataChunkData(lNewChunk) + 4 * (pPlanetId-1) : 0;
    return rv;
}

int
ExperienceLevel(Exp_Handle pH)
{
#ifndef PHOST4
    return 0;
#else
    Uns32 lPoints = ExperiencePoints(pH);
    int lLevel = 0;
    while (lLevel < gPconfigInfo->NumExperienceLevels &&
           lPoints >= gPconfigInfo->ExperienceLevels[lLevel])
        ++lLevel;
    return lLevel;
#endif
}

void
AddToExperience(Exp_Handle pH, Uns32 pNumber)
{
    if (pH.mNewPoints)
        SaturatedAdd(pH.mNewPoints, pNumber);
    else if (pH.mOldPoints)
        SaturatedAdd(pH.mOldPoints, pNumber);
}

Uns32
ExperiencePoints(Exp_Handle pH)
{
    if (pH.mOldPoints)
        return ReadDOSUns32(pH.mOldPoints);
    else
        return 0;
}
        
void
PutExperiencePoints(Exp_Handle pH, Uns32 pPoints)
{
    if (pH.mOldPoints)
        WriteDOSUns32(pH.mOldPoints, pPoints);
    if (pH.mNewPoints)
        WriteDOSUns32(pH.mNewPoints, 0);
}

Uns32
NewExperiencePoints(Exp_Handle pH)
{
    if (pH.mNewPoints)
        return ReadDOSUns32(pH.mNewPoints);
    else
        return 0;
}

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

static const char *BASE_FILE         = "bdata.hst";
static Base_Struct   **gAllBases   = 0;

static void FreeBases(void)
{
    MemFree(gAllBases); gAllBases = 0;
}

static void InitBases(void)
{
    if (gAllBases EQ 0) {
        gAllBases   = (Base_Struct  **)MemCalloc(PLANET_NR+1, sizeof(Base_Struct *));

        RegisterCleanupFunction(FreeBases);
    }
}

 Base_Struct* GetBase(Uns16 pID)
{
    InitBases();

    passert( (pID>0) AND (pID<=BASE_NR) );
    passert( gAllBases[pID] NEQ 0 AND gAllBases[pID]->Owner NEQ 0 );

    return gAllBases[pID];
}

 void PutBase(Uns16 pID, Base_Struct* pBase)
{
    InitBases();

    passert( (pID>0) AND (pID<=BASE_NR) AND (pBase NEQ NULL) );

    if (gAllBases[pID] EQ NULL) {
        gAllBases[pID] = (Base_Struct *) MemCalloc(1, sizeof(Base_Struct));
    }
    memmove(gAllBases[pID], pBase, sizeof(Base_Struct));
}

 void DeleteBase(Uns16 pID)
{
    InitBases();

    passert( (pID>0) AND (pID<=BASE_NR) AND (gAllBases[pID] NEQ NULL)
            AND (gAllBases[pID]->Owner NEQ 0) );

    MemFree(gAllBases[pID]);
    gAllBases[pID] = 0;
}

 Boolean IsBasePresent(Uns16 pID)
{
    InitBases();

    if ( (pID>0) AND (pID<=BASE_NR) AND (gAllBases[pID] NEQ NULL)
        AND (gAllBases[pID]->Owner NEQ 0) )
		return(True);
	else
		return(False);
}

Boolean IsBaseExist(Uns16 pID)
{
    InitBases();

    return IsBasePresent(pID) AND (PlanetOwner(pID) NEQ NoOwner);
}

/* bases */

 RaceType_Def BaseOwner(Uns16 pID)
{
	return(GetBase(pID)->Owner);
}

 Uns16 BaseDefense(Uns16 pID)
{
	return(GetBase(pID)->Defense);
}

 Uns16 BaseDamage(Uns16 pID)
{
	return(GetBase(pID)->Damage);
}

 Uns16 BaseFighters(Uns16 pID)
{
	return(GetBase(pID)->Fighters);
}

 BaseMission_Def BaseOrder(Uns16 pID)
{
	return(GetBase(pID)->Order);
}

 BaseFixMission_Def BaseFixOrder(Uns16 pID)
{
	return(GetBase(pID)->FixOrder);
}

 Uns16 BaseFixingShip(Uns16 pID)
{
	return(GetBase(pID)->FixingShip);
}

 Uns16 BaseTech(Uns16 pID, BaseTech_Def pType)
{
	return(GetBase(pID)->TechLevel[pType]);
}

 Uns16 BaseHulls(Uns16 pID, Uns16 pHullType)
{
	/* NEW */
	passert( (pHullType >= 1) AND (pHullType <= RACEHULLS) );
	return(GetBase(pID)->Hulls[pHullType - 1 /* NEW */]);
}

 Uns16 BaseEngines(Uns16 pID, Uns16 pEngineType)
{
	passert( (pEngineType >= 1) AND (pEngineType <= ENGINE_NR) );
	return(GetBase(pID)->Engines[pEngineType - 1 /* NEW */]);
}

 Uns16 BaseBeams(Uns16 pID, Uns16 pBeamType)
{
	passert( (pBeamType >= 1) AND (pBeamType <= BEAM_NR) );
	return(GetBase(pID)->Beams[pBeamType - 1 /* NEW */]);
}

 Uns16 BaseTubes(Uns16 pID, Uns16 pTorpType)
{
	passert( (pTorpType >= 1) AND (pTorpType <= TORP_NR) );
	return(GetBase(pID)->Tubes[pTorpType - 1 /* NEW */]);
}

 Uns16 BaseTorps(Uns16 pID, Uns16 pTorpType)
{
	passert( (pTorpType >= 1) AND (pTorpType <= TORP_NR) );
	return(GetBase(pID)->Torps[pTorpType - 1 /* NEW */]);
}

 Boolean BaseBuildOrder(Uns16 pID, BuildOrder_Struct *pOrder)
{
	Base_Struct *lBase = GetBase(pID);

	/* Check that hull is non-zero */
	if (		(lBase->ToBuild[BuildHullType] NEQ 0)
			AND (lBase->ToBuild[BuildEngineType] NEQ 0)
			AND ( (lBase->ToBuild[BuildBeamType] NEQ 0)
				   OR
				  (lBase->ToBuild[BuildBeamNumber] EQ 0)
				)
			AND ( (lBase->ToBuild[BuildTorpType] NEQ 0)
				   OR
				  (lBase->ToBuild[BuildTorpNumber] EQ 0)
				)
	   )
	{
		memcpy(pOrder, lBase->ToBuild, sizeof(BuildOrder_Struct));
		return True;
	}
	return False;
}

 void PutBaseOwner(Uns16 pID, RaceType_Def pOwner)
{
	GetBase(pID)->Owner=pOwner;
}

 void PutBaseDefense(Uns16 pID, Uns16 pDefense)
{
	passert(pDefense<=MAX_BASE_DEFENSE);
	GetBase(pID)->Defense=pDefense;
}

 void PutBaseDamage(Uns16 pID, Uns16 pDamage)
{
	passert(pDamage<=150);
	GetBase(pID)->Damage=pDamage;
}

 void PutBaseFighters(Uns16 pID, Uns16 pFighters)
{
	passert(pFighters<=MAX_BASE_FIGHTERS);
	GetBase(pID)->Fighters=pFighters;
}

 void PutBaseOrder(Uns16 pID, BaseMission_Def pOrder)
{
	GetBase(pID)->Order=pOrder;
}

 void PutBaseFixOrder(Uns16 pID, BaseFixMission_Def pOrder)
{
	GetBase(pID)->FixOrder=pOrder;
}

 void PutBaseFixingShip(Uns16 pID, Uns16 pShip)
{
	passert(pShip<=SHIP_NR);
	GetBase(pID)->FixingShip=pShip;
}

 void PutBaseTech(Uns16 pID, BaseTech_Def pType, Uns16 pTech)
{
	passert(pTech<=MAX_TECH);
	GetBase(pID)->TechLevel[pType]=pTech;
}

 void PutBaseHulls(Uns16 pID, Uns16 pHullType, Uns16 pNumber)
{
	passert( (pHullType >= 1) AND (pHullType <= RACEHULLS) );
	GetBase(pID)->Hulls[pHullType - 1 /* NEW */] = pNumber;
}

 void PutBaseEngines(Uns16 pID, Uns16 pEngineType, Uns16 pNumber)
{
	passert( (pEngineType >= 1) AND (pEngineType <= ENGINE_NR) );
	GetBase(pID)->Engines[pEngineType - 1 /* NEW */] = pNumber;
}

 void PutBaseBeams(Uns16 pID, Uns16 pBeamType, Uns16 pNumber)
{
	passert( (pBeamType >= 1) AND (pBeamType <= BEAM_NR) );
	GetBase(pID)->Beams[pBeamType - 1 /* NEW */] = pNumber;
}

 void PutBaseTubes(Uns16 pID, Uns16 pTorpType, Uns16 pNumber)
{
	passert( (pTorpType >= 1) AND (pTorpType <= TORP_NR) );
	GetBase(pID)->Tubes[pTorpType - 1 /* NEW */] = pNumber;
}

 void PutBaseTorps(Uns16 pID, Uns16 pTorpType, Uns16 pNumber)
{
	passert( (pTorpType >= 1) AND (pTorpType <= TORP_NR) );
	GetBase(pID)->Torps[pTorpType - 1 /* NEW */] = pNumber;
}

 void PutBaseBuildOrder(Uns16 pID, BuildOrder_Struct *pOrderPtr)
{
	passert(pOrderPtr NEQ 0);
	memcpy(GetBase(pID)->ToBuild, pOrderPtr, sizeof(BuildOrder_Struct));
}

 void ClearBaseBuildOrder(Uns16 pID)
{
	GetBase(pID)->ToBuild[0] = 0;	/* Enough to clear the hull? */
	BuildQueueInvalidate(pID);
}

 void ClearBaseHulls(Uns16 pID)
{
	Base_Struct *lBase = GetBase(pID);

	memset(lBase->Hulls, 0, sizeof(lBase->Hulls));
}

/* This is used to create a new starbase over a given planet */
/* NEW */
void CreateBase(Uns16 pPlanet)
{
    Base_Struct *lBasePtr;
    Uns16 lCount;

    passert( (pPlanet >= 1) AND (pPlanet <= PLANET_NR) AND !IsBasePresent(pPlanet) );

    gAllBases[pPlanet] = (Base_Struct *) MemCalloc(1, sizeof(Base_Struct));
    lBasePtr = gAllBases[pPlanet];

    lBasePtr->Id = pPlanet;
    lBasePtr->Owner = PlanetOwner(pPlanet);

	for (lCount=0; lCount<4; lCount++) {
        lBasePtr->TechLevel[lCount] = 1;
	}
}

IO_Def Read_Bases_File(Int16* pControl1, Int16* pControl2)
/* control1, control2 : leading/closing shorts in bdata.hst */
{
    FILE         *lBaseFile;
    Int16        i;
    IO_Def       lError = IO_SUCCESS;
    Base_Struct  lBase;

    passert((pControl1 NEQ NULL) AND (pControl2 NEQ NULL));

    InitBases();

    if ((lBaseFile = OpenInputFile(BASE_FILE, GAME_DIR_ONLY | NO_MISSING_ERROR)) NEQ NULL) {
        if (1 NEQ fread(pControl1, sizeof(Uns16), 1, lBaseFile)) {
            Error("Can't read file '%s'", BASE_FILE);
            lError=IO_FAILURE;
        }

        for (i=1; i<=BASE_NR; i++ )
        {
#ifdef __MSDOS__
			if (fread(&lBase, sizeof(Base_Struct), 1, lBaseFile) NEQ 1) {
#else
            memset(&lBase, 0, sizeof(lBase));
			if (! DOSReadStruct(BaseStruct_Convert,
								NumBaseStruct_Convert,
								&lBase,
								lBaseFile)) {
#endif
                Error("Can't read file '%s'", BASE_FILE);
                lError=IO_FAILURE;                       /* i/o error  */
                break;
            } else {
                if (lBase.Owner NEQ 0)
                    PutBase(i, &lBase);
            }
        }

        /* Last word may be missing after editing with PLANMAP etc. */
        *pControl2 = 0;
        (void) fread(pControl2, sizeof(Uns16), 1, lBaseFile);

        fclose(lBaseFile);
    } else lError = IO_FAILURE;
    return(lError);
}

IO_Def Write_Bases_File(Int16 pControl1, Int16 pControl2)
{
    FILE          *lBaseFile;
    Int16         i;
    IO_Def        lError = IO_SUCCESS;
    Base_Struct   lBase;

    if (gAllBases EQ 0) {
        Error("Cannot write bases file without first reading it.\n");
        return IO_FAILURE;
    }

    memset(&lBase, 0, sizeof(lBase));

    if ((lBaseFile = OpenOutputFile(BASE_FILE, GAME_DIR_ONLY)) NEQ NULL) {
        if (1 NEQ fwrite(&pControl1, sizeof(Uns16), 1, lBaseFile)) {
            Error("Can't write to file '%s'", BASE_FILE);
            lError=IO_FAILURE;
        }

        for (i=1; i<=BASE_NR; i++ )
        {
            lBase.Id=i;
#ifdef __MSDOS__
			if (1 NEQ fwrite( ((IsBasePresent(i)) ? GetBase(i) : &lBase),
										sizeof(Base_Struct), 1, lBaseFile)) {
#else
			if (! DOSWriteStruct(BaseStruct_Convert,
								 NumBaseStruct_Convert,
								 IsBasePresent(i) ? GetBase(i) : &lBase,
								 lBaseFile)) {
#endif
                Error("Can't write to file '%s'", BASE_FILE);
                lError=IO_FAILURE;
                break;
            }
        }

        if (lError EQ IO_SUCCESS)
            if (1 NEQ fwrite(&pControl2, sizeof(Uns16), 1, lBaseFile)) {
                Error("Can't write to file '%s'", BASE_FILE);
                lError=IO_FAILURE;
            }

        fclose(lBaseFile);
    } else lError = IO_FAILURE;
    return(lError);
}

void InitializeBases(void)
{
    Uns16 lBase;

    InitBases();

    for (lBase = 1; lBase <= PLANET_NR; lBase++) {
        if (IsBasePresent(lBase)) DeleteBase(lBase);
    }
}

/*************************************************************
  $HISTORY:$
**************************************************************/

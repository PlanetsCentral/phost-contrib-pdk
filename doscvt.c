
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

/* This routine reads a single 16-bit DOS-formatted quantity */
Uns16
ReadDOSUns16(const void *pVal)
{
#ifdef __MSDOS__
  return *(Uns16 *) pVal;
#else
  const Uns8 *lSrc = pVal;
  Uns16 lOutval;

  lOutval = *lSrc++;
  lOutval |= (Uns16) (((Uns16) * lSrc) << 8);

  return lOutval;
#endif
}

Uns32
ReadDOSUns32(const void *pVal)
{
#ifdef __MSDOS__
  return *(Uns32 *) pVal;
#else
  const Uns8 *lSrc = pVal;
  Uns32 lOutval;

  lOutval = *lSrc++;
  lOutval |= (Uns32) (((Uns32) * lSrc++) << 8);
  lOutval |= (Uns32) (((Uns32) * lSrc++) << 16);
  lOutval |= (Uns32) (((Uns32) * lSrc) << 24);

  return lOutval;
#endif
}

Boolean
DOSRead16(Uns16 * pData, Uns16 pNum, FILE * pInFile)
{
#ifdef __MSDOS__
  if (pNum NEQ fread(pData, 2, pNum, pInFile))
    return False;
#else
  unsigned char lData[2];

  while (pNum--) {
    if (2 NEQ fread(lData, 1, 2, pInFile))
      return False;

    *pData++ = ReadDOSUns16(lData);
  }
#endif
  return True;
}

Boolean
DOSRead32(Uns32 * pData, Uns16 pNum, FILE * pInFile)
{
#ifdef __MSDOS__
  if (pNum NEQ fread(pData, 4, pNum, pInFile))
    return False;
#else
  unsigned char lData[4];

  while (pNum--) {
    if (4 NEQ fread(lData, 1, 4, pInFile))
      return False;

    *pData++ = ReadDOSUns32(lData);
  }
#endif
  return True;
}

Boolean
DOSWrite16(const Uns16 * pData, Uns16 pNum, FILE * pOutFile)
{
#ifdef __MSDOS__
  if (pNum NEQ fwrite(pData, 2, pNum, pOutFile))
    return False;
#else
  Uns16 lVal;

  while (pNum--) {
    lVal = *pData++;
    if (EOF EQ fputc((lVal & 0xFFU), pOutFile))
      return False;
    if (EOF EQ fputc((lVal >> 8) & 0xFFU, pOutFile))
      return False;
  }
#endif
  return True;
}

Boolean
DOSWrite32(const Uns32 * pData, Uns16 pNum, FILE * pOutFile)
{
#ifdef __MSDOS__
  if (pNum NEQ fwrite(pData, 4, pNum, pOutFile))
    return False;
#else
  Uns32 lVal;

  while (pNum--) {
    lVal = *pData++;
    if (EOF EQ fputc((int) (lVal & 0xFFU), pOutFile))
      return False;
    if (EOF EQ fputc((int) ((lVal >> 8) & 0xFFU), pOutFile))
      return False;
    if (EOF EQ fputc((int) ((lVal >> 16) & 0xFFU), pOutFile))
      return False;
    if (EOF EQ fputc((int) ((lVal >> 24) & 0xFFU), pOutFile))
      return False;
  }
#endif
  return True;
}

#pragma warn -par

void
EndianSwap32(void *pData, Uns16 pNumber)
{
#ifdef BIG_ENDIAN               /* NEW */
  Uns32 lVal;

  while (pNumber-- > 0) {
    lVal = ReadDOSUns32(pData);
    memcpy(pData, &lVal, 4);
    pData = ((char *) pData) + 4;
  }
#endif
}

void
EndianSwap16(void *pData, Uns16 pNumber)
{
#ifdef BIG_ENDIAN               /* NEW */
  Uns16 lVal;

  while (pNumber-- > 0) {
    lVal = ReadDOSUns16(pData);
    memcpy(pData, &lVal, 2);
    pData = ((char *) pData) + 2;
  }
#endif
}

Boolean
DOSReadStruct(const DOSConvertElement * pStruct, Uns16 pNumElements,
      void *pData, FILE * pInFile)
{
  Uns16 lCount;

  for (lCount = 0; lCount < pNumElements; lCount++) {
    char *lData = ((char *) pData) + pStruct[lCount].mOffset;

    switch (pStruct[lCount].mType) {
    case DOSCVT_char:
      if (1 NEQ fread(lData, pStruct[lCount].mSize, 1, pInFile))
        goto bad_read;
      break;

    case DOSCVT_Uns16:
      if (!DOSRead16((Uns16 *) lData, 1, pInFile))
        goto bad_read;
      break;

    case DOSCVT_Uns32:
      if (!DOSRead32((Uns32 *) lData, 1, pInFile))
        goto bad_read;
      break;

    case DOSCVT_enum:
      /* Enum's in DOS format are 16 bits, but ours may be larger */
      switch (sizeof(DOSCVT_Def)) {
      case 2:
        if (!DOSRead16((Uns16 *) lData, 1, pInFile))
          goto bad_read;
        break;
      case 4:
        /* Must convert 2-byte DOS word to 4-byte enum on our machine */
        {
          Uns16 lEnumVal;

          if (!DOSRead16(&lEnumVal, 1, pInFile))
            goto bad_read;
          *(DOSCVT_Def *) lData = lEnumVal;
        }
        break;
      default:
        passert(0);
      }
      break;

    default:
      passert(0);
    }
  }

  return True;

bad_read:

/*    ErrorExit("Unable to read from DOS file"); */
  return False;
}

/* This routine is used by Unix hosts to write out to a file a structure that's
   stored in memory in native format by first converting it to DOS format */

Boolean
DOSWriteStruct(const DOSConvertElement * pStruct, Uns16 pNumElements,
      const void *pData, FILE * pOutFile)
{
  Uns16 lCount;

  for (lCount = 0; lCount < pNumElements; lCount++) {
    char *lData = ((char *) pData) + pStruct[lCount].mOffset;

    switch (pStruct[lCount].mType) {
    case DOSCVT_char:
      if (1 NEQ fwrite(lData, pStruct[lCount].mSize, 1, pOutFile))
        goto bad_write;
      break;

    case DOSCVT_Uns16:
      if (!DOSWrite16((Uns16 *) lData, 1, pOutFile))
        goto bad_write;
      break;

    case DOSCVT_Uns32:
      if (!DOSWrite32((Uns32 *) lData, 1, pOutFile))
        goto bad_write;
      break;

    case DOSCVT_enum:
      /* On our machine, enums had better be word-sized, like DOS, or
         longword-sized, like most 32-bit machines */
      switch (sizeof(DOSCVT_Def)) {
      case 2:
        if (!DOSWrite16((Uns16 *) lData, 1, pOutFile))
          goto bad_write;
        break;
      case 4:
        /* Must convert 4-byte enum on our machine to 2-byte enum in DOS */
        {
          Uns16 lEnumVal = (Uns16) (*(DOSCVT_Def *) lData);

          if (!DOSWrite16(&lEnumVal, 1, pOutFile))
            goto bad_write;
        }
        break;
      default:
        passert(0);
      }
      break;

    default:
      passert(0);
    }
  }

  return True;

bad_write:
  /* ErrorExit("Unable to write to DOS file"); */
  return False;
}

static
void DOS16Convert(const void *pData, Uns16 pNum, void *pOut)
{
#ifdef LITTLE_ENDIAN
  passert(pData NEQ NULL);
  passert(pOut NEQ NULL);
  passert(pNum NEQ 0);
  
  memcpy(pOut, pData, pNum*sizeof(Uns16));
#else
  Uns16 *pDst = pOut;
  const Uns8  *pSrc = pData;
  Uns16 lVal;
  
  passert(pData NEQ NULL);
  passert(pOut NEQ NULL);
  passert(pNum NEQ 0);
  
  while (pNum--) {
    lVal  = (Uns16) *pSrc++;
    lVal |= ((Uns16) (*pSrc++)) << 8;
    memcpy(pDst, &lVal, sizeof(Uns16));
    pDst++;
  }
#endif
}

/* This routine is used to read 32-bit DOS data into the target Unix format */

static void
DOS32Convert(const void *pData, Uns16 pNum, void *pOut)
{
#ifdef LITTLE_ENDIAN
  memcpy(pOut, pData, pNum*sizeof(Uns32));
#else
  Uns32 *pDst = pOut;
  const Uns8  *pSrc = pData;
  Uns32 lVal;
  
  while (pNum--) {
    lVal  = (Uns32) *pSrc++;
    lVal |= ((Uns32) (*pSrc++)) << 8;
    lVal |= ((Uns32) (*pSrc++)) << 16;
    lVal |= ((Uns32) (*pSrc++)) << 24;
    memcpy(pDst, &lVal, sizeof(Uns32));
    pDst++;
  }
#endif
}

/* This routine converts a DOS-sized structure to the target format. */
Boolean
DOSStructConvert(const DOSConvertElement *pStruct, Uns16 pNumElements,
                 const void *pSrc, void *pDst)
{
  Uns16 lCount;
  
  for (lCount = 0; lCount < pNumElements; lCount++) {
    char *lData = ((char *)pDst) + pStruct[lCount].mOffset;
    
    switch (pStruct[lCount].mType) {
    case DOSCVT_char:
      memcpy(lData, (const char *)pSrc, pStruct[lCount].mSize);
      pSrc = ((char *)pSrc) + pStruct[lCount].mSize;
      break;
      
    case DOSCVT_enum:
      /* In DOS, enums are word-sized (2 bytes). Ours may be bigger
         so we must set it to 0 to ensure that higher order words aren't
         garbled. This is important for checksum generation, too. */
      {
        DOSCVT_Def lJunk = 0;
        lJunk = ReadDOSUns16(pSrc);
        memcpy(lData, &lJunk, sizeof(lJunk));
      }
      pSrc = ((Uns16 *)pSrc) + 1;
      break;
       
    case DOSCVT_Uns16:
      DOS16Convert(pSrc, 1, lData);
      pSrc = ((Uns16 *)pSrc) + 1;
      break;
        
    case DOSCVT_Uns32:
      DOS32Convert(pSrc, 1, lData);
      pSrc = ((Uns32 *)pSrc) + 1;
      break;
      
    default:
      passert(0);
    }
  }
  
  return True;
}

/* This routine converts a Unix structure into DOS format */

void
UnixConvertStruct(const DOSConvertElement *pStruct, Uns16 pNumElements,
                  const void *pData, void *pDst)
{
  Uns16 lCount;
  char *lDst = (char *) pDst;
  
  for (lCount = 0; lCount < pNumElements; lCount++) {
    char *lData = ((char *)pData) + pStruct[lCount].mOffset;
    
    switch (pStruct[lCount].mType) {
    case DOSCVT_char:
      memcpy(lDst, lData, pStruct[lCount].mSize);
      lDst += pStruct[lCount].mSize;
      break;
       
    case DOSCVT_Uns16:
      *(Uns16 *)lDst = *(Uns16 *)lData;
      EndianSwap16(lDst, 1);
      lDst += sizeof(Uns16);
      break;
      
    case DOSCVT_Uns32:
      *(Uns32 *)lDst = *(Uns32 *)lData;
      EndianSwap32(lDst, 1);
      lDst += sizeof(Uns32);
      break;
        
    case DOSCVT_enum:
      *(Uns16 *)lDst = *(DOSCVT_Def *)lData;
      EndianSwap16(lDst, 1);
      lDst += sizeof(Uns16);
      break;
      
    default:
      passert(0);
    }
  }
}

/* This routine returns the size in bytes of an equivalent DOS structure. */

Uns16
DOSStructSize(const DOSConvertElement *pStruct, Uns16 pNumElem)
{
  Uns16 lCount;
  Uns16 lSize = 0;
  
  if (pStruct EQ 0 OR pNumElem EQ 0)
    return 0;
  
  for (lCount = 0; lCount < pNumElem; lCount++) {
    switch (pStruct[lCount].mType) {
    case DOSCVT_char:
      lSize += pStruct[lCount].mSize;
      break;
      
    case DOSCVT_enum:
    case DOSCVT_Uns16:
      lSize += 2;
      break;
        
    case DOSCVT_Uns32:
      lSize += 4;
      break;
      
    default:
      passert(0);
    }
  }
  
  return lSize;
}

#pragma warn .par

/*
 *        S T R U C T U R E    C O N V E R S I O N    D E F I N I T I O N S
 */

#undef DOSCVT_NAME
#define DOSCVT_NAME Ship_Struct
const DOSConvertElement ShipStruct_Convert[] = {
  DOSCVTUns16(Id),
  DOSCVTenum(Owner),
  DOSCVTchar(FriendlyCode, 3),
  DOSCVTUns16(Speed),
  DOSCVTARRAYUns16(Waypoint, 0),
  DOSCVTARRAYUns16(Waypoint, 1),
  DOSCVTARRAYUns16(Location, 0),
  DOSCVTARRAYUns16(Location, 1),
  DOSCVTUns16(Engine),
  DOSCVTUns16(Hull),
  DOSCVTUns16(BeamType),
  DOSCVTUns16(BeamNumber),
  DOSCVTUns16(Bays),
  DOSCVTUns16(TorpType),
  DOSCVTUns16(Ammunition),
  DOSCVTUns16(TubeNumber),
  DOSCVTenum(Mission),
  DOSCVTenum(Enemy),
  DOSCVTUns16(TowTarget),
  DOSCVTUns16(Damage),
  DOSCVTUns16(Crew),
  DOSCVTUns16(Colonists),
  DOSCVTchar(Name, 20),
  DOSCVTARRAYUns16(Cargo, 0),
  DOSCVTARRAYUns16(Cargo, 1),
  DOSCVTARRAYUns16(Cargo, 2),
  DOSCVTARRAYUns16(Cargo, 3),
  DOSCVTARRAYUns16(Cargo, 4),
  DOSCVTARRAYUns16(Dump, 0),
  DOSCVTARRAYUns16(Dump, 1),
  DOSCVTARRAYUns16(Dump, 2),
  DOSCVTARRAYUns16(Dump, 3),
  DOSCVTARRAYUns16(Dump, 4),
  DOSCVTARRAYUns16(Dump, 5),
  DOSCVTUns16(DumpPlanet),
  DOSCVTARRAYUns16(Transfer, 0),
  DOSCVTARRAYUns16(Transfer, 1),
  DOSCVTARRAYUns16(Transfer, 2),
  DOSCVTARRAYUns16(Transfer, 3),
  DOSCVTARRAYUns16(Transfer, 4),
  DOSCVTARRAYUns16(Transfer, 5),
  DOSCVTUns16(TransferShip),
  DOSCVTUns16(InterceptTarget),
  DOSCVTUns16(Credits)
};
const Uns16 NumShipStruct_Convert =
      sizeof(ShipStruct_Convert) / sizeof(ShipStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME Planet_Struct
const DOSConvertElement PlanetStruct_Convert[] = {
  DOSCVTenum(Owner),
  DOSCVTUns16(Id),
  DOSCVTchar(FriendlyCode, 3),
  DOSCVTUns16(Mines),
  DOSCVTUns16(Factories),
  DOSCVTUns16(Defense),
  DOSCVTARRAYUns32(Cargo, 0),
  DOSCVTARRAYUns32(Cargo, 1),
  DOSCVTARRAYUns32(Cargo, 2),
  DOSCVTARRAYUns32(Cargo, 3),
  DOSCVTARRAYUns32(Cargo, 4),
  DOSCVTARRAYUns32(Cargo, 5),
  DOSCVTARRAYUns32(Cargo, 6),
  DOSCVTARRAYUns32(Core, 0),
  DOSCVTARRAYUns32(Core, 1),
  DOSCVTARRAYUns32(Core, 2),
  DOSCVTARRAYUns32(Core, 3),
  DOSCVTARRAYUns16(Density, 0),
  DOSCVTARRAYUns16(Density, 1),
  DOSCVTARRAYUns16(Density, 2),
  DOSCVTARRAYUns16(Density, 3),
  DOSCVTUns16(ColTax),
  DOSCVTUns16(NatTax),
  DOSCVTUns16(ColHappy),
  DOSCVTUns16(NatHappy),
  DOSCVTenum(Govm),
  DOSCVTUns32(NativePopulation),
  DOSCVTenum(NativeType),
  DOSCVTUns16(Temp),
  DOSCVTBoolean(BuildBase)
};
const Uns16 NumPlanetStruct_Convert =
      sizeof(PlanetStruct_Convert) / sizeof(PlanetStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME Base_Struct
const DOSConvertElement BaseStruct_Convert[] = {
  DOSCVTUns16(Id),
  DOSCVTenum(Owner),
  DOSCVTUns16(Defense),
  DOSCVTUns16(Damage),
  DOSCVTARRAYUns16(TechLevel, 0),
  DOSCVTARRAYUns16(TechLevel, 1),
  DOSCVTARRAYUns16(TechLevel, 2),
  DOSCVTARRAYUns16(TechLevel, 3),
  DOSCVTARRAYUns16(Engines, 0),
  DOSCVTARRAYUns16(Engines, 1),
  DOSCVTARRAYUns16(Engines, 2),
  DOSCVTARRAYUns16(Engines, 3),
  DOSCVTARRAYUns16(Engines, 4),
  DOSCVTARRAYUns16(Engines, 5),
  DOSCVTARRAYUns16(Engines, 6),
  DOSCVTARRAYUns16(Engines, 7),
  DOSCVTARRAYUns16(Engines, 8),
  DOSCVTARRAYUns16(Hulls, 0),
  DOSCVTARRAYUns16(Hulls, 1),
  DOSCVTARRAYUns16(Hulls, 2),
  DOSCVTARRAYUns16(Hulls, 3),
  DOSCVTARRAYUns16(Hulls, 4),
  DOSCVTARRAYUns16(Hulls, 5),
  DOSCVTARRAYUns16(Hulls, 6),
  DOSCVTARRAYUns16(Hulls, 7),
  DOSCVTARRAYUns16(Hulls, 8),
  DOSCVTARRAYUns16(Hulls, 9),
  DOSCVTARRAYUns16(Hulls, 10),
  DOSCVTARRAYUns16(Hulls, 11),
  DOSCVTARRAYUns16(Hulls, 12),
  DOSCVTARRAYUns16(Hulls, 13),
  DOSCVTARRAYUns16(Hulls, 14),
  DOSCVTARRAYUns16(Hulls, 15),
  DOSCVTARRAYUns16(Hulls, 16),
  DOSCVTARRAYUns16(Hulls, 17),
  DOSCVTARRAYUns16(Hulls, 18),
  DOSCVTARRAYUns16(Hulls, 19),
  DOSCVTARRAYUns16(Beams, 0),
  DOSCVTARRAYUns16(Beams, 1),
  DOSCVTARRAYUns16(Beams, 2),
  DOSCVTARRAYUns16(Beams, 3),
  DOSCVTARRAYUns16(Beams, 4),
  DOSCVTARRAYUns16(Beams, 5),
  DOSCVTARRAYUns16(Beams, 6),
  DOSCVTARRAYUns16(Beams, 7),
  DOSCVTARRAYUns16(Beams, 8),
  DOSCVTARRAYUns16(Beams, 9),
  DOSCVTARRAYUns16(Tubes, 0),
  DOSCVTARRAYUns16(Tubes, 1),
  DOSCVTARRAYUns16(Tubes, 2),
  DOSCVTARRAYUns16(Tubes, 3),
  DOSCVTARRAYUns16(Tubes, 4),
  DOSCVTARRAYUns16(Tubes, 5),
  DOSCVTARRAYUns16(Tubes, 6),
  DOSCVTARRAYUns16(Tubes, 7),
  DOSCVTARRAYUns16(Tubes, 8),
  DOSCVTARRAYUns16(Tubes, 9),
  DOSCVTARRAYUns16(Torps, 0),
  DOSCVTARRAYUns16(Torps, 1),
  DOSCVTARRAYUns16(Torps, 2),
  DOSCVTARRAYUns16(Torps, 3),
  DOSCVTARRAYUns16(Torps, 4),
  DOSCVTARRAYUns16(Torps, 5),
  DOSCVTARRAYUns16(Torps, 6),
  DOSCVTARRAYUns16(Torps, 7),
  DOSCVTARRAYUns16(Torps, 8),
  DOSCVTARRAYUns16(Torps, 9),
  DOSCVTUns16(Fighters),
  DOSCVTUns16(FixingShip),
  DOSCVTenum(FixOrder),
  DOSCVTenum(Order),
  DOSCVTARRAYUns16(ToBuild, 0),
  DOSCVTARRAYUns16(ToBuild, 1),
  DOSCVTARRAYUns16(ToBuild, 2),
  DOSCVTARRAYUns16(ToBuild, 3),
  DOSCVTARRAYUns16(ToBuild, 4),
  DOSCVTARRAYUns16(ToBuild, 5),
  DOSCVTARRAYUns16(ToBuild, 6),
};
const Uns16 NumBaseStruct_Convert =
      sizeof(BaseStruct_Convert) / sizeof(BaseStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME MineField_Struct
const DOSConvertElement MineStruct_Convert[] = {
  DOSCVTARRAYUns16(Position, 0),
  DOSCVTARRAYUns16(Position, 1),
  DOSCVTenum(Race),
  DOSCVTUns32(Units),
  DOSCVTenum(Web)
};
const Uns16 NumMineStruct_Convert =
      sizeof(MineStruct_Convert) / sizeof(MineStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME Hullspec_Struct
const DOSConvertElement HullspecStruct_Convert[] = {
  DOSCVTchar(Name, 30),
  DOSCVTUns16(Picnumber),
  DOSCVTUns16(Bla),
  DOSCVTUns16(TritCost),
  DOSCVTUns16(DurCost),
  DOSCVTUns16(MolyCost),
  DOSCVTUns16(FuelCapacity),
  DOSCVTUns16(CrewComplement),
  DOSCVTUns16(EngineNumber),
  DOSCVTUns16(Mass),
  DOSCVTUns16(TechLevel),
  DOSCVTUns16(CargoCapacity),
  DOSCVTUns16(Bays),
  DOSCVTUns16(TubeNumber),
  DOSCVTUns16(BeamNumber),
  DOSCVTUns16(MoneyCost)
};
const Uns16 NumHullspecStruct_Convert =
      sizeof(HullspecStruct_Convert) / sizeof(HullspecStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME Turntime_Struct
const DOSConvertElement TurntimeStruct_Convert[] = {
  DOSCVTchar(HostTime, 18),
  DOSCVTUns16(TurnNumber)
};
const Uns16 NumTurntimeStruct_Convert =
      sizeof(TurntimeStruct_Convert) / sizeof(TurntimeStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME BaseOrder_Struct
const DOSConvertElement BaseOrderStruct_Convert[] = {
  DOSCVTUns16(mBase),
  DOSCVTUns16(mOrder.mHull),
  DOSCVTUns16(mOrder.mEngineType),
  DOSCVTUns16(mOrder.mBeamType),
  DOSCVTUns16(mOrder.mNumBeams),
  DOSCVTUns16(mOrder.mTubeType),
  DOSCVTUns16(mOrder.mNumTubes),
  DOSCVTenum(mByCloning),
  DOSCVTenum(mShipOwner),
  DOSCVTUns32(mPriority),
  DOSCVTchar(mReserved, 4)
};
const Uns16 NumBaseOrderStruct_Convert =
      sizeof(BaseOrderStruct_Convert) / sizeof(BaseOrderStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME HostGen_Struct
const DOSConvertElement HostGenStruct_Convert[] = {
  DOSCVTchar(Bla, 24),
  DOSCVTchar(WasUsingWinPlan, OLD_RACE_NR),
  DOSCVTchar(WasRegistered, OLD_RACE_NR),
  DOSCVTARRAYBoolean(Players, 0),
  DOSCVTARRAYBoolean(Players, 1),
  DOSCVTARRAYBoolean(Players, 2),
  DOSCVTARRAYBoolean(Players, 3),
  DOSCVTARRAYBoolean(Players, 4),
  DOSCVTARRAYBoolean(Players, 5),
  DOSCVTARRAYBoolean(Players, 6),
  DOSCVTARRAYBoolean(Players, 7),
  DOSCVTARRAYBoolean(Players, 8),
  DOSCVTARRAYBoolean(Players, 9),
  DOSCVTARRAYBoolean(Players, 10),
  DOSCVTchar(Passwd1, 110),
  DOSCVTchar(Passwd2, 110)
};
const Uns16 NumHostGenStruct_Convert =
      sizeof(HostGenStruct_Convert) / sizeof(HostGenStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME Engspec_Struct
const DOSConvertElement EngspecStruct_Convert[] = {
  DOSCVTchar(Name, 20),
  DOSCVTUns16(MoneyCost),
  DOSCVTUns16(TritCost),
  DOSCVTUns16(DurCost),
  DOSCVTUns16(MolyCost),
  DOSCVTUns16(TechLevel),
  DOSCVTARRAYUns32(FuelConsumption, 0),
  DOSCVTARRAYUns32(FuelConsumption, 1),
  DOSCVTARRAYUns32(FuelConsumption, 2),
  DOSCVTARRAYUns32(FuelConsumption, 3),
  DOSCVTARRAYUns32(FuelConsumption, 4),
  DOSCVTARRAYUns32(FuelConsumption, 5),
  DOSCVTARRAYUns32(FuelConsumption, 6),
  DOSCVTARRAYUns32(FuelConsumption, 7),
  DOSCVTARRAYUns32(FuelConsumption, 8),
};
const Uns16 NumEngspecStruct_Convert =
      sizeof(EngspecStruct_Convert) / sizeof(EngspecStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME Beamspec_Struct
const DOSConvertElement BeamspecStruct_Convert[] = {
  DOSCVTchar(Name, 20),
  DOSCVTUns16(MoneyCost),
  DOSCVTUns16(TritCost),
  DOSCVTUns16(DurCost),
  DOSCVTUns16(MolyCost),
  DOSCVTUns16(Mass),
  DOSCVTUns16(TechLevel),
  DOSCVTUns16(KillPower),
  DOSCVTUns16(DestructivePower)
};
const Uns16 NumBeamspecStruct_Convert =
      sizeof(BeamspecStruct_Convert) / sizeof(BeamspecStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME Torpspec_Struct
const DOSConvertElement TorpspecStruct_Convert[] = {
  DOSCVTchar(Name, 20),
  DOSCVTUns16(TorpCost),
  DOSCVTUns16(TubeCost),
  DOSCVTUns16(TubeTritCost),
  DOSCVTUns16(TubeDurCost),
  DOSCVTUns16(TubeMolyCost),
  DOSCVTUns16(TubeMass),
  DOSCVTUns16(TechLevel),
  DOSCVTUns16(KillPower),
  DOSCVTUns16(DestructivePower)
};
const Uns16 NumTorpspecStruct_Convert =
      sizeof(TorpspecStruct_Convert) / sizeof(TorpspecStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME XYData_Struct
const DOSConvertElement XyplanStruct_Convert[] = {
  DOSCVTUns16(X),
  DOSCVTUns16(Y),
  DOSCVTenum(Owner)
};
const Uns16 NumXyplanStruct_Convert =
      sizeof(XyplanStruct_Convert) / sizeof(XyplanStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME Shipxy_Struct
const DOSConvertElement ShipxyStruct_Convert[] = {
  DOSCVTARRAYUns16(Position, 0),
  DOSCVTARRAYUns16(Position, 1),
  DOSCVTenum(Owner),
  DOSCVTUns16(Mass)
};
const Uns16 NumShipxyStruct_Convert =
      sizeof(ShipxyStruct_Convert) / sizeof(ShipxyStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME Vcr_Struct
const DOSConvertElement VcrStruct_Convert[] = {
  DOSCVTUns16(Random1),
  DOSCVTBoolean(Dummy),
  DOSCVTUns16(Planetpic),
  DOSCVTBoolean(Is_planet2),
  DOSCVTUns16(Mass1),
  DOSCVTUns16(Mass2),
  DOSCVTchar(Ship1.Name, 20),
  DOSCVTUns16(Ship1.Damage),
  DOSCVTUns16(Ship1.Crew),
  DOSCVTUns16(Ship1.Id),
  DOSCVTenum(Ship1.Race),
  DOSCVTchar(Ship1.Picnumber, 1),
  DOSCVTchar(Ship1.Hullnumber, 1),
  DOSCVTUns16(Ship1.Beamtype),
  DOSCVTUns16(Ship1.Beamnumber),
  DOSCVTUns16(Ship1.Bays),
  DOSCVTUns16(Ship1.Torps),
  DOSCVTUns16(Ship1.Ammunition),
  DOSCVTUns16(Ship1.Tubes),
  DOSCVTchar(Ship2.Name, 20),
  DOSCVTUns16(Ship2.Damage),
  DOSCVTUns16(Ship2.Crew),
  DOSCVTUns16(Ship2.Id),
  DOSCVTenum(Ship2.Race),
  DOSCVTchar(Ship2.Picnumber, 1),
  DOSCVTchar(Ship2.Hullnumber, 1),
  DOSCVTUns16(Ship2.Beamtype),
  DOSCVTUns16(Ship2.Beamnumber),
  DOSCVTUns16(Ship2.Bays),
  DOSCVTUns16(Ship2.Torps),
  DOSCVTUns16(Ship2.Ammunition),
  DOSCVTUns16(Ship2.Tubes),
  DOSCVTUns16(Shield1),
  DOSCVTUns16(Shield2)
};
const Uns16 NumVcrStruct_Convert =
      sizeof(VcrStruct_Convert) / sizeof(VcrStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME TargetShip_Struct
const DOSConvertElement TargetShipStruct_Convert[] = {
  DOSCVTUns16(Id),
  DOSCVTenum(Owner),
  DOSCVTUns16(Speed),
  DOSCVTARRAYUns16(Location, 0),
  DOSCVTARRAYUns16(Location, 1),
  DOSCVTUns16(Hull),
  DOSCVTUns16(Heading),
  DOSCVTchar(Name, 20)
};
const Uns16 NumTargetShipStruct_Convert =
      sizeof(TargetShipStruct_Convert) / sizeof(TargetShipStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME ExtMessage_Struct
const DOSConvertElement ExtMessageStruct_Convert[] = {
  DOSCVTenum(mRace),
  DOSCVTUns32(mFilePos),
  DOSCVTUns16(mLength)
};
const Uns16 NumExtMessageStruct_Convert =
      sizeof(ExtMessageStruct_Convert) / sizeof(ExtMessageStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME UFO_Struct
const DOSConvertElement UFOStruct_Convert[] = {
  DOSCVTUns16(mColor),
  DOSCVTchar(mName, 20),
  DOSCVTchar(mInfo1, 20),
  DOSCVTchar(mInfo2, 20),
  DOSCVTUns16(mX),
  DOSCVTUns16(mY),
  DOSCVTUns16(mSpeed),
  DOSCVTUns16(mHeading),
  DOSCVTUns16(mPRange),
  DOSCVTUns16(mSRange),
  DOSCVTUns16(mRadius),
  DOSCVTUns16(mParentID)
};
const Uns16 NumUFOStruct_Convert =
      sizeof(UFOStruct_Convert) / sizeof(UFOStruct_Convert[0]);

#undef DOSCVT_NAME
#define DOSCVT_NAME Tons_Struct
const DOSConvertElement TonsStruct_Convert[] = {
  DOSCVTARRAYUns32(mTotalTons, 0),
  DOSCVTARRAYUns32(mTotalTons, 1),
  DOSCVTARRAYUns32(mTotalTons, 2),
  DOSCVTARRAYUns32(mTotalTons, 3),
  DOSCVTARRAYUns32(mTotalTons, 4),
  DOSCVTARRAYUns32(mTotalTons, 5),
  DOSCVTARRAYUns32(mTotalTons, 6),
  DOSCVTARRAYUns32(mTotalTons, 7),
  DOSCVTARRAYUns32(mTotalTons, 8),
  DOSCVTARRAYUns32(mTotalTons, 9),
  DOSCVTARRAYUns32(mTotalTons, 10),
  DOSCVTARRAYUns32(mCurrentTons, 0),
  DOSCVTARRAYUns32(mCurrentTons, 1),
  DOSCVTARRAYUns32(mCurrentTons, 2),
  DOSCVTARRAYUns32(mCurrentTons, 3),
  DOSCVTARRAYUns32(mCurrentTons, 4),
  DOSCVTARRAYUns32(mCurrentTons, 5),
  DOSCVTARRAYUns32(mCurrentTons, 6),
  DOSCVTARRAYUns32(mCurrentTons, 7),
  DOSCVTARRAYUns32(mCurrentTons, 8),
  DOSCVTARRAYUns32(mCurrentTons, 9),
  DOSCVTARRAYUns32(mCurrentTons, 10)
};
const Uns16 NumTonsStruct_Convert;

/*************************************************************
  $HISTORY:$
**************************************************************/

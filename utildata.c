#include <stdio.h>
#include "phostpdk.h"
#include "private.h"

/* utilx.dat member header */
typedef struct {
  Uns16 mRecordId;
  Uns16 mSize;
} UtilHeader_Struct;

#undef DOSCVT_NAME
#define DOSCVT_NAME UtilHeader_Struct
const DOSConvertElement UtilHeaderStruct_Convert[] = {
  DOSCVTUns16(mRecordId),
  DOSCVTUns16(mSize)
};
const Uns16 NumUtilHeaderStruct_Convert = sizeof(UtilHeaderStruct_Convert)/sizeof(UtilHeaderStruct_Convert[0]);

static UtilMode_Def gUtilMode = UTIL_Ext;

/*
 *  Set mode (=util.dat or .ext)
 */
void
SetUtilMode(UtilMode_Def pDat)
{
  gUtilMode = pDat;
}

/*
 *   Send utilx.dat record, simple version:
 *     pRace     = receiver race
 *     pRecordId = record Id
 *     pSize     = size of record
 *     pData     = data (one big chunk)
 */
Boolean
PutUtilRecordSimple(RaceType_Def pRace, Uns16 pRecordId,
                    Uns16 pSize, void* pData)
{
  return PutUtilRecord(pRace, pRecordId, 1, &pSize, &pData);
}

/*
 *   Send utilx.dat record, struct:
 *     pRace     = receiver race
 *     pRecordId = record Id
 */
Boolean
PutUtilRecordStruct(RaceType_Def pRace, Uns16 pRecordId,
                    void* pData, const DOSConvertElement *pStruct,
                    Uns16 pNumElements)
{
  Uns16 lSize;
  void* lData;
  Boolean lRet;

  lSize = DOSStructSize(pStruct, pNumElements);
  lData = MemAlloc(lSize);
  if(!lData)
    return False;

  DOSStructConvert(pStruct, pNumElements, pData, lData);
  lRet = PutUtilRecord(pRace, pRecordId, 1, &lSize, &lData);
  MemFree(lData);

  return lRet;
}

/*
 *   Send utilx.dat record, complicated version:
 *     pRace     = receiver race
 *     pRecordId = record id
 *     pCount    = number of records in the following arrays
 *     pSizes    = array of byte counts
 *     pData     = array of data blocks
 *   This one supports a `gather' operation to gather information from
 *   more than one block.
 */
Boolean
PutUtilRecord(RaceType_Def pRace, Uns16 pRecordId, Uns16 pCount,
              Uns16* pSizes, void** pData)
{
  UtilHeader_Struct lHead;    /* header */
  char lName[20];             /* file name */
  FILE* fp;                   /* file ptr */
  Boolean lRes;               /* result to return */
  int i;

  /* compute size of record; directly in header */
  lHead.mRecordId = pRecordId;
  lHead.mSize     = 0;
  for(i = 0; i < pCount; ++i) {
    lHead.mSize += pSizes[i];
    passert(lHead.mSize >= pSizes[i]);  /* prevent overflow */
  }

  sprintf(lName, "util%d.%s", pRace, gUtilMode==UTIL_Dat ? "dat" : "ext");
  fp = OpenOutputFile(lName, GAME_DIR_ONLY | NO_MISSING_ERROR | APPEND_MODE);
  if(!fp)
    return False;

  lRes = DOSWriteStruct(UtilHeaderStruct_Convert,
                        NumUtilHeaderStruct_Convert,
                        &lHead, fp);
  for(i = 0; i < pCount; ++i)
    fwrite(pData[i], pSizes[i], 1, fp);
  
  fclose(fp);
  return lRes;
}

/*
 *   Send utilx.dat file transfer:
 *     pRace     = Receiver race
 *     pName     = File name (up to 12 characters)
 *     pText     = set this if the file to be sent is text
 *     pSize     = size of file
 *     pData     = file content
 */
Boolean
PutUtilFileTransfer(RaceType_Def pRace, const char* pName,
                    Boolean pText,
                    Uns16 pSize, void* pData)
{
  char lHeader[13];    /* 12 name, 1 flag */
  void* lPtrs[2];
  Uns16 lSizes[2];
  int i;

  /* get basename */
  while(1) {
    size_t x = strcspn(pName, "/\\:");
    if(pName[x])
      pName += x+1;
    else
      break;
  }

  for(i = 0; i < 12; ++i)
    lHeader[i] = *pName ? *pName++ : ' ';
  lHeader[12] = pText;

  lPtrs[0] = &lHeader;
  lPtrs[1] = pData;
  lSizes[0] = sizeof(lHeader);
  lSizes[1] = pSize;

  return PutUtilRecord(pRace, 34, 2, lSizes, lPtrs);
}

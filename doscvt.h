#ifndef _DOSCVT_H_
#define _DOSCVT_H_

typedef enum {
  DOSCVT_char = 0,              /* 1 byte */
  DOSCVT_Uns16 = 1,             /* 2 bytes, little endian */
  DOSCVT_Int16 = 1,             /* 2 bytes, little endian */
  DOSCVT_Uns32 = 2,             /* 4 bytes, little endian */
  DOSCVT_Int32 = 2,             /* 4 bytes, little endian */
  DOSCVT_enum = 3,              /* enumeration, 2 bytes, little endian */
  DOSCVT_Boolean = 3            /* same as an enum */
} DOSCVT_Def;

/* This structure must have both a structure tag and a typedef name,
   so that it can be forward-declared in phostpdk.h. This way, people
   who do not want the DOS I/O functions need not pull them into their
   namespace. --streu */
typedef struct DOSConvertElement {
  size_t mOffset;
  Uns16 mSize;                  /* for char arguments */
  DOSCVT_Def mType;
} DOSConvertElement;

#define DOSCVT(type,item,num) { offsetof(DOSCVT_NAME, item), num, DOSCVT_##type }
#define DOSCVTchar(item, size) DOSCVT(char, item, size)
#define DOSCVTUns16(item) DOSCVT(Uns16, item, 1)
#define DOSCVTUns32(item) DOSCVT(Uns32, item, 1)
#define DOSCVTenum(item) DOSCVT(enum, item, 1)
#define DOSCVTBoolean(item) DOSCVT(Boolean, item, 1)
#define DOSCVTARRAYUns16(item, elem) DOSCVT(Uns16, item[elem], 1)
#define DOSCVTARRAYUns32(item, elem) DOSCVT(Uns32, item[elem], 1)
#define DOSCVTARRAYBoolean(item, elem) DOSCVT(Boolean, item[elem], 1)
#define DOSCVTARRAYchar(item, elem, size) DOSCVT(char, item[elem], size)

extern Boolean DOSWriteStruct(const DOSConvertElement * pStruct,
      Uns16 pNumElements, const void *pData, FILE * pOutFile);
extern Boolean DOSReadStruct(const DOSConvertElement * pStruct,
      Uns16 pNumElements, void *pData, FILE * pInFile);
extern Uns16 DOSStructSize(const DOSConvertElement *pStruct, Uns16 pNumElem);
extern Boolean DOSStructConvert(const DOSConvertElement *pStruct,
      Uns16 pNumElements, const void *pSrc, void *pDst);
extern void UnixConvertStruct(const DOSConvertElement *pStruct,
      Uns16 pNumElements, const void *pData, void *pDst);

/* Some structure definitions */
extern const DOSConvertElement ShipStruct_Convert[];
extern const Uns16 NumShipStruct_Convert;
extern const DOSConvertElement PlanetStruct_Convert[];
extern const Uns16 NumPlanetStruct_Convert;
extern const DOSConvertElement BaseStruct_Convert[];
extern const Uns16 NumBaseStruct_Convert;
extern const DOSConvertElement MineStruct_Convert[];
extern const Uns16 NumMineStruct_Convert;
extern const DOSConvertElement TurntimeStruct_Convert[];
extern const Uns16 NumTurntimeStruct_Convert;
extern const DOSConvertElement BaseOrderStruct_Convert[];
extern const Uns16 NumBaseOrderStruct_Convert;
extern const DOSConvertElement HostGenStruct_Convert[];
extern const Uns16 NumHostGenStruct_Convert;
extern const DOSConvertElement XyplanStruct_Convert[];
extern const Uns16 NumXyplanStruct_Convert;
extern const DOSConvertElement ShipxyStruct_Convert[];
extern const Uns16 NumShipxyStruct_Convert;
extern const DOSConvertElement VcrStruct_Convert[];
extern const Uns16 NumVcrStruct_Convert;
extern const DOSConvertElement TargetShipStruct_Convert[];
extern const Uns16 NumTargetShipStruct_Convert;
extern const DOSConvertElement TurnHeaderStruct_Convert[];
extern const Uns16 NumTurnHeaderStruct_Convert;
extern const DOSConvertElement ExtMessageStruct_Convert[];
extern const Uns16 NumExtMessageStruct_Convert;

/* MESS.TMP is same format as MESS.EXT so we don't really need a separate
   conversion structure */
#define MessageStruct_Convert ExtMessageStruct_Convert
#define NumMessageStruct_Convert NumExtMessageStruct_Convert

extern const DOSConvertElement HullspecStruct_Convert[];
extern const Uns16 NumHullspecStruct_Convert;
extern const DOSConvertElement EngspecStruct_Convert[];
extern const Uns16 NumEngspecStruct_Convert;
extern const DOSConvertElement BeamspecStruct_Convert[];
extern const Uns16 NumBeamspecStruct_Convert;
extern const DOSConvertElement TorpspecStruct_Convert[];
extern const Uns16 NumTorpspecStruct_Convert;

extern const DOSConvertElement RSTMineStruct_Convert[];
extern const Uns16 NumRSTMineStruct_Convert;
extern const DOSConvertElement UFOStruct_Convert[];
extern const Uns16 NumUFOStruct_Convert;
extern const DOSConvertElement WinPlanRegStruct_Convert[];
extern const Uns16 NumWinPlanRegStruct_Convert;
extern const DOSConvertElement TonsStruct_Convert[];
extern const Uns16 NumTonsStruct_Convert;

/* For API backwards compatibility */
#define EndianSwap16 WordSwapShort
#define EndianSwap32 WordSwapLong

#endif /* _DOSCVT_H_ */

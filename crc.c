
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

static CRCType CRCTable[256];

#define CCITT 67601UL           /* == 0x10811 which is the 16-12-5-1
                                   polynomial */

static int initialized = 0;

void
InitCRC(void)
{
  unsigned int i;

  if (initialized)
    return;

  for (i = 0; i < 256; i++) {
    int j;
    unsigned long k = i;

    for (j = 0; j < 8; j++) {
      if (k & 1)
        k ^= CCITT;
      k >>= 1;
    }
    CRCTable[i] = (CRCType) k;
  }

  initialized = 1;
}

CRCType GenerateCRC(void *base, Uns32 size)
{
  CRCType crc = 0;
  unsigned char HUGE *ptr = (unsigned char HUGE *) base;

  InitCRC();

  while (size--) {
    crc = CRCTable[(crc & 0xFFU) ^ *ptr++] ^ (crc >> 8);
  }

  return crc;
}

#if 0
CRCType IncrementalCRC(unsigned char byte, CRCType crc)
{
  return CRCTable[(crc & 0xFFU) ^ byte] ^ (crc >> 8);
}

CRCType GenerateCRCContinue(void *pData, Uns32 pSize, CRCType pCrc)
{
  unsigned char *pPtr = (unsigned char *) pData;

  while (pSize--) {
    pCrc = IncrementalCRC(*pPtr, pCrc);
    pPtr++;
  }
  return pCrc;
}
#endif

#ifdef BUILD_MAIN
void
main(int argc, char *argv[])
{
  CRCType crc = 0;
  int i;

  InitCRC();
  for (i = 1; i < argc; i++) {
    unsigned long val;

    val = strtoul(argv[i], 0, 0);
    crc = IncrementalCRC((unsigned char) (val & 0xFFU), crc);
  }
  printf("%04X\n", crc);
}

void *
MemAlloc(size_t bytes)
{
  return malloc(bytes);
}
#endif

/*************************************************************
  $HISTORY:$
**************************************************************/

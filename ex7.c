/*
 *  EXAMPLE PROGRAM: `util.dat' access
 *
 *  Invoked as `ex7 player filename', will send the specified file
 *  as util.dat file transfer to the specified player.
 *
 *  This example is extraordinarily trivial, but with a bit shell
 *  tailored around it can be pretty useful. It should be run in
 *  auxhost2 or earlier (if you run it after the host, add a
 *  `SetUtilMode(UTIL_Dat)').
 */
#include <stdio.h>
#include "phostpdk.h"

#define BUFFER 1024

int main(int argc, char** argv)
{
  int lPlayer;
  FILE* fp;
  char* lBuffer;
  char* lFileName;
  size_t lSize;

  /* check parameters */
  if(argc != 3) {
    fprintf(stderr, "usage: %s player filename\n", argv[0]);
    return 1;
  }
  
  lPlayer = atoi(argv[1]);
  if(lPlayer <= 0 || lPlayer > RACE_NR) {
    fprintf(stderr, "bad player number %s\n", argv[1]);
    return 1;
  }
  lFileName = argv[2];
  
  fp = fopen(lFileName, "rb");
  if(!fp) {
    perror(lFileName);
    return 1;
  }

  /* read file into buffer */
  lSize = 0;
  lBuffer = 0;
  while(1) {
    size_t lNow;
    lBuffer = MemRealloc(lBuffer, lSize + BUFFER);
    lNow = fread(&lBuffer[lSize], 1, BUFFER, fp);
    lSize += lNow;
    if(!lNow)
      break;
  }
  fclose(fp);
  
  if(lSize != (Uns16) lSize) {
    fprintf(stderr, "file too large for file transfer\n");
    return 1;
  }

  if(!PutUtilFileTransfer(lPlayer, lFileName, False, lSize, lBuffer)) {
    fprintf(stderr, "error sending file %s\n", lFileName);
    return 1;
  }
  return 0;
}

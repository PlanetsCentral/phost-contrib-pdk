/**
  *  \file null.c
  *  \brief Null Addon
  *
  *  This is a simple template for quick add-ons, tests etc.
  */

#include "phostpdk.h"

int main(int argc, char** argv)
{
    if (argc > 1)
        gGameDirectory = argv[1];
    if (argc > 2)
        gRootDirectory = argv[2];
    if (argc > 3)
        ErrorExit("usage: %s [gamedir [rootdir]]", argv[0]);

    InitPHOSTLib();
    if (!ReadGlobalData())
        ErrorExit("ReadGlobalData failed");
    if (!ReadHostData())
        ErrorExit("ReadHostData failed");

    /* Your code here. */

    if (!WriteHostData())
        ErrorExit("WriteHostData failed");
    FreePHOSTLib();
    return 0;
}

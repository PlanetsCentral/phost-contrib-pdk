/**
  *  \file ex11.c
  *  \brief PDK Example Program -- Tons Report
  *
  *  This program prints a brief report of tons lost / tons destroyed.
  *
  *  This is an example program for instructional purposes only. It is
  *  not guaranteed to work correctly, or even to necessarily compile
  *  on all systems. You use this program at your own risk.
  */

#include <phostpdk.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    int lPlayer;
    Uns32 lTotal[4] = { 0, 0, 0, 0 };
    (void) argc;

    if (*++argv)
        gGameDirectory = *argv;
    if (*++argv)
        gRootDirectory = *argv;

    InitPHOSTLib();
    if (!ReadGlobalData())
        ErrorExit("Unable to read global data");
    if (!ReadHostData())
        ErrorExit("Unable to read host data");
    printf("Tons Report\n"
           "============\n"
           "\n"
           "           This Turn      Whole Game\n"
           "Player    Sunk   Lost     Sunk   Lost\n"
           "------  ------- ------- ------- -------\n");
    for (lPlayer = 1; lPlayer <= RACE_NR; ++lPlayer) {
        if (PlayerIsActive(lPlayer)) {
            printf("%4d    %7ld %7ld %7ld %7ld\n",
                   lPlayer,
                   (long)GetTons(Tons_DestroyedThisTurn, lPlayer),
                   (long)GetTons(Tons_LostThisTurn, lPlayer),
                   (long)GetTons(Tons_Destroyed, lPlayer),
                   (long)GetTons(Tons_Lost, lPlayer));
            lTotal[0] += GetTons(Tons_DestroyedThisTurn, lPlayer);
            lTotal[1] += GetTons(Tons_LostThisTurn, lPlayer);
            lTotal[2] += GetTons(Tons_Destroyed, lPlayer);
            lTotal[3] += GetTons(Tons_Lost, lPlayer);
        }
    }
    printf("------  ------- ------- ------- -------\n"
           "TOTAL:  %7ld %7ld %7ld %7ld\n",
           (long)lTotal[0], (long)lTotal[1], (long)lTotal[2], (long)lTotal[3]);
    FreePHOSTLib();
    return 0;
}

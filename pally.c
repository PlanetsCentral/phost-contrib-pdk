#include "phostpdk.h"
#include <memory.h>
#include <string.h>
#include <ctype.h>

#define PALLY_VERSION_MAJOR 1
#define PALLY_VERSION_MINOR 0

#define  AND   &&
#define  OR    ||
#define  EQ    ==
#define  NEQ   !=
#define  GT    >
#define  GE    >=
#define  LT 	<
#define  LE    =<

static const char *gTeam = 0;
static const char *gClear = 0;

static void
usage(char *argv[])
{
    fprintf(stderr,
"Usage: PALLY [Options] [GameDir [RootDir]]\n"
"\n"
"Options:\n"
"\t(none) -- Display alliance status for all players\n"
"\t-t NNN -- Create a full team alliance for player numbers in 'NNN'.\n"
"\t          'NNN' is a list of player numbers (use 'A' for 10,\n"
"\t          'B' for 11). For example: 'pally -t 1357A'.\n"
"\t-c NNN -- Remove all alliances for player numbers in 'NNN'.\n"
"\t          'NNN' is specified as for the -t option.\n"
"\t-v     -- Print current version information and exit\n"
"\t-h     -- Print this help screen\n"
            );
    exit(1);
}

static void
processOptions(int argc, char *argv[])
{
    int c;

    while ((c = pgetopt(argc, argv, "hHvt:c:")) NEQ EOF) {
        switch (c) {
         case 'h': case 'H': default:
            usage(argv);

         case 'c':
            gClear = poptarg;
            break;

         case 't':
            gTeam = poptarg;
            break;

         case 'v':
            printf("PALLY Portable Alliance Manager\nVersion %u.%u\n\nType 'pally -h' for a help summary.\n",
                        PALLY_VERSION_MAJOR, PALLY_VERSION_MINOR);
            exit(0);
        }
    }

    if (poptind < argc) {
        gGameDirectory = argv[poptind++];
    }
    if (poptind < argc) {
        gRootDirectory = argv[poptind++];
    }
    if (poptind < argc) {
        fprintf(stderr,"Too many arguments.\n\nType 'pally -h' for a help summary.\n");
        exit(1);
    }
}

const char * constructAllyString(Uns16 pPlayer, Uns16 pAlly)
{
    static char lStr[128];
    char lStr2[64];
    char lOffers[5];
    int lLevel;

    memset(lOffers, '-', sizeof(lOffers));

    if (PlayerOffersAlliance(pPlayer, pAlly)) {
        for (lLevel = ALLY_SHIPS; lLevel <= ALLY_VISION; lLevel++) {
            if (IsAllyOfferAny(pPlayer, pAlly, lLevel)) {
                lOffers[lLevel] = IsAllyOfferConditional(pPlayer, pAlly, lLevel)
                                ? '~'
                                : '+';
            }
        }
        sprintf(lStr, "%cs %cp %cm %cc %cv  ",
                    lOffers[ALLY_SHIPS], lOffers[ALLY_PLANETS],
                    lOffers[ALLY_MINES], lOffers[ALLY_COMBAT],
                    lOffers[ALLY_VISION]);
    } else {
        sprintf(lStr, "%16s", "");
    }

    memset(lOffers, '-', sizeof(lOffers));

    if (PlayerOffersAlliance(pAlly, pPlayer)) {
        for (lLevel = ALLY_SHIPS; lLevel <= ALLY_VISION; lLevel++) {
            if (IsAllyOfferAny(pAlly, pPlayer, lLevel)) {
                lOffers[lLevel] = IsAllyOfferConditional(pAlly, pPlayer, lLevel)
                                ? '~'
                                : '+';
            }
        }
        sprintf(lStr2, "%cs %cp %cm %cc %cv  ",
                    lOffers[ALLY_SHIPS], lOffers[ALLY_PLANETS],
                    lOffers[ALLY_MINES], lOffers[ALLY_COMBAT],
                    lOffers[ALLY_VISION]);
    } else {
        sprintf(lStr2, "%16s", "");
    }
    strcat(lStr, lStr2);
    if (PlayersAreAllies(pPlayer, pAlly)) {
        strcat(lStr, "<ALLIES>");
    }

    return (PlayerOffersAlliance(pAlly, pPlayer) || PlayerOffersAlliance(pPlayer, pAlly))
                ? lStr
                : 0;
}

static void displayAlliances(void)
{
    Uns16 lPlayer, lAlly;
    const char *lAllyStr;

    for (lPlayer = 1; lPlayer <= RACE_NR; lPlayer++) {
        if (! PlayerIsActive(lPlayer)) continue;

        printf("Player %2u:\n----------\n", lPlayer);

        for (lAlly = 1; lAlly <= RACE_NR; lAlly++) {
            if (! PlayerIsActive(lAlly)) continue;

            lAllyStr = constructAllyString(lPlayer, lAlly);
            if (lAllyStr) {
                printf("        [%2u] %s\n", lAlly, lAllyStr);
            }
        }
        printf("\n");
    }
}

static void parsePlayerSpec(const char *pTeamStr, Boolean pAlly[])
{
    const char *lTeamStr = pTeamStr;
    static const char lXlat[] = "0123456789AB";
    Uns16 lPlayer;

    memset(pAlly, 0, (RACE_NR+1)*sizeof(pAlly[0]));

    while (*lTeamStr) {
        const char *p = strchr(lXlat, toupper(*lTeamStr));

        if ( (! p) OR (p EQ lXlat) ) {
            ErrorExit("Team specification string '%s' is invalid.", pTeamStr);
        }

        lPlayer = (Uns16) (p - lXlat);

        if (! PlayerIsActive(lPlayer)) {
            ErrorExit("Player number %u is not active in this game.", lPlayer);
        }

        pAlly[lPlayer] = True;

        lTeamStr++;
    }
}

static void constructTeam(const char *pTeamStr)
{
    Uns16 lPlayer, lPlayer2;
    Boolean lAlly[RACE_NR+1];
    Uns16 lLevel;

    parsePlayerSpec(pTeamStr, lAlly);

    for (lPlayer = 1; lPlayer <= RACE_NR; lPlayer++) {
        if (! PlayerIsActive(lPlayer)) continue;
        if (! lAlly[lPlayer]) continue;

        for (lPlayer2 = 1; lPlayer2 <= RACE_NR; lPlayer2++) {
            if (lPlayer2 EQ lPlayer) continue;
            if (! lAlly[lPlayer2]) continue;

            AllyAddRequest(lPlayer, lPlayer2);
            for (lLevel = ALLY_SHIPS; lLevel <= ALLY_VISION; lLevel++) {
                PutPlayerAllowsAlly(lPlayer, lPlayer2, lLevel, ALLY_STATE_ON);
            }
        }
    }
}

static void clearTeam(const char *pTeamStr)
{
    Uns16 lPlayer, lPlayer2;
    Boolean lAlly[RACE_NR+1];

    parsePlayerSpec(pTeamStr, lAlly);

    for (lPlayer = 1; lPlayer <= RACE_NR; lPlayer++) {
        if (! PlayerIsActive(lPlayer)) continue;
        if (! lAlly[lPlayer]) continue;

        for (lPlayer2 = 1; lPlayer2 <= RACE_NR; lPlayer2++) {
            if (lPlayer2 EQ lPlayer) continue;
            if (! lAlly[lPlayer2]) continue;

            AllyDropRequest(lPlayer, lPlayer2);
        }
    }
}

int
main(int argc, char *argv[])
{
    Boolean lWriteback = False;

    processOptions(argc, argv);

    InitPHOSTLib();

    if (    Read_HostGen_File()
        AND Read_HConfig_File()
        AND Read_Turntime_File()
        AND Read_AuxData_File()
       ) ; /* accept */ else exit(1);

    if (gTeam) {
        constructTeam(gTeam);
        lWriteback = True;
    }
    if (gClear) {
        clearTeam(gClear);
        lWriteback = True;
    }

    if (lWriteback) {
        if (! Write_AuxData_File()) {
            ErrorExit("Unable to write alliance changes");
        } else {
            Info("Alliance changes saved.");
        }
    } else {
        displayAlliances();
    }

    FreePHOSTLib();

    return 0;
}

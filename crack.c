#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "includes.h"
#include "pgetopt.h"
#include "phostpdk.h"
#include "version.h"

#define VERSION_MAJOR PHOST_VERSION_MAJOR
#define VERSION_MINOR PHOST_VERSION_MINOR

#define  AND   &&
#define  OR    ||
#define  EQ    ==
#define  NEQ   !=
#define  GT    >
#define  GE    >=
#define  LT 	<
#define  LE    =<

static void
version(void)
{
	printf("Portable CRACK v%u.%u\n",VERSION_MAJOR,VERSION_MINOR);
}

static void
usage(char *argv[])
{
	fprintf(stderr,
"Usage: %s [option] [GameDirectory]\n"
"\n"
"Prints the passwords of all the players in a game.\n"
"\n"
"options:\n"
"        -h      -- Print help and exit\n"
"        -v      -- Print version and exit\n"
"\n"
					,argv[0]);
	exit(1);
}

static void processOptions(int argc, char *argv[])
{
	int c;

    while ((c=pgetopt(argc, argv, "hHv")) NEQ EOF) {
		switch (c) {
		 case 'h': case 'H': default:
			usage(argv);

		 case 'v':
			version();
			exit(1);
		}
	}

    if (poptind < argc) {
        gGameDirectory = argv[poptind++];
	}

    if (poptind < argc) usage(argv);
}

int
main(int argc, char *argv[])
{
	Uns16 lRace;

	processOptions(argc, argv);

	gLogFile = OpenOutputFile("crack.log", GAME_DIR_ONLY | TEXT_MODE);

	version();

	InitPHOSTLib();

/*    if (! ReadGlobalData()) exit(1); */
    if (! ReadHostData()) exit(1);

	for (lRace = 1; lRace <= RACE_NR; lRace++) {
		if (PlayerIsActive(lRace)) {
			Info("Player %u: '%s'",lRace,PlayerPassword(lRace));
		}
	}

	FreePHOSTLib();

	return 0;
}



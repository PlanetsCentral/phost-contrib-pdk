#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "includes.h"
#include "pgetopt.h"
#include "phostpdk.h"
#include "version.h"

#define VERSION_MAJOR PHOST_VERSION_MAJOR
#define VERSION_MINOR PHOST_VERSION_MINOR

/*
  history

  1.1  Added "abort on message length error" and
	   "E" player number == Everyone.
	   By Mike Campbell

  1.0  Initial release.

*/



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
	printf("Portable SENDMESS v%u.%u\n",VERSION_MAJOR,VERSION_MINOR);
}

static void
usage(char *argv[])
{
	fprintf(stderr,
"Usage: %s [option] {123456789AB | E} [GameDirectory]\n"
"\n"
"Sends a message to one or more players. The message is assumed to come\n"
"from stdin. Messages must be limited to 15 lines of no more than 40\n"
"characters per line.\n"
"\n"
"A player number of \"E\" (Everyone) will send the message to all players.\n"
"\n"
"options:\n"
"        -a      -- Abort message send if message too long or wide.\n"
"        -f str  -- Set the originator's name as 'str' (default \"Your Host\")\n"
"        -h      -- Print help and exit\n"
"        -v      -- Print version and exit\n"
"\n"
"Example:    sendmess 135B gamedir\n"
"            cat data | sendmess E gamedir\n"
					,argv[0]);
	exit(1);
}

static char *gFrom = "Your Host";
static Boolean gAbortOnError = False;
static Boolean gReceiver[RACE_NR+1];
static const char gEveryone = 'E';

static void processOptions(int argc, char *argv[])
{
	int c;
	char *p;
	static const char lRaceXlat[] = "123456789AB";

    while ((c=pgetopt(argc, argv, "ahHvf:")) NEQ EOF) {
		switch (c) {
		 case 'h': case 'H': default:
			usage(argv);

		 case 'f':
            gFrom = poptarg;
			break;

		 case 'v':
			version();
			exit(1);

		  case 'a':
			  gAbortOnError = True;
			  break;
		}
	}

    if (poptind < argc) {
        if (strlen(argv[poptind]) EQ 0) usage(argv);

		memset(gReceiver, 0, sizeof(gReceiver));
        for (p = argv[poptind++]; *p; p++) {
			const char *lFound;

			if (*p EQ gEveryone) { /* send message to everyone. Just populate array. */
				int lIndex;
				for (lIndex = 1; lIndex <= RACE_NR; lIndex++) {
					gReceiver[lIndex] = True;
				}
                break;  /* for (p = argv[poptind++]; ...).  Everyone being sent to; may as well get out. */
			}

			lFound = strchr(lRaceXlat, *p);
			if (lFound EQ 0) {
				fprintf(stderr,"Illegal race number: '%c'\n",*p);
				exit(1);
			}
			gReceiver[(size_t)(lFound - lRaceXlat + 1)] = True;
		}
	} else usage(argv);

    if (poptind < argc) {
        gGameDirectory = argv[poptind++];
	}

    if (poptind < argc) usage(argv);
}

int
main(int argc, char *argv[])
{
	Uns16 lRace;
	char *lBuffer;
	char lTempBuf[48];
	Uns16 lLine;

	processOptions(argc, argv);

	version();

	InitPHOSTLib();

	lBuffer = (char *)MemCalloc(41*20,1);

    sprintf(lBuffer,"(-h000)<<< Sub Space Message >>>\x0D" "FROM : %s\x0D" "TO : ",gFrom);

	for (lRace=1; lRace <= RACE_NR; lRace++) {
		if (! gReceiver[lRace]) continue;

		sprintf(lTempBuf,"%u ",lRace);
		strcat(lBuffer,lTempBuf);
	}
	strcat(lBuffer,"\x0D");

	fprintf(stdout,"+---------+---------+---------+--------|\n");

	lLine = 0;
	while (fgets(lTempBuf, 45, stdin) NEQ 0) {
		size_t len;

		lLine++;

		if (lLine > 15) {
			fprintf(stderr,"Warning: All lines past line 15 were discarded.\n");
			if (gAbortOnError EQ True) {
				fprintf(stderr,"Exiting.\n");
				exit(1);
			}
			break;
		}

		len = strcspn(lTempBuf, "\n\r");
		lTempBuf[len++] = 0x0D;
		lTempBuf[len++] = 0;

		if (strlen(lTempBuf) > 41) {
			lTempBuf[len-2] = 0;
			fprintf(stderr,"Warning: line %u was truncated to '%s'\n",
								lLine, lTempBuf);
			lTempBuf[len-2] = 0x0D;
			if (gAbortOnError EQ True) {
				fprintf(stderr,"Exiting.\n");
				exit(1);
			}
		}
		strcat(lBuffer,lTempBuf);
	}

	for (lRace = 1; lRace <= RACE_NR; lRace++) {
		if (! gReceiver[lRace]) continue;

		if (! WriteExternalMessage(lRace, lBuffer)) {
			fprintf(stderr,"Error in writing message to race %u!\n",lRace);
			exit(1);
		}
	}

	FreePHOSTLib();

	return 0;
}


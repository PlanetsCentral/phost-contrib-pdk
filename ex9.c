/**
  *  \file ex9.c
  *  \brief PDK Example Program -- Galactic Bank Wire Transfer
  *
  *  This program demonstrates the use of the command message parser,
  *  also for a pretty complicated command syntax.
  *
  *  This implements a very simple wire transfer between planets. I
  *  mean, we have had cashless money transfers in the ending 20th
  *  century, I'd be rather surprised if a "space age" civilisation
  *  does not have such a thing :-) I guess it is best run in
  *  auxhost1.
  *
  *  Players send a command `bank: transfer 100 mc from 123 to 456'
  *  ("x bank: ...." in PHost 3.4a and below), and this add-on will
  *  move the cash instantly. You can also write the `transfer'
  *  commands into `bankN.txt' (where N is your player number). Thanks
  *  PDK it also supports the "old-style" command format ("x
  *  transfer...", "x bank transfer..."), with all its disadvantages.
  *
  *  The player who gives the command must own the sending planet;
  *  ownership of the receiving planet doesn't matter (i.e. you can
  *  move cash to unowned planets, or send a gift to your allies).
  *
  *  In addition, this understands the "help" command.
  *
  *  Hacked together in about an hour by Stefan Reuther.
  *
  *  Extensions that come to mind:
  *  - transfer limits (amount, distance)
  *  - better logging
  *  - send the receiver of the cash a message
  *
  *  This file was written by Stefan Reuther in 2002/2003. You may
  *  copy and distribute this file freely as long as you retain this
  *  notice and explicitly document any changes you make to this file
  *  (along with your name and date) before distributing modified
  *  versions.
  *
  *  This is an example program for instructional purposes only. It is
  *  not guaranteed to work correctly, or even to necessarily compile
  *  on all systems. You use this program at your own risk.
  */

#include <phostpdk.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static char* sProgName;         /**< Name of the program, for help screen.  */
static char sWarnings = 0;      /**< Nonzero to send warnings to screen, not subspace.  */
static char sVerbose = 0;       /**< Nonzero to be more verbose. */
static char sReadonly = 0;      /**< Nonzero for "dry-run" (read-only). */

/** Help screen. */
void
Help(void)
{
    fprintf(stderr,
            "PDK Example Program 9 -- Wire Transfer\n\n"
            "usage: %s [-hwvr] gamedir [rootdir]\n\n"
            " -h   this help screen\n"
            " -w   show complaints as warnings (default: subspace messages)\n"
            " -v   verbose\n"
            " -r   read-only\n",
            sProgName);
    exit(1);
}

/** Process a complaint. This is the CommandComplain_Func. */
void
ProcessComplaint(Uns16 pRace, const char* pLine, const char* pReason, void* pData)
{
    if (sWarnings)
        ComplainWithWarningMessage(pRace, pLine, pReason, pData);
    else
        ComplainWithSubspaceMessage(pRace, pLine, pReason, pData);
}

/** True iff strings are equal, ignoring case. */
Boolean
StringEq(const char* a, const char* b)
{
    while (*a && toupper((unsigned char)*a) == toupper((unsigned char)*b))
        ++a, ++b;
    if (*a || *b)
        return False;
    else
        return True;
}

/** Convert string to integer and MemFree it.
    \param pRace   [in] race, for error messages
    \param pLine   [in] whole line, for error messages
    \param pString [in] the string
    \param pValue  [out] value
    \param pMin,pMax [in] allowed range */
Boolean
StringToInt(Uns16 pRace, const char* pLine,
            char* pString, Uns32* pValue, Uns32 pMin, Uns32 pMax)
{
    char* p;
    unsigned long l = strtoul(pString, &p, 10);
    if (*pString && !*p && l >= pMin && l <= pMax) {
        MemFree(pString);
        *pValue = l;
        return True;
    } else {
        char buffer[50];
        /* normally we should fill in the correct value for pData
           here. This program doesn't use that parameter. */
        sprintf(buffer, "Invalid number: '%.20s'", pString);
        ProcessComplaint(pRace, pLine, buffer, 0);
        MemFree(pString);
        return False;
    }
}

/** Process a "transfer" command. This takes the string apart. This is
    just boring work, simplified somewhat by GetToken. */
void
ProcessTransferCommand(Uns16 pRace, const char* pArgs, const char* pWholeLine)
{
    static const char delim[] = " \t";
    Uns32 lAmount;
    Uns32 lSrc, lDest;
    char* p;

    /* syntax is: transfer 123 [mc] from 123 to 123 */
    p = GetToken(&pArgs, delim);
    if (!p) {
        ProcessComplaint(pRace, pWholeLine, "Expected amount to transfer", 0);
        return;
    }
    if (!StringToInt(pRace, pWholeLine, p, &lAmount, 0, 1000000000))
        return;

    p = GetToken(&pArgs, delim);
    if (p && StringEq(p, "mc")) {
        MemFree(p);
        p = GetToken(&pArgs, delim);
    }
    if (p && StringEq(p, "from")) {
        MemFree(p);
        p = GetToken(&pArgs, delim);
    }
    if (!p) {
        ProcessComplaint(pRace, pWholeLine, "Expected sender planet Id", 0);
        return;
    }
    if (!StringToInt(pRace, pWholeLine, p, &lSrc, 1, PLANET_NR))
        return;

    p = GetToken(&pArgs, delim);
    if (p && StringEq(p, "to")) {
        MemFree(p);
        p = GetToken(&pArgs, delim);
    }
    if (!p) {
        ProcessComplaint(pRace, pWholeLine, "Expected receiver planet Id", 0);
        return;
    }
    if (!StringToInt(pRace, pWholeLine, p, &lDest, 1, PLANET_NR))
        return;
    if ((p = GetToken(&pArgs, delim)) != 0) {
        MemFree(p);
        ProcessComplaint(pRace, pWholeLine, "Trailing garbage", 0);
    }

    /* now we have all parameters. Validate them. */
    if (!IsPlanetExist(lSrc) || PlanetOwner(lSrc) != pRace) {
        ProcessComplaint(pRace, pWholeLine, "You don't own the sending planet", 0);
        return;
    }
    if (!IsPlanetExist(lDest)) {
        /* allow transfers to unowned planets. Otherwise, this would give
           a pretty neat radar. */
        ProcessComplaint(pRace, pWholeLine, "Receiver planet does not exist", 0);
        return;
    }
    if (PlanetCargo(lSrc, CREDITS) < lAmount) {
        ProcessComplaint(pRace, pWholeLine, "Sending planet has too little cash", 0);
        return;
    }
    PutPlanetCargo(lSrc, CREDITS, PlanetCargo(lSrc, CREDITS) - lAmount);
    PutPlanetCargo(lDest, CREDITS, PlanetCargo(lDest, CREDITS) + lAmount);
    if (sVerbose)
        Info("Player %d transferred %ld mc from %ld to %ld.", (int)pRace, (long)lAmount, (long)lSrc, (long)lDest);
}

/** Command handler. This is the CommandReader_Func. It handles
    two commands, "help" and "transfer". */
Boolean
ProcessCommand(Uns16 pRace, const char* pCommand, const char* pArgs,
               const char* pWholeLine, void* pData)
{
    (void) pData;               /* we don't use it */
    
    /* help command */
    if (StringEq(pCommand, "help")) {
        WriteAUXHOSTMessage(pRace,
                            "(-h000)<<< Help >>>\015\015"
                            "The Bank understands these commands:\015"
                            "- 'help'\015"
                            "  this help text\015"
                            "- 'transfer NN mc from AA to BB'\015"
                            "  wire-transfer NN megacredits from\015"
                            "  planet AA (must be yours) to planet\015"
                            "  BB (any owner does)\015"
                            "  The words 'mc' 'from' 'to' are\015"
                            "  optional; 'transfer' is mandatory.\015"
                            "Prefix commands with 'bank:'. Example:\015"
                            "  bank: transfer 100 from 132 to 367\015");
        if (sVerbose)
            Info("Player %d asked for help", (int) pRace);
        return True;
    }

    /* transfer command */
    if (StringEq(pCommand, "transfer")) {
        ProcessTransferCommand(pRace, pArgs, pWholeLine);
        return True;
    }

    return False;
}

/** Main. */
int
main(int argc, char** argv)
{
    int c;
    sProgName = argv[0];

    while ((c = pgetopt(argc, argv, "hvwr")) != EOF) {
        if (c == 'h')
            Help();
        else if (c == 'v')
            sVerbose = 1;
        else if (c == 'w')
            sWarnings = 1;
        else if (c == 'r')
            sReadonly = 1;
        else
            ErrorExit("Invalid option '%c'", c);
    }

    if (poptind < argc)
        gGameDirectory = argv[poptind++];
    if (poptind < argc)
        gRootDirectory = argv[poptind++];
    if (poptind < argc)
        ErrorExit("Too many parameters");

    /* Here comes da fun */
    InitPHOSTLib();
    if (!ReadGlobalData())
        ErrorExit("Unable to read global data");
    if (!ReadHostData())
        ErrorExit("Unable to read host data");

    CommandFileReader(0 /* any race does */,
                      ProcessCommand,
                      ProcessComplaint,
                      "bank",
                      "bank%d.txt",
                      0 /* private data */);

    /* write out everything */
    if (!sReadonly) {
        if (!WriteHostData())
            ErrorExit("Unable to write back host data");
    }
    FreePHOSTLib();
    return 0;
}

/** Today's sublimal thought is: */

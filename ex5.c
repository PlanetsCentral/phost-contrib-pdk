/*
 *   Portable Host Development Kit
 *   EXAMPLE PROGRAM #5
 *
 * This program is meant to demonstrate what an AUXHOST add-in program
 * might look like. It is really no different from an external program
 * except that it calls WriteAUXHOSTMessage() instead of WriteMessageToRST()
 * or WriteExternalMessage() to write player messages.
 *
 * This example program implements rudimentary transporter technology
 * for starbases. Starbases may send one type of mineral (or colonists,
 * supplies, and money) to another starbase on each turn. The amount of
 * that substance or item to transport is fixed at 20% of the current
 * stock but this can be made configurable or intelligent by some enterprising
 * young programmer.
 *
 * The planetary friendly code is used to effect transfer. If a planet's
 * friendly code is 'TX_' (where _ is N, T, D, M, C, S, or $) then the
 * starbase in orbit of that planet is a transmitter. The special transmit
 * friendly codes, then, are:
 *          TXN     -- Transmit Neutronium
 *          TXT     -- Transmit Tritanium
 *          TXD     -- Transmit Duranium
 *          TXM     -- Transmit Molybdenum
 *          TXC     -- Transmit Colonists
 *          TXS     -- Transmit Supplies
 *          TX$     -- Transmit Megacredits
 *
 * The starbase will transmit 20% of the current stock of the item to
 * the receiver starbase. The receiver starbase must have its friendly
 * code set to 'RX_' where '_' is once again the letter code for the item
 * to receive (as above). Multiple transports may be occuring at once on
 * the same turn but they are all done in order of base ID numbers. That
 * is, the lowest numbered base with a transmit friendly code is found, and
 * then the lowest numbered base with a matching receive code is used as
 * the receiver. The next-lowest numbered base is found with a transmit
 * code, etc.
 *
 * This utility is meant to be run in the AUXHOST1 phase. For example,
 * set up an AUXHOST1.INI file in the game directory that contains the
 * following line:
 *
 *              EX5 gamedir
 *
 * This file is copyrighted by the PDK authors Andrew Sterian, Thomas Voigt
 * and Steffen Pietsch in 1995. You may copy and distribute this file
 * freely as long as you retain this notice and explicitly document any
 * changes you make to this file (along with your name and date) before
 * distributing modified versions.
 *
 * This is an example program for instructional purposes only. It is not
 * guaranteed to work correctly, or even to necessarily compile on all
 * systems. You use this program at your own risk.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <phostpdk.h>

static const char *transferString[NumCargoTypes] = {
    "KT of neutronium",
    "KT of tritanium",
    "KT of duranium",
    "KT of molybdenum",
    "colonists",
    "supplies",
    "megacredits"
};

int
main(int argc, char *argv[])
{
    Int16 dummy1, dummy2;
    Uns16 race, base, count;
    Uns16 *baseList = 0, *basePtr;
    char playerMessage[500];

    /* Parse our command-line parameters. First parameter, if any, is the
    game directory. Second parameter, if present, is the root directory. */

    switch (argc) {
     case 3:
        gRootDirectory = argv[2];
        /* FALL THROUGH */

     case 2:
        gGameDirectory = argv[1];
        /* FALL THROUGH */

     case 1:
        break;

     default:
        fprintf(stderr,"Usage: %s  [GameDirectory [RootDirectory]]\n", argv[0]);
        exit(1);
    }

    /* Initialize the PDK */
    InitPHOSTLib();

    if (! ReadGlobalData()) exit(1);
    if (! ReadHostData()) exit(1);

    /* Make a list of starbases for each race. Saves time over having to
       go through the entire list of bases. The 'baseList' array will
       hold the list of bases for a single player at a time, null-terminated.
       This could be optimized further by building 11 arrays, one for each
       race, etc. but it ain't worth it. */
    baseList = (Uns16 *) MemCalloc(sizeof(Uns16), PLANET_NR+1);

    for (race = 1; race <= RACE_NR; race++) {

        /* Build up the list of bases for this race. Might as well check
           first two letters of friendly code here. If they're not TX or
           RX, then this base is not participating in any transfer. */
        for (count = 1, basePtr = baseList; count <= PLANET_NR; count++) {
            if (   IsBaseExist(count)
                && (BaseOwner(count) == race)
                && (   (strncmp(PlanetFCode(count, 0), "TX", 2) == 0)
                    || (strncmp(PlanetFCode(count, 0), "RX", 2) == 0)
                   )
               ) {
                *basePtr++ = count;
            }
        }
        *basePtr++ = 0;

        /* Now go through each starbase and find a transmitter. */
        for (basePtr = baseList; *basePtr != 0; basePtr++) {
            Uns16 *receiver = 0;
            CargoType_Def cargoType;
            char pFCode[4];

            if (strncmp(PlanetFCode(*basePtr, pFCode), "TX", 2) == 0) {
                /* This is a transmitter. Determine cargo type. */
                const char *found;
                const char *specialChars = "NTDMCS$";
                found = strchr(specialChars, pFCode[2]);

                /* If cargo type is unknown, ignore this transmitter. */
                if (found == 0) continue;

                cargoType = (CargoType_Def) (found - specialChars);

                /* Now find a receiver. We can search for the exact FCode
                   since we know what we're looking for. */
                pFCode[0] = 'R';   /* Replace TX_ with RX_ */
                for (receiver = baseList; *receiver != 0; receiver++) {
                    if (strncmp(PlanetFCode(*receiver, 0), pFCode, 3) == 0) {
                        /* We have both a transmitter and receiver. Do the
                           transfer. */
                        Uns32 amount, transfer;
                        char planName1[PLANETNAME_SIZE+1];
                        char planName2[PLANETNAME_SIZE+1];

                        amount = PlanetCargo(*basePtr, cargoType);
                        transfer = amount/5;    /* Transmit 20% */
                        PutPlanetCargo(*basePtr, cargoType, amount - transfer);
                        PutPlanetCargo(*receiver, cargoType,
                                PlanetCargo(*receiver, cargoType) + transfer);

                        /* Send a message */
                        sprintf(playerMessage,
                          "<<< Base-To-Base Transport Log >>>\x0D"
                          "\x0D"
                          "%lu %s were transported\x0D"
                          "from %s #%u\x0D"
                          "to %s #%u\x0D",
                                transfer, transferString[cargoType],
                                PlanetName(*basePtr, planName1), *basePtr,
                                PlanetName(*receiver, planName2), *receiver);
                        WriteAUXHOSTMessage(race, playerMessage);

                        /* Skip the check for other receivers and just
                           go to the next transmitter */
                        goto next_transmitter;

                    } /* end of test for receiver */
                } /* end of receiver search */
            } /* end of test for transmitter */
next_transmitter: ;
        } /* end of transmitter search */
    } /* end of transfers for a single race */

    MemFree(baseList);

    /* Close down. We need to write the planets file back to disk. The
       base info hasn't changed. */
    if (! Write_Planets_File(dummy1)) exit(1);

    FreePHOSTLib();

    return 0;
}


/*-------------------------------------------------------------------------
 * This source code is confidential proprietary information which is
 * Copyright (c) 2017 by Great Hill Corporation.
 * All Rights Reserved
 *
 * The LICENSE at the root of this repo details your rights (if any)
 *------------------------------------------------------------------------*/
#include "etherlib.h"
#include "options.h"

//-----------------------------------------------------------------------------------
int main(int argc, const char* argv[]) {

    // Tell the system where the blocks are and which version to use
    etherlib_init("fastest");

    // Parse command line, allowing for command files
    COptions options;
    if (!options.prepareArguments(argc, argv))
        return 0;

    while (!options.commandList.empty()) {
        SFString command = nextTokenClear(options.commandList, '\n');
        if (!options.parseArguments(command))
            return 0;

        SFString message;

        CPriceQuoteArray quotes;
        if (loadPriceData(quotes, options.freshen, message) && quotes.getCount()) {

            SFString fmt = "";
            if (!verbose)
                fmt = "{ \"date\": \"[{DATE}]\", \"price\": \"[{CLOSE}]\" }";

            if (options.at) {
                cout << quotes[(uint32_t)indexFromTimeStamp(quotes, options.at)].Format(fmt);

            } else {
                if (verbose > 1)
                    UNHIDE_FIELD(CPriceQuote, "schema");
                cout << "[\n";
                uint32_t step = (uint32_t)options.freq / 5;
                bool done = false;
                for (uint32_t i = 0 ; i < quotes.getCount() && !done ; i = i + step) {

                    timestamp_t ts = toTimestamp(quotes[i].Format("[{TIMESTAMP}]"));
                    if (i > 0)
                        cout << ",\n";
                    if (i != indexFromTimeStamp(quotes, ts)) {
                        cerr << cRed << "mismatch between 'i' ("
                        << i << ") and 'index' ("
                        << indexFromTimeStamp(quotes, ts) << "). Quitting.\n" << cOff;
                        return 0;
                    }
                    cout << quotes[i].Format(fmt);

                    if (isTestMode() && dateFromTimeStamp(ts) >= SFTime(2017,8,15,0,0,0))
                        done = true;
                }
                cout << "\n]\n";
            }

        } else {
            return usage(message);

        }
    }
    return 0;
}

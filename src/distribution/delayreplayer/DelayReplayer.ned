// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.distribution.delayreplayer;

//
// This module is designed for replaying delay patterns (traces).
// It reads delay values from a specified file and replicates these delays within the simulation environment.
// It can be used to generate random delays following the distribution of this interface using the IRandomNumberProvider interface.
//
// There are two modes of operations.
//
// 1. Cycle through the delays in the file in a loop.
// For this operation mode a file with one delay value per line is required, e.g.
// <pre>
// 1ms
// 5ms
// 3ms
// ...
// <pre>
//
// 2. The delay is chosen based on the simulation time (this can be used to map delay values to real-world timestamps).
// For this operation mode a file with two comma-separated two columns is required (delay, timestamp), e.g.
// File format:
// <pre>
// delay time, start time
// 1ms, 0.1s
// 5us, 0.2s
// 3ms, 0.3s
// ...
// <pre>
simple DelayReplayer
{
    parameters:
        string csvFilename;
        int offset = default(0); // Starting position for CYCLE mode
        double timestampOffset @unit(s) = default(0s); // Time offset for TIME_BASED mode
        @class(DelayReplayer);
        @display("i=block/delay");
}


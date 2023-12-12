// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.distribution.delayreplayer;

//
// This module extends the DelayReplayer module to support multiple delayreplayer from various CSV files.
// It allows for the aggregation and management of multiple delayreplayer objects, each configured with its own CSV file.
//
// <pre>
// delayreplayers object:
// {"key for the first delayreplayer": "the csv of the delayreplayer",
//  "key for the second delayreplayer": "the csv of the delayreplayer",
// ...}
// </pre>
simple DelayReplayerContainer
{
    parameters:
        object delayreplayers = default({});
        @class(DelayReplayerContainer);
        @display("i=block/delay");
}
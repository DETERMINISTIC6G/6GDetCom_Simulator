// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.gptp;

import inet.linklayer.ieee8021as.Gptp;


// Based on INET's gPTP module.
// Contains additional functionality for DetCom, such as:
// - 5G clocks
// - Timestamping to calculate residence time inside the DetCom node
simple DetComGptp extends Gptp
{
    parameters:
        @class(DetComGptp);
        bool useC5Grr = default(false);
        string detComClockModule = default("^.detComClock");
        bool gmCapable @mutable() = default(true); // Defined whether this node is Grandmaster capable
        object detComInterfaceTypes = default(["tt"]); // Interfaces with 5G specific behavior
}

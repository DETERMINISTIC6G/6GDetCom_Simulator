// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.devices.tsntranslator;

import inet.node.ethernet.EthernetLink;

// A perfect channel used to connected the TTInterface of dstts and the nwtt TsnTranslator modules within a DetCom node.
channel TTChannel extends EthernetLink {
    length = default(0m); // length of channel
    datarate = 0bps; // data transmission rate
    @display("ls=,,da");
}
// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.devices.tsntranslator;

import inet.linklayer.ethernet.modular.LayeredEthernetInterface;
import inet.node.tsn.TsnSwitch;
import inet.linklayer.contract.IEthernetInterface;

// This module extends the LayeredEthernetInterface module.
// However, as its purpose is to connect multiple TsnTranslator modules within a DetCom node, no additional delay and
// no ifg should be used there. Thus, the interFrameGapInserter is set to 0s and the bitrate is set to inf Mbps.
// This module only works in combination when connecting it with another TTInterface using the TTChannel module.
//
// See the DetCom to see how this module is implemented.
module TTInterface extends LayeredEthernetInterface {
    parameters:
    	phyLayer.interFrameGapInserter.duration = default(0s); // duration of the inter frame gap inserter
    	bitrate = default(inf Mbps); // bitrate of UE communication link
}

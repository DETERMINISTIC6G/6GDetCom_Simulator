// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.simulations.delayreplayer;

import d6g.devices.DetCom;
import d6g.distribution.delayreplayer.DelayReplayerContainer;
import d6g.networks.DetComNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;


@license(LGPL);

//
// An extension of the TsnNetworkBase from the INET framework.
// The network consists of a client, a switch, and a server.
// A Histogram submodule is also included for statistical analysis, with data sourced from example-hist.xml.
//
network TsnTestNetwork extends DetComNetworkBase
{
    parameters:
        *.eth[*].bitrate = default(100Mbps); // Default bitrate for all Ethernet interfaces is set to 100Mbps.
    submodules:

        delayreplayerContainer: DelayReplayerContainer {
            @display("p=100,500;is=s");
        }

        device1: TsnDevice {
            @display("p=350,400");
        }
        detCom: DetCom { // DetCom module (deterministic communication).
            @display("p=450,400");
        }
        bridge1: TsnSwitch { // A DetCom Adapter.
            @display("p=550,400");
        }
        bridge2: TsnSwitch { // A TSN switch with a bridge function.
            @display("p=550,300");
        }
        device2: TsnDevice { // A TSN device acting as a talker (data sender).
            @display("p=550,200");
        }
    connections:
        device1.ethg++ <--> EthernetLink <--> detCom.dsttg++;
        detCom.nwttg <--> EthernetLink <--> bridge1.ethg++;
        bridge1.ethg++ <--> EthernetLink <--> bridge2.ethg++;
        bridge2.ethg++ <--> EthernetLink <--> device2.ethg++;
}

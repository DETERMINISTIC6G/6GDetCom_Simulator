// This file is part of Deliverable D4.4 [D44PLACEHOLDER]
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.simulations.packetdelaycorrection;

import d6g.devices.DetCom;
import d6g.distribution.histogram.HistogramContainer;
import d6g.networks.DetComNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;


//
// TODO Auto-generated network
//
network PDC extends DetComNetworkBase
{
    @display("bgb=844,923");
    submodules:
        histogramContainer: HistogramContainer {
            @display("p=100,800");
        }
        device1: TsnDevice {
            @display("p=407,671");
        }
        detCom: DetCom { // DetCom module (deterministic communication).
            @display("p=407,508");
        }
        bridge1: TsnSwitch { // A DetCom Adapter.
            @display("p=407,399");
        }
        bridge2: TsnSwitch { // A TSN switch with a bridge function.
            @display("p=407,275");
        }
        device2: TsnDevice { // A TSN device acting as a talker (data sender).
            @display("p=407,165");
        }
    connections:
        device1.ethg++ <--> EthernetLink <--> detCom.dsttg++;
        detCom.nwttg <--> EthernetLink <--> bridge1.ethg++;
        bridge1.ethg++ <--> EthernetLink <--> bridge2.ethg++;
        bridge2.ethg++ <--> EthernetLink <--> device2.ethg++;
}

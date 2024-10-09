// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.simulations.timesync;

import d6g.devices.DetCom;
import d6g.distribution.histogram.HistogramContainer;
import inet.networks.base.TsnNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnClock;
import inet.node.tsn.TsnDevice;


// 
// Base network derived from INETs gPTP example.
// We will use this network in the future to simulate time synchronization.
network TimeSyncNetwork extends TsnNetworkBase
{
    submodules:
        histogramContainer: HistogramContainer {
            @display("p=100,800");
        }
        timeTransmitter: TsnClock {
            @display("p=500,150");
        }
        detCom: DetCom {
            @display("p=500,300");
        }
        tsnDevice1: TsnDevice {
            @display("p=400,450");
        }
        tsnDevice2: TsnDevice {
            @display("p=600,450");
        }


    connections:
        timeTransmitter.ethg++ <--> EthernetLink <--> detCom.nwttg;
        tsnDevice1.ethg++ <--> EthernetLink <--> detCom.dsttg++;
        tsnDevice2.ethg++ <--> EthernetLink <--> detCom.dsttg++;

}

network TimeSyncNetworkUplink extends TsnNetworkBase
{
    submodules:
        histogramContainer: HistogramContainer {
            @display("p=100,800");
        }
        tsnDevice1: TsnDevice {
            @display("p=500,150");
        }
        detCom: DetCom {
            @display("p=500,300");
        }
        timeTransmitter: TsnClock {
            @display("p=400,450");
        }
        tsnDevice2: TsnDevice {
            @display("p=600,450");
        }


    connections:
        timeTransmitter.ethg++ <--> EthernetLink <--> detCom.dsttg++;
        tsnDevice1.ethg++ <--> EthernetLink <--> detCom.nwttg;
        tsnDevice2.ethg++ <--> EthernetLink <--> detCom.dsttg++;

}

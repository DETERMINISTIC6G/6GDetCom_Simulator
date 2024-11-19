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
import inet.node.tsn.TsnSwitch;

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

network MultiplehopDualShowcase extends TsnNetworkBase
{
    parameters:
        int numSwitches1;
        int numSwitches2;
        string pos1 = string(250 + (numSwitches1 * 100));
        string pos2 = string(int(pos1) + 100);
        string pos3 = string(int(pos2) + (numSwitches2 * 100));
    submodules:
        histogramContainer: HistogramContainer {
            @display("p=100,800");
        }
        timeTransmitter: TsnClock {
            @display("p=500,150");
        }
        tsnSwitchnwtt[numSwitches1]: TsnSwitch {
            @display("p=500,250,col,100");
        }
        detCom: DetCom {
            @display("p=500,${parent.pos1}");
        }
        tsnSwitch1[numSwitches2]: TsnSwitch {
            @display("p=400,${parent.pos2},col,100");
        }
        tsnSwitch2[numSwitches2]: TsnSwitch {
            @display("p=600,${parent.pos2},col,100");
        }
        tsnDevice1: TsnDevice {
            @display("p=400,${parent.pos3}");
        }
        tsnDevice2: TsnDevice {
            @display("p=600,${parent.pos3}");
        }

    connections:
        timeTransmitter.ethg++ <--> EthernetLink <--> tsnSwitchnwtt[0].ethg++;
        for i=0..sizeof(tsnSwitchnwtt)-2 {
            tsnSwitchnwtt[i].ethg++ <--> EthernetLink <--> tsnSwitchnwtt[i+1].ethg++;
        }
        tsnSwitchnwtt[numSwitches1-1].ethg++ <--> EthernetLink <--> detCom.nwttg;

        tsnSwitch1[0].ethg++ <--> EthernetLink <--> detCom.dsttg++;
        for i=0..sizeof(tsnSwitch1)-2 {
            tsnSwitch1[i].ethg++ <--> EthernetLink <--> tsnSwitch1[i+1].ethg++;
        }
        tsnSwitch1[numSwitches2-1].ethg++ <--> EthernetLink <--> tsnDevice1.ethg++;

        tsnSwitch2[0].ethg++ <--> EthernetLink <--> detCom.dsttg++;
        for i=0..sizeof(tsnSwitch2)-2 {
            tsnSwitch2[i].ethg++ <--> EthernetLink <--> tsnSwitch2[i+1].ethg++;
        }
        tsnSwitch2[numSwitches2-1].ethg++ <--> EthernetLink <--> tsnDevice2.ethg++;

}

network TwoMasterClocksMultiGptpShowcase extends TsnNetworkBase
{
    submodules:
        histogramContainer: HistogramContainer {
            @display("p=100,800");
        }
        tsnClock1: TsnClock {
            @display("p=500,150");
        }
        tsnClock2: TsnClock {
            @display("p=350,450");
        }
        detCom: DetCom {
            @display("p=500,300");
        }
        tsnDevice1: TsnDevice {
            @display("p=500,450");
        }
        tsnDevice2: TsnDevice {
            @display("p=650,450");
        }
	connections:
	    tsnClock1.ethg++ <--> EthernetLink <--> detCom.nwttg;
	    detCom.dsttg++ <--> EthernetLink <--> tsnClock2.ethg++;
	    detCom.dsttg++ <--> EthernetLink <--> tsnDevice1.ethg++;
	    detCom.dsttg++ <--> EthernetLink <--> tsnDevice2.ethg++;
}

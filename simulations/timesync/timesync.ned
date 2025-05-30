// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.simulations.timesync;

import d6g.devices.DetCom;
import d6g.distribution.histogram.HistogramContainer;
import inet.common.scenario.ScenarioManager;
import inet.networks.base.TsnNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;


// 
// Base network derived from INETs gPTP example.
// We will use this network in the future to simulate time synchronization.
network TimeSyncNetwork extends TsnNetworkBase
{
    submodules:
        scenarioManagerLink: ScenarioManager {
            @display("p=100,500;is=s");
        }
        scenarioManagerGm: ScenarioManager {
            @display("p=100,550;is=s");
        }
        histogramContainer: HistogramContainer {
            @display("p=100,800");
        }
        tsnDeviceA: TsnDevice {
            @display("p=800,300");
        }
        tsnSwitchA1: TsnSwitch {
            @display("p=700,300");
        }
        tsnSwitchA2: TsnSwitch {
            @display("p=600,300");
        }
        detCom: DetCom {
            @display("p=500,300");
        }
        tsnSwitchB1: TsnSwitch {
            @display("p=400,250");
        }
        tsnDeviceB: TsnDevice {
            @display("p=300,250");
        }
        tsnDeviceC: TsnDevice {
            @display("p=300,350");
        }


    connections:
        detCom.nwttg <--> EthernetLink <--> tsnSwitchA2.ethg++;
        detCom.dsttg++ <--> EthernetLink <--> tsnSwitchB1.ethg++;
        detCom.dsttg++ <--> EthernetLink <--> tsnDeviceC.ethg++;

        tsnSwitchA2.ethg++ <--> EthernetLink <--> tsnSwitchA1.ethg++;
        tsnSwitchA1.ethg++ <--> EthernetLink <--> tsnDeviceA.ethg++;

        tsnSwitchB1.ethg++ <--> EthernetLink <--> tsnDeviceB.ethg++;

}

network TimeSyncNetworkMultihop extends TsnNetworkBase
{
    parameters:
        int numSwitchesA;
    submodules:
        scenarioManagerLink: ScenarioManager {
            @display("p=100,500;is=s");
        }
        scenarioManagerGm: ScenarioManager {
            @display("p=100,550;is=s");
        }
        histogramContainer: HistogramContainer {
            @display("p=100,800");
        }
        tsnDeviceA: TsnDevice {
            @display("p=800,300");
        }
        tsnSwitchA[numSwitchesA]: TsnSwitch {
            @display("p=700,300,col,100");
            numEthInterfaces = 2;
        }
        detCom: DetCom {
            @display("p=500,300");
        }
        tsnSwitchB1: TsnSwitch {
            @display("p=400,250");
        }
        tsnDeviceB: TsnDevice {
            @display("p=300,250");
        }
        tsnDeviceC: TsnDevice {
            @display("p=300,350");
        }


    connections:
        detCom.nwttg <--> EthernetLink <--> tsnSwitchA[0].ethg[0];
        detCom.dsttg++ <--> EthernetLink <--> tsnSwitchB1.ethg++;
        detCom.dsttg++ <--> EthernetLink <--> tsnDeviceC.ethg++;

        for i=0..sizeof(tsnSwitchA)-2 {
            tsnSwitchA[i].ethg[1] <--> EthernetLink <--> tsnSwitchA[i+1].ethg[0];
        }
        tsnSwitchA[numSwitchesA-1].ethg[1] <--> EthernetLink <--> tsnDeviceA.ethg++;

        tsnSwitchB1.ethg++ <--> EthernetLink <--> tsnDeviceB.ethg++;

}

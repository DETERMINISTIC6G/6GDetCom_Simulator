//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package d6g.simulations.wfcs26_sol.wireless;

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
network Wireless extends TsnNetworkBase
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

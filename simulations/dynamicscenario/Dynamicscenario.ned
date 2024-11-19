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

package d6g.simulations.dynamicscenario;

//
// TODO Auto-generated network
//

import d6g.devices.DetCom;
import d6g.distribution.histogram.HistogramContainer;
import d6g.networks.DetComNetworkBase;
import inet.common.scenario.ScenarioManager;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;


network Dynamicscenario extends DetComNetworkBase
{
    submodules:
        histogramContainer: HistogramContainer {
            @display("p=100,800");
        }
        wirelessdevice1: TsnDevice {
            @display("p=350,400");
        }
        detCom: DetCom { // DetCom module (deterministic communication).
            @display("p=350,315");
        }
        bridge1: TsnSwitch { // A DetCom Adapter.
            @display("p=350,224");
        }
        bridge2: TsnSwitch { // A TSN switch with a bridge function.
            @display("p=350,147");
        }
        device1: TsnDevice { // A TSN device acting as a talker (data sender).
            @display("p=350,54");
        }
        device2: TsnDevice {
            @display("p=580,54");
        }
        wirelessdevice2: TsnDevice {
            @display("p=491,400");
        }

        distributionScenarioManager: ScenarioManager {
            @display("p=290,800");
        }
        streamScenarioManager: ScenarioManager {
            @display("p=429,800");
        }
    connections:
        wirelessdevice1.ethg++ <--> EthernetLink <--> detCom.dsttg++;
        wirelessdevice2.ethg++ <--> EthernetLink <--> detCom.dsttg++;
        detCom.nwttg <--> EthernetLink <--> bridge1.ethg++;
        bridge1.ethg++ <--> EthernetLink <--> bridge2.ethg++;
        bridge2.ethg++ <--> EthernetLink <--> device1.ethg++;
        bridge2.ethg++ <--> EthernetLink <--> device2.ethg++;

}

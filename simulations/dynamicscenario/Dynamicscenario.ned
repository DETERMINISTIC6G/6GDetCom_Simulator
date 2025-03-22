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
import d6g.scenariomanager.ChangeMonitor;
import d6g.scenariomanager.ObservedScenarioManager;
import inet.linklayer.configurator.gatescheduling.contract.IGateScheduleConfigurator;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;
import ned.IdealChannel;


network Dynamicscenario extends DetComNetworkBase
{
    parameters:
        *.gateScheduleConfigurator.typename = "ExternalGateScheduleConfigurator";//> like IGateScheduleConfigurator; 
		
        @display("bgb=2035,1324");
    submodules:
        histogramContainer: HistogramContainer {
            @display("p=100,795");
        }
        wirelessdevice1: TsnDevice {
            @display("p=406,571");
        }
        detComOne: DetCom { // DetCom module (deterministic communication).
            @display("p=350,315");
        }
        bridge1: TsnSwitch { // A DetCom Adapter.
            @display("p=674,321");
        }
        bridge2: TsnSwitch { // A TSN switch with a bridge function.
            @display("p=840,197");
        }
        device1: TsnDevice { // A TSN device acting as a talker (data sender).
            @display("p=350,54");
        }
        device2: TsnDevice {
            @display("p=580,54");
        }
        wirelessdevice2: TsnDevice {
            @display("p=618,566");
        }

        distributionScenarioManager: ObservedScenarioManager {
            @display("p=1665,413");
        }
        streamScenarioManager: ObservedScenarioManager {
            @display("p=1665,699");
        }

        monitor: ChangeMonitor {
            @display("p=1667,169");
        }

        detComTwo: DetCom { // DetCom module (deterministic communication).
            @display("p=445,169");
        }
        detComThree: DetCom { // DetCom module (deterministic communication).
            @display("p=1110,324");
        }
        wirelessdevice3: TsnDevice {
            @display("p=1110,570");
        }

        device3: TsnDevice {
            @display("p=1110,52");
        }
        device4: TsnDevice {
            @display("p=908,570");
        }
        device5: TsnDevice {
            @display("p=1421,199");
        }
        wirelessdevice5: TsnDevice {
            @display("p=1450,568");
        }

    connections:
        wirelessdevice1.ethg++ <--> EthernetLink { @icon("radio");@display("ls=red,,da"); } <--> detComOne.dsttg++;
        wirelessdevice2.ethg++ <--> EthernetLink { @icon("radio");@display("ls=red,,da"); } <--> detComOne.dsttg++;
        detComOne.nwttg <--> EthernetLink <--> bridge1.ethg++;
        bridge1.ethg++ <--> EthernetLink <--> bridge2.ethg++;
        detComTwo.dsttg++ <--> EthernetLink { @icon("radio");@display("ls=red,,da"); } <--> device1.ethg++;
        detComTwo.dsttg++ <--> EthernetLink { @icon("radio");@display("ls=red,,da"); } <--> device2.ethg++;

        bridge2.ethg++ <--> EthernetLink <--> detComTwo.nwttg;
        detComThree.nwttg <--> EthernetLink <--> bridge2.ethg++;
        detComThree.dsttg++ <--> EthernetLink { @icon("radio");@display("ls=red,,da"); } <--> wirelessdevice3.ethg++;
        detComThree.dsttg++ <--> EthernetLink { @icon("radio");@display("ls=red,,da"); } <--> wirelessdevice5.ethg++;

        device3.ethg++ <--> EthernetLink <--> bridge2.ethg++;
        bridge1.ethg++ <--> EthernetLink <--> device4.ethg++;
        device5.ethg++ <--> EthernetLink <--> bridge2.ethg++;
        
        
}

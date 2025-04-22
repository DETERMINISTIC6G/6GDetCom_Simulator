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


import d6g.devices.DetCom;
import d6g.distribution.histogram.HistogramContainer;
import d6g.networks.DetComNetworkBase;
import d6g.dynamicscenario.ChangeMonitor;
import d6g.dynamicscenario.ObservedScenarioManager;
import inet.linklayer.configurator.gatescheduling.contract.IGateScheduleConfigurator;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;


network BaselineNetwork extends DetComNetworkBase
{
    parameters:
        gateScheduleConfigurator.typename = default("ExternalGateScheduleConfigurator");
        bool enableMonitor = default(true); 

        @display("bgb=2529,1984");
    submodules:
        histogramContainer: HistogramContainer {
            @display("p=100,795");
        }
        
        detComOne: DetCom { // DetCom module (deterministic communication).
            @display("p=1246,1190");
        }
        bridge1: TsnSwitch { // A DetCom Adapter.
            @display("p=1246,863");
        }
        bridge2: TsnSwitch { // A TSN switch with a bridge function.
            @display("p=1246,563");
        }
        device1: TsnDevice { // A TSN device acting as a talker (data sender).
            @display("p=954,274");
        }
        device2: TsnDevice {
            @display("p=1511,276");
        }
        device3: TsnDevice {
            @display("p=717,863");
        }
        wirelessdevice1: TsnDevice {
            @display("p=954,1639");
        }
        wirelessdevice2: TsnDevice {
            @display("p=1509,1639");
        }
        
        scenarioManager: ObservedScenarioManager {
            @display("p=2242,1174");
        }

        monitor: ChangeMonitor if enableMonitor {
            @display("p=2242,1615");
        }
       
    connections:
        wirelessdevice1.ethg++ <--> EthernetLink { @icon("radio");@display("ls=red,,da"); } <--> detComOne.dsttg++;
        wirelessdevice2.ethg++ <--> EthernetLink { @icon("radio");@display("ls=red,,da"); } <--> detComOne.dsttg++;
        detComOne.nwttg <--> EthernetLink <--> bridge1.ethg++;
        bridge1.ethg++ <--> EthernetLink <--> bridge2.ethg++;
        bridge1.ethg++ <--> EthernetLink <--> device3.ethg++;
        bridge2.ethg++ <--> EthernetLink  <--> device1.ethg++;
        bridge2.ethg++ <--> EthernetLink <--> device2.ethg++;        
}

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

package d6g.dynamicscenario;

import inet.linklayer.configurator.gatescheduling.base.GateScheduleConfiguratorBase;
import inet.linklayer.configurator.gatescheduling.contract.IGateScheduleConfigurator;


//
// This module extends the GateScheduleConfiguratorBase module to dynamically configure the network at runtime
// after multiple changes by ScenarioManagers. It calls an external scheduler tool to calculate the GCLs,
// and configures all talkers/network elements interfaces, and the production offsets of the talkers.
//
// @see DynamicPacketSource
// @see ObservedScenarioManager
//
// It also prepares input files for the external scheduler, including network and stream configurations, 
// and wireless link distributions obtained from the Monitor module.
//
// @see ChangeMonitor
//
// <pre>
// Command usage:
// e.g.: "./path/scheduler -n %s -s %s -d %s -o %s"
// Please make sure to set the SCHEDULER_ROOT environment variable!
// All paths are relative to the SCHEDULER_ROOT variable and the folders must exist.
// Observe the correct order.
// Parameters:
// 1. -n %s: Specifies the network configuration (input-)file (networkFile). 
// 2. -s %s: Specifies the streams configuration (input-)file (streamsFile).
// 3. -d %s: Specifies changes of wireless link distributions (histogramsFile).
// 4. -o %s: Specifies the scheduler's output file (configurationFile).
// (%s will be replaced with the filename)
// </pre>
simple ExternalGateScheduleConfigurator extends GateScheduleConfiguratorBase like IGateScheduleConfigurator
{
    parameters:
    //the globally used gate scheduling period in each PeriodicGate modules
    //hyperperiod is automatically calculated
    gateCycleDuration @mutable = default(0s); 
            
    string command;
    
    volatile string networkFile;
    volatile string streamsFile;
    volatile string histogramsFile;
    volatile string configurationFile;
       
    @class(ExternalGateScheduleConfigurator);
}

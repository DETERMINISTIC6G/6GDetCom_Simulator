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

package d6g.scenariomanager;

import inet.linklayer.configurator.gatescheduling.base.GateScheduleConfiguratorBase;
import inet.linklayer.configurator.gatescheduling.contract.IGateScheduleConfigurator;


//
// This module extends the GateScheduleConfiguratorBase module to dynamically configure the network
// after multiple changes by ScenarioManagers. It calls an external scheduler tool for the calculation of the GCLs.
// It prepares input files for the scheduler, which include network configurations, stream configurations,
// and distributions over wireless links.
//
// <pre>
// Command usage:
// e.g.: "./path/scheduler -n %s -s %s -d %s -o %s"
// Parameters:
// -n %s: Specifies the network configuration (input-)file (networkFile). 
// -s %s: Specifies the scheduler's configuration (input-)file (streamsFile).
// -d %s: Specifies changes of wireless link distributions (histogramsFile).
// -o %s: Defines the output file (configurationFile).
// (%s will be replaced with the filename)
// </pre>
simple ExternalGateScheduleConfigurator extends GateScheduleConfiguratorBase like IGateScheduleConfigurator
{
    parameters:
    
    gateCycleDuration @mutable = default(0s); //hyperperiod is also automatically calculated in the monitor module
        
    string command;
    
    volatile string networkFile;
    volatile string streamsFile;
    volatile string histogramsFile;
    volatile string configurationFile;
       
    @class(ExternalGateScheduleConfigurator);
}

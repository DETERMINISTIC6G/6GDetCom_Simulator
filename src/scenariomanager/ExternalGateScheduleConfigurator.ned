// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.scenariomanager;

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

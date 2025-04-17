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


//
// This module is responsible of gathering all important information for the scheduler,
// such as histograms and stream requirements. It observes changes in DynamicPacketSource, TsnTranslator modules
// (and also in ObservedScenarioManager module) and forwards them to ExternalGateScheduleConfigurator module.
// In case the histogram is not provided using our Histogram module, it samples the defined delay distribution. 
// In case all TsnSwitch and TsnDevice modules in the network are configured with the same number of queues, 
// it maps the PCP value to the gateIndex globally.
simple ChangeMonitor
{
    parameters:
        string gateScheduleConfigurator = default("gateScheduleConfigurator");
        // wait for the specified time before performing a call to the scheduler
        volatile double schedulerCallDelay @unit(s) = default(0s);
               
        // If globallyNumTrafficClasses = 0 : mapping priority to traffic class is the responsibility of the scheduler. 
        // The number of configurable queues in the TsnSwitch and TsnDevice modules may vary accordingly.
        // Otherwise (globallyNumTrafficClasses > 0), assign the gateIndex to the pcp value.
      	int globallyNumTrafficClasses = default(8);
               
        // IEEE Standard for Local and Metropolitan Area Networks, Bridges and Bridged Networks
        // Table 8-5—Recommended priority to traffic class mappings
        object mapping = default([[0, 0, 0, 0, 0, 1, 1, 1],   // BE best effort
                                  [0, 0, 0, 0, 0, 0, 0, 0],   // BK background
                                  [0, 0, 0, 1, 1, 2, 2, 2],   // EE excellent effort
                                  [0, 0, 0, 1, 1, 2, 3, 3],   // CA critical applications
                                  [0, 1, 1, 2, 2, 3, 4, 4],   // VI video
                                  [0, 1, 1, 2, 2, 3, 4, 5],   // VO voice
                                  [0, 1, 2, 3, 3, 4, 5, 6],   // IC internetwork control
                                  [0, 1, 2, 3, 4, 5, 6, 7]]); // NC network control
        
        // The simulation will be stopped immediately if not all streams are scheduled. 
        // Otherwise, only the streams present in the scheduler's output will be allowed in the network.                          
        bool stopWhenNotSchedulable;
         //number of samples for the creation of a histogram, if uplink/downlink is an expression or a convolution
        int numberOfSamples = default(1000000);
        @class(ChangeMonitor);
}

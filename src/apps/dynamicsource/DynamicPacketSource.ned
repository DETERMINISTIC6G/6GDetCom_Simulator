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

package d6g.apps.dynamicsource;

import inet.queueing.source.ActivePacketSource;


// This module extends the ActivePacketSource module. It is to be used within the TsnDevice's (UdpSourceApp)applications.
// Various parameters can be changed dynamically in the (Observed)ScenarioManager (or automatically reconfigured 
// by ChangeMonitor/ExternalGateScheduleConfigurator modules). 
// It provides two types of changes: immediate effect and delayed effect ("pending").
//
// Additionally, it enables the individual specification of packet production offsets within a given cycle.
simple DynamicPacketSource extends ActivePacketSource
{
    parameters:
      // If you change these values, the effects are immediately visible in the network  
      productionInterval  @mutable;
      packetLength @mutable;
      volatile bool enabled @mutable = default(true); //start or stop the app dynamically
           
      // If you change these values, the effects in the network are only visible 
      // after the permission of the scheduler.
      // New 'Pending' parameters are only applied explicitly after receiving a new configuration
      volatile double pendingProductionInterval @unit(s) @mutable = default(productionInterval);
      volatile int pendingPacketLength @unit(b) @mutable = default(packetLength);              
      volatile bool pendingEnabled @mutable = default(enabled);
                
      // If the production interval is changed, adjust the deadline in ScenarioManager accordingly
      double maxJitter @mutable @unit(ms)  = default(productionInterval * 0.05);
      double maxLatency @mutable @unit(ms) = default(productionInterval);
             
      int pcp = default(0);           
      string objectiveType @enum("NO_OBJECTIVE",
      								"LATENESS",
      								"TARDINESS",
      								"JITTER",
      								"TARDINESS_AND_JITTER") = default("TARDINESS_AND_JITTER");     
      double reliability = default(0.95);
      //double weight = default(1.0); //
      //int packetLoss = default(0);
      //int policy = default(0); // packet delay budget
      
      //Offsets relative to the production interval, e.g., 
      //for period=20ms, the offset array could look like this: 
      //				[0ms, 1ms, 5ms, 10ms], 
      //meaning: Packet1 is sent at 0ms, 
      //		Packet2 is sent at 21ms, 
      //		Packet3 is sent at 45ms,
      //		Packet4 is sent at 70ms etc.
      //then repeated cyclically
      object productionOffsets @unit(s) = default([0ns]);
      
      @class(DynamicPacketSource);
      @signal[parameter-change-event](type=cMessage);
}

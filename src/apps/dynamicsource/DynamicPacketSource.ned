// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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
      // Unique stream identifier 
      string streamName = default("");
            
      // If you change these values, the effects are immediately visible in the network  
      productionInterval  @mutable;
      packetLength @mutable;
      //start or stop the app immediately
      volatile bool enabled @mutable = default(true); 
                 
      // If you change these values, the effects in the network are only visible 
      // after the permission of the scheduler.
      // New 'Pending' parameters are only applied explicitly after receiving a new configuration
      
      volatile double pendingProductionInterval @unit(s) @mutable = default(productionInterval);
      volatile int pendingPacketLength @unit(b) @mutable = default(packetLength);
      // Request to start or stop the app           
      volatile bool pendingEnabled @mutable = default(enabled);
        
                       
      // Defines the maximum allowed variation in packet delay
      double maxJitter @mutable @unit(ms)  = default(productionInterval * 0.05);
      // Defines the maximum acceptable end-to-end latency
      double maxLatency @mutable @unit(ms) = default(productionInterval);
      
      // Defines the priority level of the associated traffic class      
      int pcp = default(0);
      // Defines the reliability requirement of the app                								     
      double reliability = default(1.00);
      
      //Map for optional parameters that a specific scheduler may require,
      //e.g., {"policy": 0, "packetLoss": 0, "weight": 1.0, "objectiveType" : 4}
      object customParams = default({});
      
      //Offsets relative to the production interval, e.g., 
      //for period=20ms, the offset array could look like this: 
      //				[0ms, 1ms, 5ms, 10ms], 
      //meaning: Packet1 is sent at 0ms, 
      //		Packet2 is sent at 21ms, 
      //		Packet3 is sent at 45ms,
      //		Packet4 is sent at 70ms etc.
      //then repeated cyclically
      object productionOffsets @mutable @unit(s)  = default([0ns]);
      
      // Is not used, see 'productionOffsets' instead
      initialProductionOffset = 0ns; 
      
      @class(DynamicPacketSource);
      @signal[parameter-change-event](type=cMessage);
}

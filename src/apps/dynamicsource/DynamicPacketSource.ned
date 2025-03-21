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


//
// TODO auto-generated module
//
simple DynamicPacketSource extends ActivePacketSource
{
    parameters:
      // If you change the value, the effects are immediately visible in the network  
      productionInterval  @mutable;
      packetLength @mutable;
      
      // If you change the value, the effects in the network are only visible after the decision and permission of the scheduler
      volatile double dynamicProductionInterval @unit(s) @mutable = default(productionInterval);
      volatile int dynamicPacketLength @unit(b) @mutable = default(packetLength);
           
      // start and stop the app dynamically
      volatile bool enabled @mutable = default(true);
      
      // hard constraint time
      double jitter @unit(ms)  = default(1.0ms);
      double latency @unit(ms) = default(productionInterval);
            
      // additional  Traffic Class Requirements
      int pcp = default(0); //
      int gateIndex = default(0);
      int objectiveType = default(4);
      int packetLoss = default(0);
      int policy = default(0);
      double reliability = default(1.0);
      double weight = default(1.0); //
      
      
      @class(DynamicPacketSource);
      @signal[parameter-change-event](type=cMessage);
}

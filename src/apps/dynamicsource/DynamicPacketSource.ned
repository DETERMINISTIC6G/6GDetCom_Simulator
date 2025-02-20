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
      productionInterval  @mutable; // override
      packetLength @mutable;
      
      // start--stop app
      volatile bool enabled @mutable = default(true);
      
      // hard constraint time
      double jitter @unit(ms)  = default(0.05ms);
      double latency @unit(ms) = default(10ms);
      
      
      // additional  Traffic Class Requirements
      volatile int pcp @mutable = default(0);
      int objectiveType = default(3);
      int packetLoss = default(0);
      int policy = default(0);
      double reliability = default(0.9999);
      
      
      @class(DynamicPacketSource);
      @signal[parameter-change-event](type=cMessage);
}

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

package d6g.devices.pdc;

import inet.queueing.base.PacketDelayerBase;
import inet.queueing.contract.IPacketDelayer;


//
// TODO auto-generated type
//
simple PdcDelayer extends PacketDelayerBase like IPacketDelayer
{
    parameters:
        volatile double defaultPdc @unit(s) @mutable = default(0s); // specifies default pdc
        volatile double defaultJitter @unit(s) @mutable = default(0us); // specifies default jitter
        // array of objects, where each object maps
        // a PDC value, a jitter value to a stream name, 
        //for example [{stream: "stream1", pdc: "5ms", jitter: "100us"}, ...]
        volatile object mapping @mutable = default([]);
         
        object indInterfaceTypes = default([]); // Only apply delay if the ind interface is of specific type (e.g. ["tt"] or ["tt", "eth"], empty means any)
        object reqInterfaceTypes = default([]); // Only apply delay if the req interface is of specific type (e.g. ["tt"] or ["tt", "eth"], empty means any)
        @class(PdcDelayer);
}

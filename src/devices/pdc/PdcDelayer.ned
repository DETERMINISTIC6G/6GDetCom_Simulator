// This file is part of Deliverable D4.4 [D44PLACEHOLDER]
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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

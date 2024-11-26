//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package d6g.delayer;

import inet.queueing.common.BackPressureBarrier;
import inet.queueing.common.QueueingPacketDelayer;
import inet.queueing.contract.IPacketDelayer;

//This module uses a queue to ensure the sequence of packets.
//Also in the TTDelayerPacketServer, it calculates the delay which the packet stays in the TTDelayerQueue and compensates for it to meet the total delay distribution.
//The delay will shift to right.
module QueueingTTDelayer extends QueueingPacketDelayer like IPacketDelayer
{
    parameters:
        queue.typename = "TTDelayerQueue"; // Type of queue
        server.typename = "TTDelayerPacketServer"; // Type of server
        object indInterfaceTypes = default([]); // Only apply delay if the ind interface is of specific type (e.g. ["tt"] or ["tt", "eth"], empty means any)
        object reqInterfaceTypes = default([]); // Only apply delay if the req interface is of specific type (e.g. ["tt"] or ["tt", "eth"], empty means any)
        server.indInterfaceTypes = default(dup(this.indInterfaceTypes));
        server.reqInterfaceTypes = default(dup(this.reqInterfaceTypes));
    submodules:
        backPressureBarrier: BackPressureBarrier {
            @display("p=550,100");
        }
    connections:
        server.out --> {@reconnect;} --> backPressureBarrier.in;
        backPressureBarrier.out --> { @reonnect; @display("m=e");} --> out;
}

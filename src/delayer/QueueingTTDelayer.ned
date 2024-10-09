//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package d6g.delayer;

import inet.queueing.common.BackPressureBarrier;
import inet.queueing.common.QueueingPacketDelayer;
import inet.queueing.contract.IPacketDelayer;

// TODO: DESCRIPTION
module QueueingTTDelayer extends QueueingPacketDelayer like IPacketDelayer
{
    parameters:
        queue.typename = "TTDelayerQueue";
        server.typename = "TTDelayerPacketServer";
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

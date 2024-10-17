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
// This module is used as queueing TT Delayer in TsnTranslator.
// By configuring different parameters, users can assign which types should be delayed, and which queue or server should be used.
// BackPressureBarrier is used as buffer.

module QueueingTTDelayer extends QueueingPacketDelayer like IPacketDelayer
{
    parameters:
        queue.typename = "TTDelayerQueue"; // Type name of queue
        server.typename = "TTDelayerPacketServer"; // Type name of server
        object indInterfaceTypes = default([]); // Only apply delay if the ind interface is of specific type (e.g. ["tt"] or ["tt", "eth"], empty means any)
        object reqInterfaceTypes = default([]); // Only apply delay if the req interface is of specific type (e.g. ["tt"] or ["tt", "eth"], empty means any)
        server.indInterfaceTypes = default(dup(this.indInterfaceTypes));
        server.reqInterfaceTypes = default(dup(this.reqInterfaceTypes));
    submodules:
        backPressureBarrier: BackPressureBarrier { // Used as buffer to control the volume
            @display("p=550,100");
        }
    connections:
        server.out --> {@reconnect;} --> backPressureBarrier.in;
        backPressureBarrier.out --> { @reonnect; @display("m=e");} --> out;
}

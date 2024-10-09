//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package d6g.delayer;

import inet.queueing.contract.IPacketServer;
import inet.queueing.server.PacketServer;


// TODO: DESCRIPTION
simple TTDelayerPacketServer extends PacketServer like IPacketServer
{
    parameters:
        @class(TTDelayerPacketServer);
        object indInterfaceTypes = default([]); // Only apply delay if the ind interface is of specific type (e.g. ["tt"] or ["tt", "eth"], empty means any)
        object reqInterfaceTypes = default([]); // Only apply delay if the req interface is of specific type (e.g. ["tt"] or ["tt", "eth"], empty means any)
}

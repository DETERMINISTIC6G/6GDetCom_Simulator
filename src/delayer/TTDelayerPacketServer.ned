//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package d6g.delayer;

import inet.queueing.contract.IPacketServer;
import inet.queueing.server.PacketServer;


//
// This module repeatedly pulls packets from the connected packet provider and
// after a processing delay, it pushes the packet into the connected packet
// consumer. The processing delay is `processingTime` + `packetLength` / `processingBitrate`.
//
simple TTDelayerPacketServer extends PacketServer like IPacketServer
{
    parameters:
        @class(TTDelayerPacketServer);
}

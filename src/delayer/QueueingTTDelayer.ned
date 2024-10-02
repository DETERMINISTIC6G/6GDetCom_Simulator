//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package d6g.delayer;

import inet.queueing.common.QueueingPacketDelayer;
import inet.queueing.contract.IPacketDelayer;


module QueueingTTDelayer extends QueueingPacketDelayer like IPacketDelayer
{
    parameters:
        server.typename = "TTDelayerPacketServer";
}


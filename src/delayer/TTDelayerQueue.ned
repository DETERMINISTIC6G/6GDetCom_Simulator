//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package d6g.delayer;

import inet.queueing.queue.PacketQueue;

// TODO: DESCRIPTION
// This module is a type of packet queue and can synchronize its operation with external
// clock using clockModule.
simple TTDelayerQueue extends PacketQueue
{
    parameters:
        @class(TTDelayerQueue);
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
}

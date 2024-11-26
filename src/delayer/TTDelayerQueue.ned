//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package d6g.delayer;

import inet.queueing.queue.PacketQueue;

// This module adds the tag TTDelayerTag to the packet which records the enqueue time of the packet and uses it for compensation.
simple TTDelayerQueue extends PacketQueue
{
    parameters:
        @class(TTDelayerQueue);
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
}

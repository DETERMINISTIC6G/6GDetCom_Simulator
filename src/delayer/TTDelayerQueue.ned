// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


package d6g.delayer;

import inet.queueing.queue.PacketQueue;

// This module adds the tag TTDelayerTag to the packet which records the enqueue time of the packet and uses it for compensation.
simple TTDelayerQueue extends PacketQueue
{
    parameters:
        @class(TTDelayerQueue);
        string clockModule = default(""); // Relative path of a module that implements IClock; optional
}

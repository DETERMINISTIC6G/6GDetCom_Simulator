// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


package d6g.delayer;

import inet.queueing.contract.IPacketServer;
import inet.queueing.server.PacketServer;

//This module:
//Initializes the packet server. Calculates the delay for a packet based on whether it matchesInterfaceConfiguration: if it does, it uses delayParameter; otherwise, the delay is 0.
//Then sets the delay parameter. Compensates for the delay the packet has already spent in the queue by comparing the current clock time with the enqueue time obtained from the TTDelayerTag.
simple TTDelayerPacketServer extends PacketServer like IPacketServer
{
    parameters:
        @class(TTDelayerPacketServer);
        object indInterfaceTypes = default([]); // Only apply delay if the ind interface is of specific type (e.g. ["tt"] or ["tt", "eth"], empty means any)
        object reqInterfaceTypes = default([]); // Only apply delay if the req interface is of specific type (e.g. ["tt"] or ["tt", "eth"], empty means any)
}

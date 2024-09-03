// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.timestamping;

import inet.queueing.base.PacketFlowBase;


module TimeChunkInserter extends PacketFlowBase
{
    parameters:
        @class(TimeChunkInserter);
        @display("i=block/inserter");
        object reqInterfaceTypes = default(["tt"]); // Only apply delay if the req interface is of specific type (e.g. ["tt"] or ["tt", "eth"], empty means any)
        string nextProtocol = default("ethernetmac");
}

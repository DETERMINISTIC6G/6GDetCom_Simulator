// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
package d6g.delayer;


import inet.queueing.base.PacketDelayerBase;
import inet.queueing.contract.IPacketDelayer;

// The module is designed to delay packets based on the type of involved interfaces.
// It can be used in combination with the DetCom node to specify an uplink and downlink delay for a given device.
// See DetCom, TsnTranslator and TTDelayLayer module description for configuration details.
simple TTDelayer extends PacketDelayerBase like IPacketDelayer
{
    parameters:
        volatile double delay @unit(s) = default(0s); // specifies delay per packet
        object indInterfaceTypes = default([]); // Only apply delay if the ind interface is of specific type (e.g. ["tt"] or ["tt", "eth"], empty means any)
        object reqInterfaceTypes = default([]); // Only apply delay if the req interface is of specific type (e.g. ["tt"] or ["tt", "eth"], empty means any)
        @class(TTDelayer);
}

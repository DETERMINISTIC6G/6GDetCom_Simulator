// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.devices.tsntranslator;

import inet.linklayer.ethernet.common.PacketDirectionReverser;


// This module extends the PacketDirectionReverser of INET to keep internal DetCom time tags,
// such as the DetComIngressTimeTag.
module TTPacketDirectionReverser extends PacketDirectionReverser
{
    parameters:
        @class(TTPacketDirectionReverser);
}

// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "TTPacketDirectionReverser.h"

#include "../../timestamping/DetComTimeTag_m.h"

namespace d6g {
Define_Module(TTPacketDirectionReverser);

void TTPacketDirectionReverser::processPacket(inet::Packet *packet)
{
    auto detComIngressTimeTag = packet->findTag<DetComIngressTimeTag>();
    auto detComResidenceTimeTag = packet->findTag<DetComResidenceTimeTag>();
    PacketDirectionReverser::processPacket(packet);
    if (detComIngressTimeTag != nullptr) {
        auto newDetComIngressTimeTag = packet->addTag<DetComIngressTimeTag>();
        newDetComIngressTimeTag->setReceptionStarted(detComIngressTimeTag->getReceptionStarted());
        newDetComIngressTimeTag->setReceptionEnded(detComIngressTimeTag->getReceptionEnded());
    }
    if (detComResidenceTimeTag != nullptr) {
        auto newDetComResidenceTimeTag = packet->addTag<DetComResidenceTimeTag>();
        newDetComResidenceTimeTag->setResidenceTime(detComResidenceTimeTag->getResidenceTime());
    }
}
} // namespace d6g

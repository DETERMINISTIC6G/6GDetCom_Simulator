//
// Created by haugls on 9/3/24.
//

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
}

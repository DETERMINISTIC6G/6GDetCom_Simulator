//
// Created by haugls on 9/3/24.
//

#include "TTPacketDirectionReverser.h"

#include "../../timestamping/TimeTagDetCom_m.h"

namespace d6g {
Define_Module(TTPacketDirectionReverser);

void TTPacketDirectionReverser::processPacket(inet::Packet *packet)
{
    auto detComIngressTimeTag = packet->findTag<DetComIngressTimeTag>();
    PacketDirectionReverser::processPacket(packet);
    if (detComIngressTimeTag != nullptr) {
        auto newDetComIngressTimeTag = packet->addTag<DetComIngressTimeTag>();
        newDetComIngressTimeTag->setReceptionStarted(detComIngressTimeTag->getReceptionStarted());
        newDetComIngressTimeTag->setReceptionEnded(detComIngressTimeTag->getReceptionEnded());
    }
}
}

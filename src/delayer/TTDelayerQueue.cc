// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "TTDelayerQueue.h"

#include "TTDelayerTag_m.h"
#include "inet/common/clock/ClockUserModuleMixinImpl.h"

namespace inet {
template class ClockUserModuleMixin<inet::queueing::PacketQueue>;
}

namespace d6g {

Define_Module(TTDelayerQueue);

void TTDelayerQueue::pushPacket(Packet *packet, const cGate *gate) {
    auto enqueueTime = getClockTime();
    auto ttDelayerTag = packet->addTag<TTDelayerTag>();
    ttDelayerTag->setEnqueueTime(enqueueTime);
    PacketQueue::pushPacket(packet, gate);
}
} // namespace d6g

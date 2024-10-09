//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

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

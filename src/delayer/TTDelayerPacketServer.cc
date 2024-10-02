//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "TTDelayerPacketServer.h"

namespace d6g {

Define_Module(TTDelayerPacketServer);

void TTDelayerPacketServer::initialize(int stage)
{
    InterfaceFilterMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        setDelay(&par("processingTime"));
    }
}

clocktime_t TTDelayerPacketServer::computeDelay(Packet *packet) const
{
    clocktime_t delay;
    if (!matchesInterfaceConfiguration(packet)) {
        delay = 0;
    }
    else {
        delay = delayParameter->doubleValue();
    }

    return delay;
}

void TTDelayerPacketServer::setDelay(cPar *delay) { delayParameter = delay; }

void TTDelayerPacketServer::scheduleProcessingTimer()
{
    clocktime_t processingTime = computeDelay(packet);
    auto processingBitrate = bps(par("processingBitrate"));
    processingTime += s(packet->getDataLength() / processingBitrate).get();
    scheduleClockEventAfter(processingTime, processingTimer);
}
} // namespace d6g

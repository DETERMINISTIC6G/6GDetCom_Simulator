// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "TTDelayerPacketServer.h"

#include "TTDelayerTag_m.h"

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

    auto ttDelayerTag = packet->removeTag<TTDelayerTag>();
    auto enqueueTime = ttDelayerTag->getEnqueueTime();
    auto timeInQueue = getClockTime() - enqueueTime;
    auto remainingProcessingTime = processingTime - timeInQueue;
    if (remainingProcessingTime < 0)
        remainingProcessingTime = 0;

    scheduleClockEventAfter(remainingProcessingTime, processingTimer);
}
} // namespace d6g

//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "DynamicPacketSource.h"

namespace d6g {

Define_Module(DynamicPacketSource);

void DynamicPacketSource::initialize(int stage)
{
    ActivePacketSource::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        isStartedParameter = par("isStarted");
    }
}

void DynamicPacketSource::handleMessage(cMessage *msg)
{
    ActivePacketSource::handleMessage(msg);
}

void DynamicPacketSource::handleParameterChange(const char *name) {
    if (!strcmp(name, "isStarted")) {
        isStartedParameter = par("isStarted");
        if (!isStartedParameter) {
            if (productionTimer->isScheduled()) {
                cancelEvent(productionTimer);
            }
        } else {
            if (!productionTimer->isScheduled()) {
                scheduleProductionTimerAndProducePacket();
            }
        }
    }
    if (!strcmp(name, "initialProductionOffset")) {
        initialProductionOffset = par("initialProductionOffset");
        if (productionTimer->isScheduled()) {
            cancelEvent(productionTimer);
        }
        initialProductionOffsetScheduled = false;
        scheduleProductionTimerAndProducePacket();
    }
    if (!strcmp(name, "productionInterval")) {
        productionIntervalParameter = &par("productionInterval");
    }
    if (!strcmp(name, "packetLength")) {
        packetLengthParameter = &par("packetLength");
    }
}


void DynamicPacketSource::scheduleProductionTimer(clocktime_t delay){
    if (!isStartedParameter) return;
    if (scheduleForAbsoluteTime)
        scheduleClockEventAt(getClockTime() + delay, productionTimer);
    else
        scheduleClockEventAfter(delay, productionTimer);
}

void DynamicPacketSource::scheduleProductionTimerAndProducePacket() {
    if (!isStartedParameter) return;

    if (!initialProductionOffsetScheduled && initialProductionOffset >= CLOCKTIME_ZERO) {
        scheduleProductionTimer(initialProductionOffset);
        initialProductionOffsetScheduled = true;
    }else if (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate())) {
        scheduleProductionTimer(productionIntervalParameter->doubleValue());
        producePacket();
    }
}

} //namespace

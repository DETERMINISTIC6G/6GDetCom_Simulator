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
        runningState = &par("enabled");
        pendingEnabledState = par("pendingEnabled").boolValue();

        parameterChangeEvent = new ClockEvent("parameter-change");
        productionTimer->setSchedulingPriority(10);

        streamName = par("streamName").stdstringValue();

        cValueArray *productionOffsets = check_and_cast<cValueArray *>(par("productionOffsets").objectValue());

        if (productionOffsets->size() == 0) {
            throw cRuntimeError("The productionOffsets parameter must contain at least one value.");
        }
        computeProductionOffsets(productionOffsets);
    }
}

void DynamicPacketSource::incrementProductionOffset()
{
    nextProductionIndex = (nextProductionIndex + 1) % offsets.size();
}

void DynamicPacketSource::handleMessage(cMessage *msg)
{
    if (msg == productionTimer) {
        if (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate())) {
            if (offsets.size() > 1) {
                auto currentProductionOffset = offsets[nextProductionIndex];
                incrementProductionOffset();
                scheduleProductionTimer(productionIntervalParameter->doubleValue() + currentProductionOffset.dbl());
            }
            else {
                scheduleProductionTimer(productionIntervalParameter->doubleValue());
            }
            producePacket();
        }
    }
    else if (msg == parameterChangeEvent) {
        emit(DynamicScenarioObserver::parameterChangeSignal, msg);
    }
    else
        throw cRuntimeError("Unknown message type.");
}

void DynamicPacketSource::handleParameterChange(const char *name)
{
    if (!strcmp(name, "initialProductionOffset")) return;

    if (!strcmp(name, "pendingEnabled")) {
        pendingEnabledState = par("pendingEnabled").boolValue();
        hasSchedulerPermission = false;
    }
    if (!strcmp(name, "enabled")) {
        pendingEnabledState = runningState->boolValue();
        if (!runningState->boolValue() && productionTimer->isScheduled()) {
            hasSchedulerPermission = false;
            cancelEvent(productionTimer);
        }
        else if (runningState->boolValue() && !productionTimer->isScheduled()) {
            if (!ignoreChange) {
                hasSchedulerPermission = false;
                scheduleProductionTimer(0);
            }
        }
    }
    if (!strcmp(name, "productionOffsets")) {
        auto *productionOffsets = check_and_cast<cValueArray *>(par("productionOffsets").objectValue());
        if (productionOffsets->size() == 0) {
            throw cRuntimeError("The productionOffsets parameter must contain at least one value.");
        }
        if (ignoreChange) {
            computeProductionOffsets(productionOffsets);
            if (productionTimer->isScheduled()) { // Stop the production of packets from the old configuration
                cancelEvent(productionTimer);
            }
            if (hasSchedulerPermission) {
                scheduleProductionTimer(ClockTime(productionOffsets->get(0).doubleValueInUnit("ns"), SIMTIME_NS));
                incrementProductionOffset();
            }
        }else
            hasSchedulerPermission = false;
    } // endif

    if (!strcmp(name, "pendingProductionInterval") || !strcmp(name, "pendingPacketLength")) {
        hasSchedulerPermission = false;
    }
    if (!strcmp(name, "productionInterval") || !strcmp(name, "packetLength")) {
        if (!ignoreChange) {
            par("pendingProductionInterval").setValue(par("productionInterval").getValue());
            par("pendingPacketLength").setValue(par("packetLength").getValue());
            return;
        }
    }
    if (!strcmp(name, "maxLatency") || !strcmp(name, "maxJitter"))
        hasSchedulerPermission = false;

    /* Send the signal only once if multiple parameters change simultaneously.
     * Do not send a signal if the configurator module requests the app to stop/start
     * or it already configures the new production offsets. */
    if (!parameterChangeEvent->isScheduled() && !ignoreChange)
        scheduleClockEventAt(getClockTime(), parameterChangeEvent);
}

void DynamicPacketSource::scheduleProductionTimer(clocktime_t delay)
{
    if (!runningState->boolValue())
        return;
    if (scheduleForAbsoluteTime) {
        productionTimer->setSchedulingPriority(10);
        scheduleClockEventAt(getClockTime() + delay, productionTimer);
    }
    else {
        productionTimer->setSchedulingPriority(10);
        scheduleClockEventAfter(delay, productionTimer);
    }
}

void DynamicPacketSource::scheduleProductionTimerAndProducePacket()
{
    if (!runningState->boolValue())
        return;
    auto productionOffsets = check_and_cast<cValueArray *>(par("productionOffsets").objectValue());
    auto initialOffset = ClockTime(productionOffsets->get(0).doubleValueInUnit("ns"), SIMTIME_NS);
    nextProductionIndex = 0;
    if (!initialProductionOffsetScheduled && initialOffset >= CLOCKTIME_ZERO) {
        scheduleProductionTimer(initialOffset);
        initialProductionOffsetScheduled = true;
        incrementProductionOffset();
    }
    else if (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate())) {
        // TODO: Check
        auto nextSchedule = productionIntervalParameter->doubleValue();
        incrementProductionOffset();
        if (offsets.size() > 1) {
            nextSchedule += offsets[nextProductionIndex].dbl();
        }
        incrementProductionOffset();
        scheduleProductionTimer(nextSchedule);
        producePacket();
    }
}

cValueMap *DynamicPacketSource::getConfiguration() const
{
    cModule *appModule = this->getParentModule();
    cModule *deviceModule = appModule->getParentModule();

    cValueMap *map = new cValueMap();
    map->set("enabled", runningState->boolValue());
    map->set("stopReq", !pendingEnabledState);

    if (streamName != "") {
        map->set("name", par("streamName").stdstringValue());
    }
    map->set("pcp", par("pcp").intValue());
    map->set("application", cValue(appModule->getFullName()));
    map->set("source", cValue(deviceModule->getFullName()));
    map->set("destination", appModule->getSubmodule("io")->par("destAddress").stringValue());
    map->set("reliability", par("reliability").doubleValue());
    map->set("packetLength", par("pendingPacketLength").getValue());
    map->set("packetInterval", par("pendingProductionInterval").getValue());
    map->set("maxLatency", par("maxLatency").getValue());
    map->set("maxJitter", par("maxJitter").getValue());

    map->set("customParams", cValue(par("customParams").objectValue()));

    return map;
}

void DynamicPacketSource::setNewConfiguration(const std::vector<simtime_t> &productionTimesInHyperCycleVector)
{
    hasSchedulerPermission = true;

    ignoreChange = true;
    par("enabled").setBoolValue(true);
    par("productionInterval").setValue(par("pendingProductionInterval").getValue());
    par("packetLength").setValue(par("pendingPacketLength").getValue());
    ignoreChange = false;

    nextProductionIndex = 0;
    // length is always at least 1. otherwise, use the already existing TO_REMOVE.
    size_t vectorSize = productionTimesInHyperCycleVector.size();
    auto *tempArray = new cValueArray();
    // relative to the hyperperiod (= vec.size()*period)
    for (int i = 0; i < vectorSize; i++) {
        double offset_i = productionTimesInHyperCycleVector[i].inUnit(SIMTIME_NS) -
                          i * productionIntervalParameter->doubleValueInUnit("ns");
        tempArray->add(cValue(offset_i, "ns"));
    } // endfor
    ignoreChange = true;
    par("productionOffsets").setObjectValue(tempArray);
    ignoreChange = false;
}

void DynamicPacketSource::computeProductionOffsets(const cValueArray *values)
{
    offsets.clear();
    nextProductionIndex = 0;
    // Iterate over values array
    for (int i = 0; i < values->size(); i++) {
        const auto &currentValue = values->get(i);
        const auto &prevValue = values->get((i - 1 + values->size()) % values->size());
        double diff = currentValue.doubleValueInUnit("ns") - prevValue.doubleValueInUnit("ns");
        // Remove rounding errors of offsets
        offsets.emplace_back(diff, SIMTIME_NS, true);
    }
}

bool DynamicPacketSource::stopIfNotScheduled()
{
    if (!(hasSchedulerPermission && runningState->boolValue())) {
        ignoreChange = true;
        par("enabled").setBoolValue(false);
        par("pendingEnabled").setBoolValue(false);
        ignoreChange = false;
        return true;
    }
    return false;
}

DynamicPacketSource::~DynamicPacketSource() { cancelAndDeleteClockEvent(parameterChangeEvent); }
} // namespace d6g

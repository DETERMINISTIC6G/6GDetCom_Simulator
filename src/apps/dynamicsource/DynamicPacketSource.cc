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
#include "inet/applications/udpapp/UdpSocketIo.h"

namespace d6g {

Define_Module(DynamicPacketSource);

void DynamicPacketSource::initialize(int stage)
{
    ActivePacketSource::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {

        runningState = &par("enabled");
        wantToRun = par("pendingEnabled").boolValue();

        parameterChangeEvent = new ClockEvent("parameter-change");
        productionTimer->setSchedulingPriority(10);

        cValueArray *productionOffsets = check_and_cast<cValueArray *>(par("productionOffsets").objectValue());
        //std::vector<intval_t> test = productionOffsets->asIntVectorInUnit("ns");
        std::vector<simtime_t> tempVector(productionOffsets->size());
        for (int i = 0; i < productionOffsets->size(); i++) {
             const cValue& value = (*productionOffsets)[i];
             tempVector.push_back(SimTime(value.intValueInUnit("ns"), SIMTIME_NS));
        }
        computeProductionOffsets(tempVector);
    }else if (stage == INITSTAGE_LAST) {
        isFirstTimeRun = ! runningState->boolValue();
    }
}

void DynamicPacketSource::handleMessage(cMessage *msg) {
    if (msg == productionTimer) {
        if (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate())) {
            if (offsets.size() > 1) {
                auto currentProductionOffset = offsets[nextProductionIndex];
                nextProductionIndex = (nextProductionIndex + 1) % offsets.size();
                scheduleProductionTimer(productionIntervalParameter->doubleValue() + currentProductionOffset.dbl());
            } else {
                scheduleProductionTimer(productionIntervalParameter->doubleValue());
            }
            producePacket();
        }
    } else if (msg == parameterChangeEvent) {
        emit(DynamicScenarioObserver::parameterChangeSignal, msg);
    } else
        throw cRuntimeError("Unknown message type.");
}


void DynamicPacketSource::handleParameterChange(const char *name) {
    if (!strcmp(name, "pendingEnabled")) {
        wantToRun = par("pendingEnabled").boolValue();
        hasSchedulerPermission = false;
    }
    if (!strcmp(name, "enabled")) {
        wantToRun = runningState->boolValue();
        if (!runningState->boolValue() && productionTimer->isScheduled()) {
            hasSchedulerPermission = false;
            cancelEvent(productionTimer);
        } else if (runningState->boolValue()
                && !productionTimer->isScheduled()) {
            if (!ignoreChange) {
                hasSchedulerPermission = false;
                scheduleProductionTimer(0);
            }
            isFirstTimeRun = false;
        }
    }
    if (!strcmp(name, "initialProductionOffset")) {
        initialProductionOffset = par("initialProductionOffset");
        if (productionTimer->isScheduled()) { //Stop the production of packets from the old configuration
            cancelEvent(productionTimer);
        }
        if (hasSchedulerPermission) {
            scheduleProductionTimer(initialProductionOffset);
        }
    } //endif initialProductionOffset

    if (!strcmp(name, "pendingProductionInterval")
            || !strcmp(name, "pendingPacketLength")) {
        hasSchedulerPermission = false;
    }
    if (!strcmp(name, "productionInterval") || !strcmp(name, "packetLength")) {
        if (!ignoreChange) {
            par("pendingProductionInterval").setValue(par("productionInterval").getValue());
            par("pendingPacketLength").setValue(par("packetLength").getValue());
            return;
        }
    }
    if (!strcmp(name, "maxLatency"))
        return;

    /* Send the signal only once if multiple parameters change simultaneously.
     * Do not send a signal if the configurator requests the app to stop/start
     * or it already configures the new production offsets. */
    if (strcmp(name, "initialProductionOffset")
            && !parameterChangeEvent->isScheduled() && !ignoreChange)
        scheduleClockEventAt(getClockTime(), parameterChangeEvent);
}


void DynamicPacketSource::scheduleProductionTimer(clocktime_t delay){
    if (!runningState->boolValue()) return;
    if (scheduleForAbsoluteTime) {
        productionTimer->setSchedulingPriority(10);
        scheduleClockEventAt(getClockTime() + delay, productionTimer);
    }
    else{
        productionTimer->setSchedulingPriority(10);
        scheduleClockEventAfter(delay, productionTimer);
    }
}

void DynamicPacketSource::scheduleProductionTimerAndProducePacket() {
    if (!runningState->boolValue()) return;

    if (!initialProductionOffsetScheduled && initialProductionOffset >= CLOCKTIME_ZERO) {
        scheduleProductionTimer(initialProductionOffset);
        initialProductionOffsetScheduled = true;
    }else if (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate())) {
        scheduleProductionTimer(productionIntervalParameter->doubleValue());
        producePacket();
    }
}

cValueMap* DynamicPacketSource::getConfiguration() const{
    cModule *appModule = this->getParentModule();
    cModule *deviceModule = appModule->getParentModule();

    cValueMap *map = new cValueMap();
    map->set("enabled", runningState->boolValue());
    map->set("stopReq", !wantToRun);

    if (flowName != "") {
        map->set("name", cValue(flowName));
    }
    map->set("pcp", par("pcp").intValue());
    //map->set("gateIndex", par("gateIndex").intValue());
    map->set("application", cValue(appModule->getFullName()));
    map->set("source", cValue(deviceModule->getFullName()));
    map->set("destination", appModule->getSubmodule("io")->par("destAddress").stringValue());

    map->set("reliability", par("reliability").doubleValue());
    //map->set("policy", par("policy").intValue());
    map->set("packetLength", par("pendingPacketLength").getValue());
    map->set("packetInterval", par("pendingProductionInterval").getValue());

    map->set("phase", isFirstTimeRun ? par("initialProductionOffset").getValue() : cValue(0, "us"));
    map->set("maxLatency", par("maxLatency").getValue());
    map->set("maxJitter", par("maxJitter").getValue());
    //map->set("weight", par("weight").doubleValue());
    map->set("objectiveType", objective(par("objectiveType")));//par("objectiveType").intValue());
    //map->set("packetLoss", par("packetLoss").intValue());

    return map;
}

void DynamicPacketSource::setNewConfiguration(const std::vector<simtime_t>& productionTimesInHyperCycleVector) {
    hasSchedulerPermission = true;

    ignoreChange = true;
    par("enabled").setBoolValue(true);
    par("productionInterval").setValue(par("pendingProductionInterval").getValue());
    par("packetLength").setValue(par("pendingPacketLength").getValue());
    ignoreChange = false;

    offsets.clear();
    nextProductionIndex = 0;
    // length is always at least 1. otherwise, use the already existing InitialProductionOffset.
    size_t vectorSize = productionTimesInHyperCycleVector.size();
    if (vectorSize > 1) {
        std::vector<simtime_t> tempVector(vectorSize);
        // relative to the hyperperiod (= vec.size()*period)
        for (int i = 0; i < vectorSize; i++) {
            auto offset_i = productionTimesInHyperCycleVector[i] - i*productionIntervalParameter->doubleValue();
            tempVector[i] = offset_i;
        }//endfor
        computeProductionOffsets(tempVector);
    }//endif

    //start
    par("initialProductionOffset") = productionTimesInHyperCycleVector[0].dbl();
}

void DynamicPacketSource::computeProductionOffsets(const std::vector<simtime_t>& productionOffsets) {
    nextProductionIndex++;
    size_t vectorSize = productionOffsets.size();
    for (int i = 0; i < vectorSize; i++) {
         auto offset_prev = productionOffsets[(i-1 + vectorSize) % vectorSize].dbl();
         offsets.push_back(productionOffsets[i] - offset_prev);
    }
}


bool DynamicPacketSource::stopIfNotScheduled() {
    if (!(hasSchedulerPermission && runningState->boolValue())) {
        ignoreChange = true;
        par("enabled").setBoolValue(false);
        par("pendingEnabled").setBoolValue(false);
        ignoreChange = false;
        return true;
    }
    return false;
}

void DynamicPacketSource::cancelLastChanges() {
    if (!runningState->boolValue()) {
        ignoreChange = true;
        par("enabled").setBoolValue(false);
        par("pendingEnabled").setBoolValue(false);
        ignoreChange = false;
    }else {
        hasSchedulerPermission = true;
    }
}

int DynamicPacketSource::objective(const char* type) const {
        StreamObjectives value;
        if (!strcmp(type, "LATENESS"))
                value = StreamObjectives::LATENESS;
            else if (!strcmp(type, "TARDINESS"))
                value = StreamObjectives::TARDINESS;
            else if (!strcmp(type, "JITTER"))
                value = StreamObjectives::JITTER;
            else if (!strcmp(type, "TARDINESS_AND_JITTER"))
                value = StreamObjectives::TARDINESS_AND_JITTER;
            else
                value = StreamObjectives::NO_OBJECTIVE;
        return static_cast<int>(value);
    }

DynamicPacketSource::~DynamicPacketSource() {
        cancelAndDeleteClockEvent(parameterChangeEvent);
    }

/*
 * setSilent() {
ignoreChange = true;
par.setValue(xyz); -> ruft handleParameterChange();
}

handleParameterChange() {
if (ignoreChange){
ignoreChage=false
return
}
// normaler handleparameterchange code
}
 * */

} //namespace

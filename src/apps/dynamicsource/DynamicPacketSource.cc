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
        enabledParameter = par("enabled");
        parameterChangeEvent = new ClockEvent("parameter-change");

        pcp = par("pcp").intValue();
        latency = &par("latency");
        jitter = &par("jitter");
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
        EV << "Self-Message received immediately!\n";
        emit(DynamicScenarioObserver::parameterChangeSignal, msg);
    } else
        throw cRuntimeError("Unknown message");
}


void DynamicPacketSource::handleParameterChange(const char *name) {
    if (!strcmp(name, "enabled")) {
        enabledParameter = par("enabled");
        if (!enabledParameter) {
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

    }//endif initialProductionOffset
    if (!strcmp(name, "productionInterval")) {
        productionIntervalParameter = &par("productionInterval");
    }
    if (!strcmp(name, "packetLength")) {
        packetLengthParameter = &par("packetLength");
    }

    /*  */
    if (strcmp(name, "initialProductionOffset") && !parameterChangeEvent->isScheduled())
        //scheduleAt(simTime(), parameterChangeEvent);
        scheduleClockEventAt(getClockTime(), parameterChangeEvent);
}


void DynamicPacketSource::scheduleProductionTimer(clocktime_t delay){
    if (!enabledParameter) return;
    if (scheduleForAbsoluteTime)
        scheduleClockEventAt(getClockTime() + delay, productionTimer);
    else
        scheduleClockEventAfter(delay, productionTimer);
}

void DynamicPacketSource::scheduleProductionTimerAndProducePacket() {
    if (!enabledParameter) return;

    if (!initialProductionOffsetScheduled && initialProductionOffset >= CLOCKTIME_ZERO) {
        scheduleProductionTimer(initialProductionOffset);
        initialProductionOffsetScheduled = true;
    }else if (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate())) {
        scheduleProductionTimer(productionIntervalParameter->doubleValue());
        producePacket();
    }
}

cValueMap* DynamicPacketSource::getConfiguration() {
    cModule *appModule = this->getParentModule();
    cModule *deviceModule = appModule->getParentModule();
    //UdpSocketIo *socketModule = dynamic_cast<UdpSocketIo*>(appModule->getSubmodule("io"));

    cValueMap *map = new cValueMap();
    map->set("enabled", par("enabled").boolValue());
    if (flowName != "") {
        map->set("name", cValue(flowName));
    }
    map->set("pcp", par("pcp").intValue());
    map->set("gateIndex", par("gateIndex").intValue());
    map->set("application", cValue(appModule->getFullName()));
    map->set("source", cValue(deviceModule->getFullName()));
   // map->set("destination", socketModule->par("destAddress").stringValue());
    map->set("destination", appModule->getSubmodule("io")->par("destAddress").stringValue());

    map->set("reliability", par("reliability").doubleValue());
    map->set("policy", par("policy").intValue());
    map->set("packetLength", par("packetLength").getValue());
    map->set("packetInterval", par("productionInterval").getValue());

    map->set("phase", par("initialProductionOffset").getValue());
    map->set("maxLatency", par("latency").getValue());
    map->set("maxJitter", par("jitter").getValue());
    map->set("weight", par("weight").doubleValue());
    map->set("objectiveType", par("objectiveType").intValue());
    map->set("packetLoss", par("packetLoss").intValue());

    std::cout << "app pcp: " <<  pcp << endl;

    return map;
}

void DynamicPacketSource::setNewConfiguration(const std::vector<simtime_t>& simtimeVector) {
    offsets.clear();
    nextProductionIndex = 0;
    // length is always at least 1. otherwise, use the already existing InitialProductionOffset.
    if (simtimeVector.size() > 1) {
        nextProductionIndex++;
        // relative to the hyperperiod (= vec.size()*period)
        for (size_t i = 0; i < simtimeVector.size(); i++) {
            auto x = simtimeVector[i] - i*productionIntervalParameter->doubleValue();
            auto x_prev = simtimeVector[(i-1) % simtimeVector.size()].dbl();
            offsets.push_back(x - x_prev);
        }
    }
}

DynamicPacketSource::~DynamicPacketSource() {
        cancelAndDeleteClockEvent(parameterChangeEvent);

       // cancelAndDelete(parameterChangeEvent);
    }

} //namespace

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
        enabledParameter = par("enabled").boolValue();
        isFirstTimeRun = true;
        hasSchedulerPermission = false;

        parameterChangeEvent = new ClockEvent("parameter-change");
        productionTimer->setSchedulingPriority(10);

    }else if (stage == INITSTAGE_LAST) {
        isFirstTimeRun = ! static_cast<bool>(enabledParameter);
    }
}

void DynamicPacketSource::handleMessage(cMessage *msg) {
    if (msg == productionTimer) {
      //  std::cout << "Self-Message received App: " << simTime() << " flow " << flowName  << " prio: " << msg->getSchedulingPriority() << endl;
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
    bool configuratorChanges = false;
    if (!strcmp(name, "enabled")) {
        if (enabledParameter > 1) configuratorChanges = true;
        enabledParameter = par("enabled").boolValue();
        if (enabledParameter) {isFirstTimeRun = false;}
        else {
            if (productionTimer->isScheduled()) {
                cancelEvent(productionTimer);
            }
            hasSchedulerPermission = false;

        } /*else { //enabled
            if (hasSchedulerPermission && !productionTimer->isScheduled()) {
                scheduleProductionTimerAndProducePacket();
            }
        }*/
    }
    if (!strcmp(name, "initialProductionOffset")) {
        initialProductionOffset = par("initialProductionOffset");

        if (productionTimer->isScheduled()) { //Stop the production of packets from the old configuration
            cancelEvent(productionTimer);
        }

        //scheduleProductionTimerAndProducePacket();
        if (hasSchedulerPermission && enabledParameter)
            scheduleProductionTimer(initialProductionOffset);
        //initialProductionOffsetScheduled = false;

    }//endif initialProductionOffset

    if (!strcmp(name, "dynamicProductionInterval") || !strcmp(name, "dynamicPacketLength")) {
        hasSchedulerPermission = false;
    }

    if (!strcmp(name, "productionInterval") || !strcmp(name, "packetLength")) {
        //par("dynamicPacketLength").setValue(par("packetLength").getValue());
        //par("dynamicProductionInterval").setValue(par("productionInterval").getValue());
        return;
    }


    /*  */
    if (strcmp(name, "initialProductionOffset") && !parameterChangeEvent->isScheduled() && !configuratorChanges)
        //scheduleAt(simTime(), parameterChangeEvent);
        scheduleClockEventAt(getClockTime(), parameterChangeEvent);
}


void DynamicPacketSource::scheduleProductionTimer(clocktime_t delay){
    if (!enabledParameter) return;
    if (scheduleForAbsoluteTime)
        scheduleClockEventAt(getClockTime() + delay, productionTimer);
    else{
        productionTimer->setSchedulingPriority(10);
        scheduleClockEventAfter(delay, productionTimer);
    }
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
    map->set("enabled", static_cast<bool>(enabledParameter));//par("enabled").boolValue());
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
    map->set("packetLength", par("dynamicPacketLength").getValue());
    map->set("packetInterval", par("dynamicProductionInterval").getValue());

    map->set("phase", isFirstTimeRun ? par("initialProductionOffset").getValue() : cValue(0, "us"));
    map->set("maxLatency", par("latency").getValue());
    map->set("maxJitter", par("jitter").getValue());
    map->set("weight", par("weight").doubleValue());
    map->set("objectiveType", par("objectiveType").intValue());
    map->set("packetLoss", par("packetLoss").intValue());

    //std::cout << "app pcp: " <<  pcp << endl;

    return map;
}

void DynamicPacketSource::setNewConfiguration(const std::vector<simtime_t>& simtimeVector) {
    hasSchedulerPermission = true;
    offsets.clear();
    nextProductionIndex = 0;

    par("productionInterval").setValue(par("dynamicProductionInterval").getValue());
    par("packetLength").setValue(par("dynamicPacketLength").getValue());

    // length is always at least 1. otherwise, use the already existing InitialProductionOffset.
    if (simtimeVector.size() > 1) {
        nextProductionIndex++;
        // relative to the hyperperiod (= vec.size()*period)
        for (size_t i = 0; i < simtimeVector.size(); i++) {
            auto x = simtimeVector[i] - i*productionIntervalParameter->doubleValue();
          //  auto x_prev = simtimeVector[(i-1) % simtimeVector.size()].dbl();
            offsets.push_back(x);
          //  offsets.push_back(x - x_prev);

        }//endfor
        for (size_t i = 0; i < simtimeVector.size(); i++) {
            auto x_prev = offsets[(i-1) % simtimeVector.size()].dbl();
            offsets[i] = offsets[i] - x_prev;
            std::cout << flowName << " offset: " <<  offsets[i] << " simtimevector: " << simtimeVector[i] << endl;
        }
    }//endif
}


bool DynamicPacketSource::stopIfNotScheduled() {
    if (!(hasSchedulerPermission && enabledParameter)) {
        enabledParameter = 2;
        par("enabled").setBoolValue(false);
        return true;
    }
    return false;
}

DynamicPacketSource::~DynamicPacketSource() {
        cancelAndDeleteClockEvent(parameterChangeEvent);


    }

} //namespace

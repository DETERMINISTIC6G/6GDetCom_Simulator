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
    }
}

void DynamicPacketSource::handleMessage(cMessage *msg)
{
    //ActivePacketSource::handleMessage(msg);
    if (msg == productionTimer) {
            if (consumer == nullptr || consumer->canPushSomePacket(outputGate->getPathEndGate())) {
                scheduleProductionTimer(productionIntervalParameter->doubleValue());
                producePacket();
            }
        } else if (msg == parameterChangeEvent) {
            EV << "Self-Message received immediately!\n";
            emit(DynamicScenarioObserver::parameterChangeSignal, msg);
        }
        else
            throw cRuntimeError("Unknown message");
}


/*  if (!strcmp(name, "delayDownlink") || !strcmp(name, "delayUplink") ) {
        cMsgPar *details = new cMsgPar("details");
        details->setStringValue(name);
        emit(DynamicScenarioObserver::distributionChangeSignal,
                distributionChangeEvent, details);
        delete details;
    }

 * */


void DynamicPacketSource::handleParameterChange(const char *name) {
    if (!strcmp(name, "enabled")) {
        enabledParameter = par("enabled");
        if (!enabledParameter) {
            if (productionTimer->isScheduled()) {
                cancelEvent(productionTimer);
            }
            cMsgPar *details = new cMsgPar("details");
            details->setStringValue("notEnabled");
            emit(DynamicScenarioObserver::parameterChangeSignal,
                    parameterChangeEvent, details);
             delete details;
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

    /*  */
    if (!parameterChangeEvent->isScheduled())
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

    UdpSocketIo *socketModule = dynamic_cast<UdpSocketIo*>(appModule->getSubmodule("io"));

    cValueMap *map = new cValueMap();


    map->set("name", cValue(""));
    map->set("pcp", cValue(4));
    map->set("gateIndex", cValue(1));

    map->set("application", cValue(appModule->getFullName()));
    map->set("source", cValue(deviceModule->getFullName()));

    map->set("destination", socketModule->par("destAddress").stringValue());

    map->set("packetLength", par("packetLength").intValue());
    map->set("packetInterval", par("productionInterval").doubleValue());
    map->set("maxLatency", cValue(0));

    //delete map;

    return map;//cValue(map);

}

DynamicPacketSource::~DynamicPacketSource() {
        cancelAndDeleteClockEvent(parameterChangeEvent);
       // cancelAndDelete(parameterChangeEvent);
    }

} //namespace

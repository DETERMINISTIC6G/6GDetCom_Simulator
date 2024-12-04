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

#include "ChangeMonitor.h"

namespace d6g {

Define_Module(ChangeMonitor);

void ChangeMonitor::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    // TODO - Generated method body
    if (stage == INITSTAGE_LOCAL) {
        schedulerCallDelayParameter = &par("schedulerCallDelay");
        observer = new DynamicScenarioObserver(this);

        timer = new ClockEvent("changeEventCollectionTimer");

        subscribe();

    }
}

void ChangeMonitor::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    if (msg == timer) {
        prepaireChangesForProcessing();
        externalSchedulerCall();
    }else
        throw cRuntimeError("Unknown message");

}

void ChangeMonitor::subscribe() {
    cSimulation *sim = getSimulation();
    for (int i = 0; i < sim->getLastComponentId(); ++i) {
        cModule *mod = sim->getModule(i);
        if (mod) {
            if (!strcmp(mod->getNedTypeName(),
                    "d6g.scenariomanager.ObservedScenarioManager")) { //"d6g.distribution.histogram.HistogramContainer")) {
                EV << "Subscribing module: " << mod->getFullPath() << " to the observer." << endl;
                mod->subscribe(DynamicScenarioObserver::scenarioEventSignal,
                        observer);
            }

            if (!strcmp(mod->getNedTypeName(),
                    "d6g.apps.dynamicsource.DynamicPacketSource")) {
                EV << "Subscribing module: " << mod->getFullPath() << " to the observer." << endl;
                mod->subscribe(DynamicScenarioObserver::parameterChangeSignal,
                        observer);
            }
            if (!strcmp(mod->getNedTypeName(), "d6g.devices.tsntranslator.TsnTranslator")
                    && mod->par("isDstt")){
                EV << "Subscribing module: " << mod->getFullPath() << " to the observer." << endl;
                mod->subscribe(
                        DynamicScenarioObserver::distributionChangeSignal,
                        observer);
            }

        }

    }
}

void ChangeMonitor::notify(std::string source) {
    bubble(("Changes in " + source).c_str());

    EV << "Changes in " << source << endl;
    if (!timer->isScheduled()) {
        scheduleClockEventAt(getClockTime() + schedulerCallDelayParameter->doubleValue(), timer);
    }
}



void ChangeMonitor::externalSchedulerCall() {

    int result = std::system("python3 scripts/dummy_scheduler.py");

}

void ChangeMonitor::prepaireChangesForProcessing() {
    ;
}


ChangeMonitor::~ChangeMonitor() {
    delete observer;
    cancelAndDeleteClockEvent(timer);
}

} //namespace

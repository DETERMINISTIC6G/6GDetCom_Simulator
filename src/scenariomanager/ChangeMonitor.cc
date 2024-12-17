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
    cSimpleModule::initialize(stage); //TSNschedGateScheduleConfigurator

    // TODO - Generated method body
    if (stage == INITSTAGE_LOCAL) {
        schedulerCallDelayParameter = &par("schedulerCallDelay");
        observer = new DynamicScenarioObserver(this);

        timer = new ClockEvent("changeEventCollectionTimer");

        subscribe();

        auto path = par("gateScheduleConfigurator").stringValue();
        gateScheduleConfigurator = check_and_cast<GateScheduleConfiguratorBase *>(getModuleByPath(path));
        configureMappings();
      //test
                cValueArray  *configArray = new cValueArray ();
                gateScheduleConfigurator->par("configuration").setObjectValue(configArray);

    }
}

void ChangeMonitor::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
    if (msg == timer) {
        prepaireChangesForProcessing();
        externalSchedulerCall();


        gateScheduleConfigurator->par("configuration").setObjectValue(convertToCValueArray(configMappings));

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



void ChangeMonitor::externalSchedulerCall() const {

   // int result = std::system("python3 scripts/dummy_scheduler.py");
    ;

}

void ChangeMonitor::prepaireChangesForProcessing() {
    ;
}

void ChangeMonitor::configureMappings()
{
    auto mappingParameter = check_and_cast<cValueArray *>(gateScheduleConfigurator->par("configuration").objectValue());


    configMappings.resize(mappingParameter->size());
    for (int i = 0; i < mappingParameter->size(); i++) {
        auto element = check_and_cast<cValueMap *>(mappingParameter->get(i).objectValue());
        Mapping &mapping = configMappings[i];

        mapping.name= element->get("name").stringValue();
        mapping.pcp = element->get("pcp").intValue();
        mapping.gateIndex = element->get("gateIndex").intValue();
        mapping.application = element->get("application").stringValue();
        mapping.source = element->get("source").stringValue();
        mapping.destination = element->get("destination").stringValue();

       /* mapping.packetLength = b(element->get("packetLength").doubleValueInUnit("b"));
        mapping.packetInterval = element->get("packetInterval").doubleValueInUnit("s");
        mapping.maxLatency = element->containsKey("maxLatency") ? element->get("maxLatency").doubleValueInUnit("s") : -1;
        mapping.maxJitter = element->containsKey("maxJitter") ? element->get("maxJitter").doubleValueInUnit("s") : 0;*/

        mapping.packetLength = element->get("packetLength");
        mapping.packetInterval = element->get("packetInterval");
        mapping.maxLatency = element->get("maxLatency");
        // mapping.maxJitter = element->get("maxJitter");

    }

    for (const auto& mapping : configMappings) {
            std::cout << mapping << std::endl;
        }
}

cValueArray* ChangeMonitor::convertToCValueArray(
        const std::vector<Mapping> &configMappings) {

    cValueArray *mappingParameter = new cValueArray();

    for (const auto &mapping : configMappings) {

        cValue temp = convertMappingToCValue(mapping);
        mappingParameter->add(temp);
    }

    return mappingParameter;
}

cValue ChangeMonitor::convertMappingToCValue(const Mapping &mapping) {

    cValueMap *map = new cValueMap();

    map->set("name", cValue(mapping.name));
    map->set("pcp", cValue(mapping.pcp));
    map->set("gateIndex", cValue(mapping.gateIndex));
    map->set("application", cValue(mapping.application));
    map->set("source", cValue(mapping.source));
    map->set("destination", cValue(mapping.destination));

    map->set("packetLength", mapping.packetLength);
    map->set("packetInterval", mapping.packetInterval);
    map->set("maxLatency", mapping.maxLatency);
    return cValue(map);
}




ChangeMonitor::~ChangeMonitor() {
    delete observer;
    cancelAndDeleteClockEvent(timer);
}

} //namespace


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

        distributions = new std::map<std::string, omnetpp::cValueArray*>();

        timer = new ClockEvent("changeEventCollectionTimer");

        subscribe();

        auto path = par("gateScheduleConfigurator").stringValue();
        gateScheduleConfigurator = check_and_cast<GateScheduleConfiguratorBase *>(getModuleByPath(path));

        configureMappings();

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

    if (!timer->isScheduled()) {
        scheduleClockEventAt(getClockTime() + schedulerCallDelayParameter->doubleValue(), timer);
    }
}



void ChangeMonitor::externalSchedulerCall() const {

   // int result = std::system("python3 scripts/dummy_scheduler.py");
    ;

}

void ChangeMonitor::prepaireChangesForProcessing() {

    // distribution changed
    //gateScheduleConfigurator->par("distribution").setObjectValue(distributions);

     gateScheduleConfigurator->par("configuration").setObjectValue(convertToCValueArray(configMappings));

     //distributions.clear();

}

void ChangeMonitor::configureMappings() {
    configMappings.clear();  //  configMappings.shrink_to_fit();

    auto mappingParameter = check_and_cast<cValueArray*>(
            gateScheduleConfigurator->par("configuration").objectValue());
    if (mappingParameter->size())
    {
        configMappings.resize(mappingParameter->size());
        for (int i = 0; i < mappingParameter->size(); i++)
        {
            auto element = check_and_cast<cValueMap*>(
                    mappingParameter->get(i).objectValue());
            createMapping(element, i);
            delete element;
        }
    }else
    {
        cSimulation *sim = getSimulation();
        int sourceNumber = 0;
        for (int i = 0; i < sim->getLastComponentId(); ++i) {
            cModule *mod = sim->getModule(i);
            if (mod
                    && !strcmp(mod->getNedTypeName(),
                            "d6g.apps.dynamicsource.DynamicPacketSource"))
            {

                DynamicPacketSource *sourceModule =
                        dynamic_cast<DynamicPacketSource*>(mod);
                cValueMap *element = sourceModule->getConfiguration();
                if (!element->get("enabled").boolValue()) {
                    delete element;
                    continue;
                }

                configMappings.resize(sourceNumber + 1);

                createMapping(element, sourceNumber);

                sourceModule->flowName = configMappings[sourceNumber].name;

                sourceNumber++;

                delete element;
            } else if (mod
                    && !strcmp(mod->getNedTypeName(),
                            "d6g.devices.tsntranslator.TsnTranslator")
                    && mod->par("isDstt"))
            { //translator
                TsnTranslator *sourceModule = dynamic_cast<TsnTranslator*>(mod);

                cValueArray *element = sourceModule->getDistribution("delayDownlink",
                        1000000);
                if (element->size())
                {
                    auto bridge = std::string(sourceModule->getParentModule()->getName()) + "."
                            + std::string(sourceModule->getFullName()) + "_"
                            + "Downlink";
                    updateDistributions(bridge, element);
                }else {
                    delete element;
                }

                element = sourceModule->getDistribution("delayUplink", 1000000);
                if (element->size())
                {
                    auto bridge = std::string(sourceModule->getParentModule()->getName()) + "."
                            + std::string(sourceModule->getFullName()) + "_"
                            + "Uplink";
                    updateDistributions(bridge, element);
                }else {
                    delete element;
                }

            } //translator
        }

    }

    for (const auto &mapping : configMappings) {
        std::cout << mapping << std::endl;
    }

}

void ChangeMonitor::createMapping(cValueMap *element, int i) {
    Mapping &mapping = configMappings[i];

    mapping.name = element->containsKey("name") ? element->get("name").stringValue() : (std::string("flow") + std::to_string(flowIndex++)).c_str();

    mapping.pcp = element->get("pcp").intValue();
    mapping.gateIndex = element->get("gateIndex").intValue();
    mapping.application = element->get("application").stringValue();
    mapping.source = element->get("source").stringValue();
    mapping.destination = element->get("destination").stringValue();

    mapping.packetLength = element->get("packetLength");
    mapping.packetInterval = element->get("packetInterval");
    mapping.maxLatency = element->get("maxLatency");

    // mapping.maxJitter = element->get("maxJitter");
}


void ChangeMonitor::updateMappings(cValueMap* element) {

    std::string application = element->get("application").stringValue();
    std::string source = element->get("source").stringValue();
    std::string destination = element->get("destination").stringValue();


    auto it = std::find_if(configMappings.begin(), configMappings.end(), [&](const ChangeMonitor::Mapping& m) {
                        return m.application == application && m.source == source && m.destination == destination;
              });

    if (it != configMappings.end()) { // found

        int i = std::distance(configMappings.begin(), it);

        if (element->get("enabled").boolValue()) {
            createMapping(element, i);


        } else {

            configMappings.erase(it);

            configMappings.shrink_to_fit();
        }

    } else { // not found
        if (!element->get("enabled").boolValue()) return;
        configMappings.resize(configMappings.size() + 1);
        createMapping(element, configMappings.size() - 1);
        auto path = (source + "." + application + ".source");
        DynamicPacketSource *sourceModule = check_and_cast<DynamicPacketSource*>(getModuleByPath(path.c_str()));
        sourceModule->flowName = configMappings[configMappings.size() - 1].name;

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


cValueArray* ChangeMonitor::getStreamConfigurations() {
    return convertToCValueArray(configMappings);
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

void ChangeMonitor::updateDistributions(std::string bridge,  cValueArray* element) {
    std::cout << "updateDistributions " << bridge << endl;

    auto it = distributions->find(bridge);
    if (it != distributions->end() && element->size()) {
            delete it->second;
          /*  if (!element->size()) {
                distributions->erase(it);
            }else {*/
                it->second = element;
            //}
        } else if (element->size()){
             (*distributions)[bridge] = element;
        }else {
            delete element;
        }

}

std::map<std::string, cValueArray*> *ChangeMonitor::getDistributions() {
    return distributions;
}


ChangeMonitor::~ChangeMonitor() {
    delete observer;

    //if (distributions != nullptr) {
        for (auto it = distributions->begin(); it != distributions->end(); ++it) {
              delete it->second;
        }
   // }

    delete distributions;
    cancelAndDeleteClockEvent(timer);
}

} //namespace


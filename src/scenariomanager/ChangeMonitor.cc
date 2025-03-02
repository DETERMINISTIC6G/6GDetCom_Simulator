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
#include "DynamicScenarioObserver.h"

#include <numeric> // lcm
#include <bits/stdc++.h>
#include <algorithm>


namespace d6g {



Define_Module(ChangeMonitor);

void ChangeMonitor::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        schedulerCallDelayParameter = &par("schedulerCallDelay");
        observer = new DynamicScenarioObserver(this);

        distributions = new std::map<std::string, omnetpp::cValueArray*>();

        timer = new ClockEvent("changeEventCollectionTimer");

        subscribeForDynamicChanges();

        auto path = par("gateScheduleConfigurator").stringValue();
        gateScheduleConfigurator = check_and_cast<GateScheduleConfiguratorBase *>(getModuleByPath(path));


    } else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {

        configureMappings();

    }
}

void ChangeMonitor::handleMessage(cMessage *msg)
{
    if (msg == timer) {
        prepaireChangesForProcessing();
    }else
        throw cRuntimeError("Unknown message");

}

void ChangeMonitor::subscribeForDynamicChanges() {
    cSimulation *sim = getSimulation();
    for (int i = 0; i < sim->getLastComponentId(); ++i) {
        cModule *mod = sim->getModule(i);
        if (mod) {
            if (!strcmp(mod->getModuleType()->getName(), "ObservedScenarioManager")) {
            //(!strcmp(mod->getNedTypeName(), "d6g.scenariomanager.ObservedScenarioManager")) {
                EV << "Subscribing module: " << mod->getFullPath() << " to the observer." << endl;
                mod->subscribe(DynamicScenarioObserver::scenarioEventSignal, observer);
            }

            if (!strcmp(mod->getModuleType()->getName(), "DynamicPacketSource")) {
            //(!strcmp(mod->getNedTypeName(), "d6g.apps.dynamicsource.DynamicPacketSource")) {
                EV << "Subscribing module: " << mod->getFullPath() << " to the observer." << endl;
                mod->subscribe(DynamicScenarioObserver::parameterChangeSignal, observer);
            }
            if (!strcmp(mod->getModuleType()->getName(), "TsnTranslator") && mod->par("isDstt")) {
            //(!strcmp(mod->getNedTypeName(), "d6g.devices.tsntranslator.TsnTranslator") && mod->par("isDstt")){
                EV << "Subscribing module: " << mod->getFullPath() << " to the observer." << endl;
                mod->subscribe(DynamicScenarioObserver::distributionChangeSignal, observer);
            }

        }

    }//endfor
}


void ChangeMonitor::notify(std::string source) {
    bubble(("Changes in " + source).c_str());

    if (!timer->isScheduled()) {
        scheduleClockEventAt(getClockTime() + schedulerCallDelayParameter->doubleValue(), timer);
    }
}


void ChangeMonitor::prepaireChangesForProcessing() {

    //gateScheduleConfigurator->par("gateCycleDuration").setValue(cValue(res, "ns"));
    gateScheduleConfigurator->par("configuration").setObjectValue(getStreamConfigurations());

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

               addEntriesToDistributionMapFor(sourceModule);
            } //translator
        }

    }

    for (const auto &mapping : configMappings) {
        std::cout << mapping << std::endl;
    }

}

void ChangeMonitor::addEntriesToDistributionMapFor(TsnTranslator *translator) {
    auto delayParam = {"Uplink", "Downlink"};
    for (auto param : delayParam) {
        auto expr = translator->getDistribution(("delay" + std::string(param)).c_str());
        auto element = observer->createHistogram(*expr);
        delete expr;
        /*if (!element->size()) {
            delete element;
            continue;
        }*/
        auto bridge = std::string(translator->getParentModule()->getName()) + "." + std::string(translator->getFullName()) + "_" + std::string(param);
        //(*distributions)[bridge] = element;
        updateDistributions(bridge, element);
    }//endfor
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

    mapping.maxJitter = element->get("maxJitter");
    mapping.reliability = element->get("reliability").doubleValue();
    mapping.policy = element->get("policy").intValue();
    mapping.phase = element->get("phase");
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


cValueArray* ChangeMonitor::convertToCValueArray(const std::vector<Mapping> &configMappings) {
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
    map->set("maxJitter", mapping.maxJitter);
    map->set("reliability", mapping.reliability);
    map->set("policy", mapping.policy);

    map->set("phase", mapping.phase);

    return cValue(map);
}

void ChangeMonitor::updateDistributions(std::string bridge,  cValueArray* element) {
    std::cout << "updateDistributions " << bridge << endl;

    auto it = distributions->find(bridge);
    if (it != distributions->end()) {
            delete it->second;
            //this->drop(x);
            if (!element->size()) {
                distributions->erase(it);
                delete element;
                return;
            }
            it->second = element;
        } else {
            if (!element->size()) {
                delete element;
                return;
            }
            //if (element->size())
            (*distributions)[bridge] = element;
        }
}

std::map<std::string, cValueArray*> *ChangeMonitor::getDistributions() {
    return distributions;
}

cValueArray* ChangeMonitor::getStreamConfigurations() {
    return convertToCValueArray(configMappings);
}

ChangeMonitor::~ChangeMonitor() {
    for (auto it = distributions->begin(); it != distributions->end(); ++it) {
         delete it->second;
    }
    delete distributions;
    cancelAndDeleteClockEvent(timer);
    delete observer;
}


} //namespace


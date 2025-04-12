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

#include <numeric> // lcm

#include "DynamicScenarioObserver.h"

namespace d6g {

Define_Module(ChangeMonitor);

void ChangeMonitor::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        schedulerCallDelayParameter = &par("schedulerCallDelay");
        timer = new ClockEvent("changeEventCollectionTimer");

        fixedPriority = par("isPortQueuesCountConsistentGlobally").boolValue();
        pcpMapping = check_and_cast<cValueArray *>(par("mapping").objectValue());

        observer = new DynamicScenarioObserver(this);
        distributions = new std::map<std::string, cValueArray *>();

        auto path = par("gateScheduleConfigurator").stringValue();
        gateScheduleConfigurator = check_and_cast<GateScheduleConfiguratorBase *>(getModuleByPath(path));

        subscribeForDynamicChanges();
    }
    else if (stage == INITSTAGE_NETWORK_INTERFACE_CONFIGURATION) {
        configureInitStreamsAndDistributions();
        prepareChangesForProcessing(0); //
    }
}

void ChangeMonitor::handleMessage(cMessage *msg)
{
    if (msg == timer) {
        std::cout << "MONITOR: scheduler call at " << simTime() << "s" << endl;
        prepareChangesForProcessing(1);
    }
    else {
        throw cRuntimeError("Unknown message type.");
    }
}

void ChangeMonitor::subscribeForDynamicChanges()
{
    cSimulation *sim = getSimulation();
    for (int i = 0; i < sim->getLastComponentId(); ++i) {
        cModule *mod = sim->getModule(i);
        if (mod) {
            if (!strcmp(mod->getModuleType()->getName(), "ObservedScenarioManager")) {
                EV << "Subscribing module: " << mod->getFullPath() << " to the observer." << endl;
                mod->subscribe(DynamicScenarioObserver::scenarioEventSignal, observer);
            }
            if (!strcmp(mod->getModuleType()->getName(), "DynamicPacketSource")) {
                EV << "Subscribing module: " << mod->getFullPath() << " to the observer." << endl;
                mod->subscribe(DynamicScenarioObserver::parameterChangeSignal, observer);
            }
            if (!strcmp(mod->getModuleType()->getName(), "TsnTranslator") && mod->par("isDstt"))
            { // subscribe only DSTT
                EV << "Subscribing module: " << mod->getFullPath() << " to the observer." << endl;
                mod->subscribe(DynamicScenarioObserver::distributionChangeSignal, observer);
            }
        } // endif
    } // endfor
}

/**
 * @brief  Set the updated parameters of a GateScheduleConfigurator after the
 * timer expires.
 *
 * This method calculates the new hyperperiod [ns] and
 * updates the configuration parameters, which activates a
 * GateScheduleConfigurator.
 *
 * @param  0 if initial scheduling calculation, other number otherwise.
 */
void ChangeMonitor::prepareChangesForProcessing(int initialized)
{
    int hyperperiod = 1;

    std::vector<long> intervals;
    for (const auto &mapping : streamConfigurations) {
        intervals.push_back(std::round(mapping.packetInterval.doubleValueInUnit("ns")));
    }
    for (long interval : intervals) {
        hyperperiod = std::lcm(hyperperiod, interval);
    }

    gateScheduleConfigurator->par("gateCycleDuration").setValue(cValue(hyperperiod, "ns"));

    if (initialized) {
        gateScheduleConfigurator->par("configuration").setObjectValue(nullptr);
    }
}

void ChangeMonitor::configureInitStreamsAndDistributions()
{
    streamConfigurations.clear(); // configMappings.shrink_to_fit();

    // read if the configurations were set in the .ini file
    auto mappingParameter = check_and_cast<cValueArray *>(gateScheduleConfigurator->par("configuration").objectValue());
    if (mappingParameter->size()) {
        streamConfigurations.resize(mappingParameter->size());
        for (int i = 0; i < mappingParameter->size(); i++) {
            auto element = check_and_cast<cValueMap *>(mappingParameter->get(i).objectValue());
            addEntryToStreamConfigurations(element, i);
            delete element;
        }
        // request the configurations themselves otherwise
    }
    else {
        cSimulation *sim = getSimulation();
        int sourceNumber = 0;
        for (int i = 0; i < sim->getLastComponentId(); ++i) {
            cModule *mod = sim->getModule(i);
            if (mod && !strcmp(mod->getNedTypeName(), "d6g.apps.dynamicsource.DynamicPacketSource")) { // app source
                                                                                                       // module
                DynamicPacketSource *sourceModule = dynamic_cast<DynamicPacketSource *>(mod);
                cValueMap *element = sourceModule->getConfiguration();
                if ((element->get("stopReq").boolValue())) {
                    // if (!element->get("enabled").boolValue()) {  // not enabled
                    delete element;
                    continue;
                }
                streamConfigurations.resize(sourceNumber + 1);
                addEntryToStreamConfigurations(element, sourceNumber);
                sourceModule->streamName = streamConfigurations[sourceNumber].name; // set unique flow name
                sourceNumber++;
                delete element;
            }
            else if (mod && !strcmp(mod->getNedTypeName(), "d6g.devices.tsntranslator.TsnTranslator") &&
                     mod->par("isDstt"))
            { // DSTT translator
                TsnTranslator *sourceModule = dynamic_cast<TsnTranslator *>(mod);
                addEntriesToDistributionsFor(sourceModule);
            } // translator
        } // endfor
    }
    // #################################################
    for (const auto &mapping : streamConfigurations) {
        std::cout << mapping << std::endl;
    }
    // #################################################
}

void ChangeMonitor::addEntriesToDistributionsFor(TsnTranslator *translator)
{ // both: Uplink and Downlink
    auto delayParam = {"Uplink", "Downlink"};
    for (std::string param : delayParam) {
        auto dynExpr = translator->getDistributionExpression(("delay" + param).c_str());
        auto element = observer->createHistogram(*dynExpr);
        take(element);
        drop(element);
        auto key = std::string(translator->getParentModule()->getName()) + "." +
                   std::string(translator->getFullName()) + "_" + param;
        updateDistributions(key, element);
        delete dynExpr;
    } // endfor
}

void ChangeMonitor::addEntryToStreamConfigurations(cValueMap *element, int i)
{
    Mapping &mapping = streamConfigurations[i];

    mapping.name = element->containsKey("name") ? element->get("name").stringValue()
                                                : (std::string("flow") + std::to_string(flowIndex++)).c_str();

    // mapping.stopReq = element->get("stopReq").boolValue();

    mapping.pcp = element->get("pcp").intValue();

    mapping.gateIndex = // element->get("gateIndex").intValue();
        element->containsKey("gateIndex") ? element->get("gateIndex").intValue() : classify(mapping.pcp);

    mapping.application = element->get("application").stringValue();
    mapping.source = element->get("source").stringValue();
    mapping.destination = element->get("destination").stringValue();

    mapping.packetLength = element->get("packetLength");
    mapping.packetInterval = element->get("packetInterval");
    mapping.maxLatency = element->get("maxLatency");

    mapping.maxJitter = element->get("maxJitter");
    mapping.reliability = element->get("reliability").doubleValue();
    // mapping.policy = element->get("policy").intValue();
    mapping.phase = element->get("phase");
    // mapping.packetLoss = element->get("packetLoss").intValue();
    mapping.objectiveType = element->get("objectiveType").intValue();
    // mapping.weight = element->get("weight").doubleValue();
}

int ChangeMonitor::classify(int pcp)
{
    if (!fixedPriority)
        return pcp;
    int numTrafficClasses = par("globallyNumTrafficClasses").intValue();
    if (numTrafficClasses > pcpMapping->size())
        return 0;
    if (pcp < 0 || pcp > pcpMapping->size() - 1)
        return 0;
    auto pcpToGateIndex = check_and_cast<cValueArray *>(pcpMapping->get(pcp).objectValue());
    return pcpToGateIndex->get(numTrafficClasses - 1);
}

cValueArray *ChangeMonitor::convertToCValueArray(const std::vector<Mapping> &configMappings)
{
    cValueArray *mappingParameter = new cValueArray();
    for (const auto &mapping : configMappings) {
        auto temp = convertMappingToCValue(mapping);
        mappingParameter->add(temp);
    }
    return mappingParameter;
}

cValueMap *ChangeMonitor::convertMappingToCValue(const Mapping &mapping)
{
    cValueMap *map = new cValueMap();
    map->set("name", cValue(mapping.name));
    // map->set("stopReq", cValue(mapping.stopReq));
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
    // map->set("policy", mapping.policy);
    map->set("phase", mapping.phase);
    // map->set("packetLoss", mapping.packetLoss);
    map->set("objectiveType", mapping.objectiveType);
    // map->set("weight", mapping.weight);

    return map;
}

void ChangeMonitor::updateStreamConfigurations(cValueMap *element)
{
    take(element);
    std::string application = element->get("application").stringValue();
    std::string source = element->get("source").stringValue();
    std::string destination = element->get("destination").stringValue();

    auto it =
        std::find_if(streamConfigurations.begin(), streamConfigurations.end(), [&](const ChangeMonitor::Mapping &m) {
            return m.application == application && m.source == source && m.destination == destination;
        });
    if (it != streamConfigurations.end()) { // found
        int i = std::distance(streamConfigurations.begin(), it);
        if (!(element->get("stopReq").boolValue())) { // enabled  //
                                                      // element->get("enabled").boolValue() ||

            addEntryToStreamConfigurations(element, i);
        }
        else { // not enabled
            if (element->get("enabled").boolValue())
                streamWantsToStop.push_back(source + "." + application + ".source");
            streamConfigurations.erase(it);
            streamConfigurations.shrink_to_fit();
        }
    }
    else                                              //{ // not found
        if (!(element->get("stopReq").boolValue())) { // element->get("enabled").boolValue())
                                                      // { // enabled
            // return;

            streamConfigurations.resize(streamConfigurations.size() + 1);
            addEntryToStreamConfigurations(element, streamConfigurations.size() - 1);
            auto path = (source + "." + application + ".source");
            DynamicPacketSource *sourceModule = check_and_cast<DynamicPacketSource *>(getModuleByPath(path.c_str()));
            if (sourceModule->streamName == "") {
                sourceModule->streamName = streamConfigurations[streamConfigurations.size() - 1].name;
                std::cout << streamConfigurations[streamConfigurations.size() - 1] << std::endl;
            }
            if (element->get("enabled").boolValue()) { // #######
                std::cout << "already started at " << simTime() << " : "
                          << std::string(element->get("source").stringValue()) + "." +
                                 std::string(element->get("application").stringValue())
                          << endl;
            }
        }

    if (!element->get("stopReq").boolValue() && !element->get("enabled").boolValue()) { // #######
        std::cout << "want to run at " << simTime() << " : "
                  << std::string(element->get("source").stringValue()) + "." +
                         std::string(element->get("application").stringValue())
                  << endl;
    }
    else if (element->get("stopReq").boolValue() && !element->get("enabled").boolValue()) { // #######
        std::cout << "already stopped at " << simTime() << " : "
                  << std::string(element->get("source").stringValue()) + "." +
                         std::string(element->get("application").stringValue())
                  << endl;
    }
}

void ChangeMonitor::updateDistributions(std::string key, cValueArray *element)
{
    take(element);
    auto it = distributions->find(key);
    if (it != distributions->end() && it->second != nullptr) { // replace if found
        take(it->second);
        drop(it->second);
        delete it->second;
        it->second = nullptr;

        if (!element->size()) { // delete entry if no new histogram
            distributions->erase(it);

            delete element;
            return;
        }
        it->second = element;
    }
    else { // not found
        if (!element->size()) {
            drop(element);
            delete element; // delete if no new histogram
            return;
        }
        (*distributions)[key] = element; // create a new entry
    }
}

std::map<std::string, cValueArray *> *ChangeMonitor::getDistributions() { return distributions; }

cValueArray *ChangeMonitor::getStreamConfigurations() { return convertToCValueArray(streamConfigurations); }

/**
 * @brief Add the source modules with a stop-production request
 *          to the sources vector in the output
 *
 */
void ChangeMonitor::addApplicationsWithStopReq(std::vector<cModule *> &sources)
{
    for (auto &appPath : streamWantsToStop) {
        auto application = getModuleByPath(appPath.c_str());
        std::cout << "want to stop at " << simTime() << " : " << appPath << endl;
        sources.push_back(application);
    }
    streamWantsToStop.clear();
}

/**
 * @brief  Notify Monitor module
 *
 *
 *
 *
 * @param  Module name responsible for notifying the monitor
 * @param  (optional)
 *
 */
void ChangeMonitor::scheduleTimer(std::string source, cObject *details)
{
    bubble(("Changes in " + source + " announced.").c_str());

    if (!timer->isScheduled()) {
        scheduleClockEventAt(getClockTime() + schedulerCallDelayParameter->doubleValue(), timer);
    }
}

void ChangeMonitor::computeConvolution(cModule *source, cModule *target)
{
    auto key =
        std::string(source->getParentModule()->getName()) + "." + source->getFullName() + "-" + target->getFullName();
    auto expr1 = ((TsnTranslator *)source)->getDistributionExpression("delayUplink");
    auto expr2 = ((TsnTranslator *)target)->getDistributionExpression("delayDownlink");

    auto convString = expr1->str() + "+" + expr2->str();
    delete expr1;
    delete expr2;

    auto convolveExpr = new cDynamicExpression();
    convolveExpr->parse(convString.c_str());

    cMsgPar *histogramDetails = new cMsgPar("convolution");
    auto element = observer->createHistogram(*convolveExpr, histogramDetails);
    this->take(element);
    updateDistributions(key, element);

    delete convolveExpr;
    delete histogramDetails;
}

ChangeMonitor::~ChangeMonitor()
{
    if (distributions != nullptr) {
        for (auto &pair : *distributions) {
            if (pair.second != nullptr) {
                delete pair.second;
            }
        }
        delete distributions;
    }
    cancelAndDeleteClockEvent(timer);
    delete observer;
}

} // namespace d6g

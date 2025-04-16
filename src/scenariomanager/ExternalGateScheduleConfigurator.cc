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

#include "ExternalGateScheduleConfigurator.h"

#include <fstream>
#include <numeric>

namespace d6g {

Define_Module(ExternalGateScheduleConfigurator);

void ExternalGateScheduleConfigurator::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        command = par("command").stdstringValue();

        networkFilePar = &par("networkFile");
        streamsFilePar = &par("streamsFile");
        histogramsFilePar = &par("histogramsFile");
        configurationFilePar = &par("configurationFile");

        configurationComputedEvent = new ClockEvent("configuration_computed");
        configurationComputedEvent->setSchedulingPriority(0);

        hashMapNodeId = new std::map<std::string, uint16_t>();

        auto schedulerRootEnv = getenv("SCHEDULER_ROOT");
        if (!schedulerRootEnv) {
            throw cRuntimeError("SCHEDULER_ROOT environment variable not set");
        }
        schedulerRoot = schedulerRootEnv;
        // Check if path exists
        if (!exists(schedulerRoot)) {
            throw cRuntimeError("SCHEDULER_ROOT path does not exist");
        }
    }
    else if (stage == INITSTAGE_GATE_SCHEDULE_CONFIGURATION) {
        // query monitor if it exists
        if ((monitor = check_and_cast<ChangeMonitor *>(getModuleByPath("monitor")))) {
            configuration = monitor->getStreamConfigurations();
            computeConfiguration();
            configureGateScheduling();
            configureApplicationOffsets();
            clearConfiguration();
        } // endif
        if (!monitor)
            throw cRuntimeError("Monitor module not found!");
    }
}

void ExternalGateScheduleConfigurator::handleMessage(cMessage *msg)
{
    if (msg == configurationComputedEvent) {
        if (((Output *)gateSchedulingOutput)->hasSchedule()) {
            configureGateScheduling();
            configureApplicationOffsets();
        }
    }
    else
        throw cRuntimeError("Unknown message type");
}

void ExternalGateScheduleConfigurator::handleParameterChange(const char *name)
{
    if (!strcmp(name, "configuration")) {
        clearConfiguration();
        configuration = monitor->getStreamConfigurations();
        if (configurationComputedEvent->isScheduled()) {
            cancelEvent(configurationComputedEvent);
        }
        computeConfiguration();
        configurationComputedEvent->setSchedulingPriority(0);
        scheduleAt(commitTime, configurationComputedEvent);
    } // endif

    if (!strcmp(name, "gateCycleDuration")) {
        gateCycleDuration = par("gateCycleDuration");
    }
}

void ExternalGateScheduleConfigurator::printJson(std::ostream &stream, const cValue &value, int level) const
{ // not static
    std::string indent(level * 2, ' ');
#if OMNETPP_BUILDNUM < 1527
    if (value.getType() == cValue::OBJECT) {
#else
    if (value.getType() == cNedValue::POINTER && value.containsObject()) {
#endif
        auto object = value.objectValue();
        if (auto array = dynamic_cast<cValueArray *>(object)) {
            if (array->size() == 0)
                stream << "[]";
            else {
                stream << "[\n";
                for (int i = 0; i < array->size(); i++) {
                    if (i != 0)
                        stream << ",\n";
                    stream << indent << "  ";
                    printJson(stream, array->get(i), level + 1);
                }
                stream << "\n" << indent << "]";
            }
        }
        else if (auto map = dynamic_cast<cValueMap *>(object)) {
            if (map->size() == 0)
                stream << "{}";
            else {
                stream << "{\n";
                auto it = map->getFields().begin();
                for (int i = 0; i < map->size(); i++, ++it) {
                    if (i != 0)
                        stream << ",\n";
                    stream << indent << "  \"" << it->first << "\": ";
                    auto originalSecond = it->second;
                    cValue newSecond(originalSecond);
                    if (originalSecond.isNumeric()) {
                        if (originalSecond.getUnit()) {
                            auto valueRaw = std::to_string(originalSecond.doubleValueRaw());
                            newSecond.set(valueRaw + std::string(originalSecond.getUnit()));
                        }
                        else {
                            newSecond = std::isinf(originalSecond.doubleValue())
                                            ? (originalSecond.doubleValue() > 0 ? cValue(1e308) : cValue(-1e308))
                                            : originalSecond;
                        }
                    }
                    printJson(stream, newSecond, level + 1);
                }
                stream << "\n" << indent << "}";
            }
        }
        else
            throw cRuntimeError("Unknown object type");
    }
    else
        stream << value.str();
}


ExternalGateScheduleConfigurator::Output *
ExternalGateScheduleConfigurator::computeGateScheduling(const Input &input) const
{
    writeStreamsToFile(input);
    writeNetworkToFile(input);
    writeDistributionsToFile();
    auto filepath = schedulerRoot / configurationFilePar->stdstringValue();

    invokeScheduler();
    Output *scheduleOutput = (Output *)readOutputFromFile(input, filepath);
    return scheduleOutput;
}

void ExternalGateScheduleConfigurator::invokeScheduler() const
{
    char currentDir[PATH_MAX];
    getcwd(currentDir, sizeof(currentDir));
    chdir(schedulerRoot.c_str());
    std::string commandFromINI =
        format(command, networkFilePar->stdstringValue().c_str(), streamsFilePar->stdstringValue().c_str(),
               histogramsFilePar->stdstringValue().c_str(), configurationFilePar->stdstringValue().c_str());
    if (std::system(commandFromINI.c_str()) != 0) {
        auto eventTime = simTime().dbl() - monitor->par("schedulerCallDelay").doubleValueInUnit("s");
        std::string errorMsg =
                format("No schedule for the event at %.2fs has been calculated.",
                eventTime > 0 ? eventTime : 0);
        throw cRuntimeError("%s", errorMsg.c_str());
    }
    chdir(currentDir);
}

// ####################### WRITE ###########################

void ExternalGateScheduleConfigurator::writeDistributionsToFile() const
{
    cValueMap *json = new cValueMap();
    cValueArray *jsonDistributions = new cValueArray();
    json->set("distributions", jsonDistributions);
    auto distributions = monitor->getDistributions();
    if (distributions != nullptr) {
        for (const auto &pair : *distributions) {
            auto bridgeName = pair.first;
            cValueArray *histogram = pair.second;
            cValueMap *entry = new cValueMap();
            entry->set("name", bridgeName);
            entry->set("data", histogram);
            entry->set("type", histogram->getName());
            jsonDistributions->add(cValue(entry));
        }
    } // endif
    write(histogramsFilePar->stdstringValue(), json);
    delete json;
}

void ExternalGateScheduleConfigurator::writeStreamsToFile(const Input &input) const
{
    auto jsonStreams = convertInputToJsonStreams(input);
    cValueMap *metaData = new cValueMap();
    jsonStreams->set("META", metaData);
    metaData->set("simulation_time", simTime().inUnit(SIMTIME_NS));
    metaData->set("simulation_time_unit", "ns");
    metaData->set("schedule_type", "Hypercycle");
    metaData->set("fixed_priority", monitor->isFixedPriority());
    write(streamsFilePar->stdstringValue(), jsonStreams);
    delete jsonStreams;
}

void ExternalGateScheduleConfigurator::writeNetworkToFile(const Input &input) const
{
    auto jsonNetwork = convertInputToJsonNetwork(input);
    cValueArray *jsonIDs = new cValueArray();
    jsonNetwork->set("id_mapping", jsonIDs);
    for (const auto &pair : *hashMapNodeId) {
        cValueMap *json = new cValueMap();
        json->set("node", pair.first);
        json->set("id", pair.second);
        jsonIDs->add(json);
    }
    write(networkFilePar->stdstringValue(), jsonNetwork);
    delete jsonNetwork;
}

void ExternalGateScheduleConfigurator::write(std::string fileName, cValueMap *json) const
{
    std::ofstream stream;
    auto path = schedulerRoot / fileName;
    stream.open(path);
    if (stream.fail())
        throw cRuntimeError("Cannot open file %s", fileName.c_str());
    this->printJson(stream, cValue(json));
}

// ############################# CONVERT INPUT JSON ###########################

cValueMap *ExternalGateScheduleConfigurator::convertInputToJsonStreams(const Input &input) const
{
    cValueMap *json = new cValueMap();
    cValueArray *jsonFlows = new cValueArray();
    json->set("flows", jsonFlows);

    for (auto flow : input.flows) {
        cValueMap *jsonFlow = new cValueMap();
        jsonFlows->add(jsonFlow);
        jsonFlow->set("name", flow->name);
        jsonFlow->set("type", "unicast");

        Application *dynApplication = (Application *)(flow->startApplication);

        jsonFlow->set("source_device", dynApplication->device->module->getFullName());
        jsonFlow->set("pcp", flow->gateIndex);
        jsonFlow->set("packet_periodicity", dynApplication->packetInterval.inUnit(SIMTIME_NS));
        jsonFlow->set("packet_periodicity_unit", "ns");
        jsonFlow->set("packet_size", b(dynApplication->packetLength).get());
        jsonFlow->set("packet_size_unit", "bit");
        jsonFlow->set("hard_constraint_time_latency", dynApplication->maxLatency.inUnit(SIMTIME_NS));
        jsonFlow->set("hard_constraint_time_jitter", dynApplication->maxJitter.inUnit(SIMTIME_NS));
        jsonFlow->set("hard_constraint_time_unit", "ns");
        jsonFlow->set("phase", dynApplication->phase.inUnit(SIMTIME_NS));
        jsonFlow->set("phase_unit", "ns");
        jsonFlow->set("custom_parameters", dynApplication->customParams);
        jsonFlow->set("target_device", cValue(flow->endDevice->module->getFullName()));
        // packet delay budget
        cValueArray *pdb_map = new cValueArray();
        jsonFlow->set("pdb_map", pdb_map);
        cValueArray *hops = new cValueArray();
        jsonFlow->set("route", hops);
        bool isWirelessPath = false;
        for (int j = 0; j < flow->pathFragments.size(); j++) {
            auto pathFragment = flow->pathFragments[j];
            for (int k = 0; k < pathFragment->networkNodes.size() - 1; k++) {
                auto networkNode = pathFragment->networkNodes[k];
                auto nextNetworkNode = pathFragment->networkNodes[k + 1];
                cValueMap *hop = new cValueMap();
                hops->add(hop);
                auto nameNetworkNode = getExpandedNodeName(networkNode->module);
                hop->set("currentNodeName", nameNetworkNode);
                auto nameNextNetworkNode = getExpandedNodeName(nextNetworkNode->module);
                hop->set("nextNodeName", nameNextNetworkNode);
                // pdb map: find a distribution
                isWirelessPath |= addEntryToPDBMap(pdb_map, networkNode->module, nextNetworkNode->module);
            } // 3. for
        } // 2. for
        jsonFlow->set("isWirelessPath", isWirelessPath);
        int pdbSize = pdb_map->size();
        // #DetCom-link-th root of the reliability parameter
        double reliability = std::pow(dynApplication->reliability, 1.0 / pdbSize);
        for (int i = 0; i < pdbSize; i++) {
            auto vMap = check_and_cast<cValueMap *>(pdb_map->get(i).objectValue());
            vMap->set("reliability", reliability);
        } // endfor
    } // 1.for
    return json;
}

cValueMap *ExternalGateScheduleConfigurator::convertInputToJsonNetwork(const Input &input) const
{
    cValueMap *json = new cValueMap();
    cValueArray *jsonDevices = new cValueArray();
    json->set("end_devices", jsonDevices);
    for (auto device_ : input.devices) {
        cValueMap *jsonDevice = convertJsonDevice(device_);
        jsonDevices->add(jsonDevice);
    }
    cValueArray *jsonSwitches = new cValueArray();
    json->set("switches", jsonSwitches);
    for (auto switch_ : input.switches) {
        cValueMap *jsonSwitch = convertJsonDevice(switch_);
        jsonSwitches->add(jsonSwitch);
    }
    return json;
}

cValueMap *ExternalGateScheduleConfigurator::convertJsonDevice(Input::NetworkNode *device) const
{
    cValueMap *jsonDevice = new cValueMap();

    auto nameNetworkNode = getExpandedNodeName(device->module);
    jsonDevice->set("name", nameNetworkNode);
    jsonDevice->set("device_type", getDeviceType(device->module));
    jsonDevice->set("processing_delay", 0); // temporary hard coded
    jsonDevice->set("processing_delay_unit", "ns");

    (*hashMapNodeId)[nameNetworkNode] = device->module->getId();

    auto jsonDevicePorts = new cValueArray();
    jsonDevice->set("ports", jsonDevicePorts);
    for (int j = 0; j < device->ports.size(); j++) {
        auto port = device->ports[j];
        auto jsonPort = new cValueMap();
        jsonDevicePorts->add(jsonPort);

        auto nameNode = getExpandedNodeName(port->startNode->module);
        jsonPort->set("port_name", nameNode + "-" + port->module->getFullName());
        auto connectedToFullName = getExpandedNodeName(port->endNode->module);
        jsonPort->set("connects_to", connectedToFullName);

        DetComLinkType detComLinkType;
        short link_type = isDetComLink(port->startNode->module, port->endNode->module, detComLinkType);
        jsonPort->set("link_type_descr", getDetComLinkDescription(detComLinkType));
        jsonPort->set("link_type", link_type);
        if (link_type)
            jsonPort->set("multiple_subcarriers", true);
        jsonPort->set("propagation_delay", port->propagationTime.inUnit(SIMTIME_NS));
        jsonPort->set("propagation_delay_unit", "ns");
        jsonPort->set("data_rate", bps(port->datarate).get());
        jsonPort->set("data_rate_unit", "bps");
        auto queue = port->module->findModuleByPath(".macLayer.queue");
        jsonPort->set("num_queues", strcmp(queue->getModuleType()->getName(), "PacketQueue")
                                        ? queue->par("numTrafficClasses").intValue()
                                        : 0);
    }
    return jsonDevice;
}

// ############################# CONVERT OUTPUT JSON ###########################

ExternalGateScheduleConfigurator::Output *
ExternalGateScheduleConfigurator::convertJsonToOutput(const Input &input, const cValueMap *json) const
{
    // META DATA
    auto jsonMETA = check_and_cast<cValueMap *>(json->get("META").objectValue());
    int scheduled = jsonMETA->containsKey("scheduled") ? jsonMETA->get("scheduled").intValue() : 0;
    if ((monitor->par("stopWhenNotSchedulable").boolValue()
          && input.flows.size() != scheduled) || !scheduled) {
        throw cRuntimeError("The simulation was halted due to the incomplete scheduling of flows.");
    }
    commitTime = SimTime(jsonMETA->get("commit_time").intValue(), SIMTIME_NS);
    gateCycleDuration = SimTime(jsonMETA->get("hypercycle").intValue(), SIMTIME_NS);

    auto output = new Output();

    // GATE CONTROL LISTS
    auto jsonGCL = check_and_cast<cValueMap *>(json->get("GCL").objectValue());
    for (auto &gcl : jsonGCL->getFields()) { // 1.for
        auto linkName = gcl.first;
        auto port = getConfigurablePort(input, linkName);
        auto &schedules = output->gateSchedules[port];
        auto queues = check_and_cast<cValueMap *>(gcl.second.objectValue());
        for (auto &queue : queues->getFields()) { // 2.for
            auto queueIndex = std::stoi(queue.first.substr(1));
            cValueMap *queueMap = check_and_cast<cValueMap *>(queue.second.objectValue());
            auto schedule = new Schedule();
            schedule->port = port;
            schedule->gateIndex = queueIndex;
            schedule->cycleDuration = gateCycleDuration;
            schedule->open = queueMap->get("initial").intValue();

            auto dur = check_and_cast<cValueArray *>(queueMap->get("durations").objectValue());
            schedule->durations = new cValueArray();
            for (int i = 0; i < dur->size(); i++) {
                const cValue &value = (*dur)[i];
                cValue newValue = cValue(value.intValue(), "ns");
                schedule->durations->add(newValue);
            }
            dur = nullptr;
            schedule->offset = SimTime(queueMap->get("offset").intValue(), SIMTIME_NS);
            schedules.push_back(schedule);
        } // 2.endfor
    } // 1.endfor

    // APPLICATIONS
    auto talkers = check_and_cast<cValueMap *>(json->get("TALKERS").objectValue());
    for (auto &field : talkers->getFields()) {
        const std::string &key = field.first;
        std::string flowID, packetNr;
        parseString(key, flowID, packetNr, '#');

        auto flowIt = std::find_if(input.flows.begin(), input.flows.end(),
                                   [&](Input::Flow *flow) { return flow->name == flowID; });
        if (flowIt == input.flows.end())
            throw cRuntimeError("Cannot find flow: %s", flowID.c_str());
        auto flow = *flowIt;
        auto application = flow->startApplication;
        auto startTime = SimTime(field.second.intValue(), SIMTIME_NS);
        if (output->applicationStartTimes.find(application) == output->applicationStartTimes.end()) {
            output->applicationStartTimes[application] = startTime;
        } // endif
        output->applicationStartTimesArray[application].push_back(startTime);
    } // endfor applications
    for (auto &offsetsPair : output->applicationStartTimesArray) {
        auto &offsets = offsetsPair.second;
        std::sort(offsets.begin(), offsets.end(), [](simtime_t a, simtime_t b) { return a < b; });
    }
    for (auto &application : gateSchedulingInput->applications) {
        auto appModule = application->module->getSubmodule("source");
        output->appsInputAndWithStopReq.push_back(appModule);
    }
    monitor->addApplicationsWithStopReqToOutput(output->appsInputAndWithStopReq);

    return output;
}


//##################################### GET ########################################################

std::string ExternalGateScheduleConfigurator::getExpandedNodeName(cModule *module) const
{
    auto fullNameNetworkNode = std::string(module->getFullName());
    auto type = getDeviceType(module);
    auto nameNetworkNode = (type == 2 || type == 3)
                               ? std::string(module->getParentModule()->getName()) + "." + fullNameNetworkNode
                               : fullNameNetworkNode;

    return nameNetworkNode;
}

bool ExternalGateScheduleConfigurator::isDetComLink(cModule *source, cModule *target,
                                                    DetComLinkType &detComLinkType) const
{
    detComLinkType = DetComLinkType::NO_DETCOM_LINK;
    if (strcmp(source->getModuleType()->getName(), "TsnTranslator") ||
            strcmp(target->getModuleType()->getName(), "TsnTranslator"))
        return false;
    if (!source->par("isDstt") && !target->par("isDstt"))
        return false;
    if (source->par("isDstt") && target->par("isDstt")) {
        detComLinkType = DetComLinkType::DSTT_DSTT;
        return true;
    }
    if (source->par("isDstt")) {
        detComLinkType = DetComLinkType::DSTT_NWTT;
        return true;
    }
    detComLinkType = DetComLinkType::NWTT_DSTT;
    return true;
}


short ExternalGateScheduleConfigurator::getDeviceType(cModule *module) const
{
    auto type = DeviceType::UNSPECIFIED;

    if (!strcmp(module->getModuleType()->getName(), "TsnDevice")) {
        type = DeviceType::END_DEVICE;
    }
    if (!strcmp(module->getModuleType()->getName(), "TsnSwitch")) {
        type = DeviceType::TSN_BRIDGE;
    }
    if (!strcmp(module->getModuleType()->getName(), "TsnTranslator")) {
        if (module->par("isDstt")) {
            type = DeviceType::DS_TT;
        }
        else {
            type = DeviceType::NW_TT;
        }
    }
    return (short)type;
}


std::string ExternalGateScheduleConfigurator::getDetComLinkDescription(DetComLinkType type) const
{ // for DetCom links
    switch (type) {
    case DetComLinkType::DSTT_NWTT:
        return "_Uplink";
    case DetComLinkType::NWTT_DSTT:
        return "_Downlink";
    case DetComLinkType::DSTT_DSTT:
        return "_Convolution";
    case DetComLinkType::NO_DETCOM_LINK:
        return "_NoDetComLink";
    default:
        return "_Unknown";
    }
}


ExternalGateScheduleConfigurator::Input::Port *
ExternalGateScheduleConfigurator::getConfigurablePort( // LINK : "[source,target]" as string
    const Input &input, std::string &linkName) const
{
    linkName = linkName.substr(1, linkName.size() - 2);
    std::string source, target;
    parseString(linkName, source, target, ',');

    auto sourceNode = findConfigurableNetworkNode(input,  source);
    auto targetNode = findConfigurableNetworkNode(input, target);

    auto jt = std::find_if(sourceNode->ports.begin(), sourceNode->ports.end(), [&](Input::Port *port) {
        return port->startNode == sourceNode && port->endNode == targetNode;
    });
    if (jt == sourceNode->ports.end())
        throw cRuntimeError("Cannot find port: %s", linkName.c_str());
    auto port = *jt;
    return port;
}

ExternalGateScheduleConfigurator::Input::NetworkNode *
ExternalGateScheduleConfigurator::findConfigurableNetworkNode(const Input &input, std::string source) const
{
    auto it = std::find_if(input.networkNodes.begin(), input.networkNodes.end(),
                              [&](Input::NetworkNode *switch_) { return getExpandedNodeName(switch_->module) == source; });
    if (it == input.networkNodes.end())
       throw cRuntimeError("Cannot find TSN device: %s", source.c_str());
    auto sourceNode = *it;
    return sourceNode;
}


//############################## ADD ########################################

bool ExternalGateScheduleConfigurator::addEntryToPDBMap(cValueArray *pdb_map, cModule *source, cModule *target) const
{
    DetComLinkType linkType = DetComLinkType::NO_DETCOM_LINK;
    auto nameNetworkNode = getExpandedNodeName(source);
    auto nameNextNetworkNode = getExpandedNodeName(target);
    size_t mapSize = pdb_map->size();

    if (isDetComLink(source, target, linkType)) {
        cValueMap *pdb = new cValueMap();
        cValueMap *link = new cValueMap();
        pdb->set("link", link);
        link->set("currentNodeName", nameNetworkNode);
        link->set("nextNodeName", nameNextNetworkNode);

        std::string key;
        switch (linkType) {
            case DetComLinkType::DSTT_NWTT: {
                key = nameNetworkNode + getDetComLinkDescription(linkType);
                break;
            }
            case DetComLinkType::NWTT_DSTT: {
                key = nameNextNetworkNode + getDetComLinkDescription(linkType);
                break;
            }
            case DetComLinkType::DSTT_DSTT: {
                std::string egress = (nameNextNetworkNode.find('.') != std::string::npos)
                                         ? nameNextNetworkNode.substr(nameNextNetworkNode.find('.') + 1)
                                         : nameNextNetworkNode;
                key = nameNetworkNode + "-" + egress;
                monitor->computeConvolutionAndUpdateDistributions(source, target);
                break;
            }
            case DetComLinkType::NO_DETCOM_LINK: {
                ;
            }
        }
        auto distributions = monitor->getDistributions();
        if (distributions->find(key) != distributions->end()) {
            pdb_map->add(pdb);
            pdb->set("key", key);
        }
        if (mapSize == pdb_map->size()) {
            delete pdb;
        }
        return true;
    } // endif isDetComLink

    return false;
}


void ExternalGateScheduleConfigurator::addFlows(Input &input) const
{
    EV_DEBUG << "Computing flows from configuration" << EV_FIELD(configuration) << EV_ENDL;
    for (int k = 0; k < configuration->size(); k++) {
        auto entry = check_and_cast<cValueMap *>(configuration->get(k).objectValue());

        cModule *source = getModuleByPath(entry->get("source").stringValue());
        auto sourceNode = (Node *)topology->getNodeFor(source);
        auto startDevice = input.getDevice(source);

        cModule *destination = getModuleByPath(entry->get("destination").stringValue());
        auto destinationNode = (Node *)topology->getNodeFor(destination);
        auto endDevice = input.getDevice(destination);

        // ADD Application
        auto startApplication = new Application();
        auto startApplicationModule = startDevice->module->getModuleByPath(
            (std::string(".") + std::string(entry->get("application").stringValue())).c_str());
        if (startApplicationModule == nullptr)
            throw cRuntimeError("Cannot find flow start application, path = %s",
                                entry->get("application").stringValue());

        startApplication->module = startApplicationModule;
        startApplication->device = startDevice;
        startApplication->pcp = entry->get("pcp").intValue();
        startApplication->packetLength = b(entry->get("packetLength").doubleValueInUnit("b") + 464); // add Header +58B
        startApplication->packetInterval = entry->get("packetInterval").doubleValueInUnit("s");
        startApplication->maxLatency = entry->get("maxLatency").doubleValueInUnit("s");
        startApplication->maxJitter = entry->get("maxJitter").doubleValueInUnit("s");
        auto phase = entry->get("phase").doubleValueInUnit("s");
        startApplication->phase = (phase <= 0.0) ? 0.0 : phase;
        startApplication->reliability = entry->get("reliability").doubleValue();
        startApplication->customParams = check_and_cast<cValueMap *>(entry->get("customParams").objectValue());
        input.applications.push_back(startApplication);
        // ADD Flow
        auto flow = new Input::Flow();
        if (!entry->containsKey("name"))
            throw cRuntimeError("Cannot identify the flow!");
        flow->name = entry->get("name").stringValue();
        flow->gateIndex = entry->get("gateIndex").intValue();
        flow->startApplication = startApplication;
        flow->endDevice = endDevice;
        // ADD Route
        cValueArray *pathFragments = new cValueArray();
        for (auto node : computeShortestNodePath(sourceNode, destinationNode)) {
            auto nameNode = getExpandedNodeName(node->module);
            pathFragments->add(nameNode);
        }
        auto path = new Input::PathFragment();
        for (int m = 0; m < pathFragments->size(); m++) {
            for (auto networkNode : input.networkNodes) {
                auto name = pathFragments->get(m).stdstringValue();
                auto nameNode = getExpandedNodeName(networkNode->module);
                if (nameNode == name) {
                    if (m != pathFragments->size() - 1) {
                        auto startNode = networkNode;
                        auto endNodeName =
                                pathFragments->get(m + 1).stdstringValue();
                        auto outputPort = *std::find_if(
                                startNode->ports.begin(),
                                startNode->ports.end(),
                                [&](const auto &port) {
                                    auto nameNode = getExpandedNodeName(
                                            port->endNode->module);
                                    return nameNode == endNodeName;
                                });
                        path->outputPorts.push_back(outputPort);
                        path->inputPorts.push_back(outputPort->otherPort);
                    }
                    path->networkNodes.push_back(networkNode);
                    break;
                } // endif
            } // endfor
        } // endfor
        flow->pathFragments.push_back(path);
        delete pathFragments;
        input.flows.push_back(flow);
    } // endfor configuration
}


void ExternalGateScheduleConfigurator::parseString(std::string s,
        std::string &leftString, std::string &rightString, char sep) const {
    std::stringstream ss(s);
    std::getline(ss, leftString, sep);
    std::getline(ss, rightString);
}


//############################ CONFIGURE ##########################################

void ExternalGateScheduleConfigurator::configureGateScheduling()
{
    EV_DEBUG << "CONFIGURATOR: " << EV_FIELD(simTime()) << "configure scheduling" << EV_ENDL;

    auto scheduleMap = gateSchedulingOutput->gateSchedules;
    for (auto it = scheduleMap.begin(); it != scheduleMap.end(); ++it) {
        Input::Port *port = it->first;
        if (!strcmp(port->module->getName(), "tt"))
            continue;
        std::vector<Output::Schedule *> &schedules = it->second;
        auto queue = port->module->findModuleByPath(".macLayer.queue");
        for (Output::Schedule *schedule : schedules) {
            Schedule *newSchedule = (Schedule *)schedule;
            int gateIndex = newSchedule->gateIndex;
            auto gate = dynamic_cast<PeriodicGate *>(queue->getSubmodule("transmissionGate", gateIndex));
            gate->par("initiallyOpen") = newSchedule->open;

            //cPar &offsetPar = gate->par("offset");
            //offsetPar.setValue(cValue(newSchedule->offset.inUnit(SIMTIME_NS), "ns"));
            gate->par("offset") = newSchedule->offset.dbl();
            cPar &durationsPar = gate->par("durations");
            durationsPar.copyIfShared();
            durationsPar.setObjectValue(newSchedule->durations->dup());
        } // endfor
    } // endfor

}


void ExternalGateScheduleConfigurator::configureApplicationOffsets()
{
    auto output = (Output *)gateSchedulingOutput;

    for (auto &it : output->applicationStartTimes) {
        auto startOffset = it.second;
        auto applicationModule = it.first->module;
        auto sourceModule = check_and_cast<DynamicPacketSource *>(applicationModule->getSubmodule("source"));
        const auto &offsets = output->applicationStartTimesArray[it.first];
        EV_DEBUG << "starting at" << EV_FIELD(simTime()) << EV_FIELD(sourceModule->streamName) << EV_ENDL;
        sourceModule->setNewConfiguration(offsets);
    }
    for (auto &application : output->appsInputAndWithStopReq) {
        auto appModule = check_and_cast<DynamicPacketSource *>(application);
        if (appModule->stopIfNotScheduled()) {
            EV_DEBUG << "stopped at" << EV_FIELD(simTime()) << EV_FIELD(appModule->streamName ) << EV_ENDL;
            cValueMap *element = appModule->getConfiguration();
            monitor->updateStreamConfigurations(element);
        } // endif
    } // endfor
}


//############################## CLEAR ###############################################

void ExternalGateScheduleConfigurator::clearConfiguration()
{
    deleteOldConfigurationPar();
    if (topology != nullptr)
        topology->clear();
    if (gateSchedulingInput != nullptr)
        delete gateSchedulingInput;
    gateSchedulingInput = nullptr;

    if (gateSchedulingOutput != nullptr) {
        auto scheduleMap = gateSchedulingOutput->gateSchedules;
        for (auto it = scheduleMap.begin(); it != scheduleMap.end(); ++it) {
            std::vector<Output::Schedule*> &schedules = it->second;
            for (Output::Schedule *schedule : schedules) {
                Schedule *oldSchedule = (Schedule*) schedule;
                oldSchedule->~Schedule();
            } // endfor
        } // endfor
        delete (Output*)gateSchedulingOutput;
    }
    gateSchedulingOutput = nullptr;
}

void ExternalGateScheduleConfigurator::deleteOldConfigurationPar()
{
    if (configuration != nullptr) {
        for (int i = 0; i < configuration->size(); i++) {
            auto entry = configuration->remove(i);
            delete entry.objectValue();
        }
        configuration->clear();
        drop(configuration);
        delete configuration;
        configuration = nullptr;
    }
}

ExternalGateScheduleConfigurator::~ExternalGateScheduleConfigurator()
{
    clearConfiguration();

    if (configuration != nullptr)
        delete configuration;

    delete hashMapNodeId;

    if (configurationComputedEvent != nullptr) {
        if (configurationComputedEvent->isScheduled())
            cancelEvent(configurationComputedEvent);
        drop(configurationComputedEvent);
        delete configurationComputedEvent;
    }
}

} // namespace d6g

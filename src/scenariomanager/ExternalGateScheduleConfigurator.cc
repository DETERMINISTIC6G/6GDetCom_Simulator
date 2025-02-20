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

#include "ChangeMonitor.h"

#include <cstdio>
#include <fstream>
#include <functional> // hash

namespace d6g {

Define_Module(ExternalGateScheduleConfigurator);


void ExternalGateScheduleConfigurator::initialize(int stage) {
    if (stage == INITSTAGE_LOCAL) {
        fileNameStreams = par("fileNameStreams").stdstringValue();
        fileNameNetwork = par("fileNameNetwork").stdstringValue();
        fileNameDistribution = par("fileNameDistribution").stdstringValue();
        outputFileName  = par("outputFileName").stdstringValue();

        configuration = check_and_cast<cValueArray*>(par("configuration").objectValue()); // specified in .ini
        gateCycleDuration = par("gateCycleDuration");

        hashMap = new std::map<std::string, uint16_t>();
    } else if (stage == INITSTAGE_GATE_SCHEDULE_CONFIGURATION) {
        ChangeMonitor *monitor = nullptr;
        if (!configuration->size() && (monitor = check_and_cast<ChangeMonitor*>(getModuleByPath("monitor")))) { // query monitor if it exists
            distribution = monitor->getDistributions();
            par("configuration").setObjectValue(monitor->getStreamConfigurations());
            return;
        }
        computeConfiguration();
        // configureGateScheduling();
        // configureApplicationOffsets();
    }
}

/*void ExternalGateScheduleConfigurator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}*/

void ExternalGateScheduleConfigurator::handleParameterChange(const char *name) {
    if (!strcmp(name, "configuration")) {
        configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
        clearConfiguration();
        computeConfiguration();
        //configureGateScheduling();
        //configureApplicationOffsets();
    }
    if (!strcmp(name, "gateCycleDuration")) {
        gateCycleDuration = par("gateCycleDuration");
    }
}


void ExternalGateScheduleConfigurator::printJson(std::ostream& stream, const cValue& value, int level) const { // not static
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
                    auto newSecond =  originalSecond.isNumeric() && std::isinf(originalSecond.doubleValue()) ?
                            (originalSecond.doubleValue() > 0 ? cValue(1e308) : cValue(-1e308))  : originalSecond;
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


void ExternalGateScheduleConfigurator::executeTSNsched(std::string inputFileName) const
{
    /*FILE* pipe = popen("python3 process_data.py", "w");
    if (pipe) {
       // fprintf(pipe, "%s", j.dump().c_str());
        pclose(pipe);
    }*/
    int result = std::system("python3 scripts/dummy_scheduler.py");
    if (result != 0)
        throw cRuntimeError("Command execution failed");
}

ExternalGateScheduleConfigurator::Output *ExternalGateScheduleConfigurator::computeGateScheduling(const Input& input) const {
    std::string baseName = "test";

   // std::string outputFileName = "tsn_configuration.json";
    std::remove(outputFileName.c_str());

    writeStreamsToFile(input);
    writeNetworkToFile(input);
    writeDistributionsToFile();

    executeTSNsched(baseName);
    return new Output();//readOutputFromFile(input, outputFileName);
}


void ExternalGateScheduleConfigurator::writeDistributionsToFile() const {
    cValueMap *json = new cValueMap();
    cValueArray *jsonDistributions = new cValueArray();
    json->set("distributions", jsonDistributions);
    if (distribution != nullptr) {
        for (const auto &pair : *distribution) {
            const char *bridgeName = pair.first.c_str();
            cValueArray *histogram = pair.second;
            cValueMap *entry = new cValueMap();
            entry->set("name", bridgeName);
            entry->set("data", histogram);
            entry->set("type", histogram->getName());
            jsonDistributions->add(cValue(entry));
        }
    }
    this->write(fileNameDistribution, json);
    delete json;
}

void ExternalGateScheduleConfigurator::writeStreamsToFile(const Input& input) const {
    auto jsonStreams = convertInputToJsonStreams(input);
    this->write(fileNameStreams, jsonStreams);
    delete jsonStreams;
}


void ExternalGateScheduleConfigurator::writeNetworkToFile(const Input& input) const {
    auto jsonNetwork = convertInputToJsonNetwork(input);
    cValueArray *jsonIDs = new cValueArray();
    jsonNetwork->set("id_mapping", jsonIDs);
    for (const auto& pair : *hashMap) {
        cValueMap *json = new cValueMap();
        json->set("node", pair.first);
        json->set("id", pair.second);
        jsonIDs->add(json);
    }
    this->write(fileNameNetwork, jsonNetwork);
    delete jsonNetwork;
}


void ExternalGateScheduleConfigurator::write(std::string fileName, cValueMap *json) const {
    std::ofstream stream;
    stream.open(fileName.c_str());
    if (stream.fail())
        throw cRuntimeError("Cannot open file %s", fileName.c_str());
    this->printJson(stream, cValue(json));
}


cValueMap *ExternalGateScheduleConfigurator::convertInputToJsonStreams(const Input& input) const {
    cValueMap *json = new cValueMap();
    cValueArray *jsonFlows = new cValueArray();
    json->set("flows", jsonFlows);

    for (auto flow : input.flows) {
        cValueMap *jsonFlow = new cValueMap();
        jsonFlows->add(jsonFlow);
        jsonFlow->set("name", flow->name);
        jsonFlow->set("type", "unicast");
        jsonFlow->set("source_device", flow->startApplication->device->module->getFullName());
       // jsonFlow->set("fixed_priority", "true");
        jsonFlow->set("pcp", flow->gateIndex);
        jsonFlow->set("packet_periodicity", flow->startApplication->packetInterval.dbl() * 1000000);
        jsonFlow->set("packet_periodicity_unit", "us");
        jsonFlow->set("packet_size", b(flow->startApplication->packetLength).get());
        jsonFlow->set("packet_size_unit", "bit");

        jsonFlow->set("hard_constraint_time_latency", flow->startApplication->maxLatency.dbl() * 1000000);
        jsonFlow->set("hard_constraint_time_jitter", flow->startApplication->maxJitter.dbl() * 1000000);
        jsonFlow->set("hard_constraint_time_unit", "us");

        cValueArray *endDevices = new cValueArray();
        jsonFlow->set("end_devices", endDevices);
        endDevices->add(cValue(flow->endDevice->module->getFullName()));

        cValueArray *pdb_map = new cValueArray();
        jsonFlow->set("pdb_map", pdb_map);

        cValueArray *hops = new cValueArray();
        jsonFlow->set("route", hops);
        for (int j = 0; j < flow->pathFragments.size(); j++) {
            auto pathFragment = flow->pathFragments[j];
            for (int k = 0; k < pathFragment->networkNodes.size() - 1; k++) {
                auto networkNode = pathFragment->networkNodes[k];
                auto nextNetworkNode = pathFragment->networkNodes[k + 1];
                cValueMap *hop = new cValueMap();
                hops->add(hop);
                auto nameNetworkNode = expandNodeName(networkNode->module);
                hop->set("currentNodeName", nameNetworkNode);
                auto nameNextNetworkNode = expandNodeName(nextNetworkNode->module);
                hop->set("nextNodeName", nameNextNetworkNode);
                //pdb map: find a distribution
                DetComLinkType linkType = DetComLinkType::NO_DETCOM_LINK;
                if (isDetComLink(networkNode->module, nextNetworkNode->module, linkType)) {
                    addEntryToPDBMap(pdb_map, linkType, nameNetworkNode, nameNextNetworkNode);
                }
            } // 3. for
        }//2. for
        setReliabilityAndPolicyToPDBMapEntry(pdb_map, flow->name);
        //###############################
        //temporary hard coded
        jsonFlow->set("weight", 1.0);
        jsonFlow->set("phase", 0);
        jsonFlow->set("objective_type", 3);
        jsonFlow->set("packet_loss", 0);
        //################################
    } //1.for
    return json;
}


std::string ExternalGateScheduleConfigurator::expandNodeName(cModule *module) const {
    auto fullNameNetworkNode = std::string(module->getFullName());
    //auto nameNetworkNode = (fullNameNetworkNode.find("nwtt") != std::string::npos || fullNameNetworkNode.find("dstt") != std::string::npos) ?
                   // std::string(module->getParentModule()->getName()) + "." + std::string(module->getFullName()) : std::string(module->getFullName());
    auto type = getSwitchType(module);
    auto nameNetworkNode = (1 < type && type < 4) ? std::string(module->getParentModule()->getName()) + "." + std::string(module->getFullName()) : std::string(module->getFullName());
    return nameNetworkNode;
}


void ExternalGateScheduleConfigurator::setReliabilityAndPolicyToPDBMapEntry(cValueArray *pdb_map, std::string name) const {
    double reliability;
    double policy;
    int pdbSize = pdb_map->size();
    for (int k = 0; k < configuration->size(); k++) {
        auto entry = check_and_cast<cValueMap *>(configuration->get(k).objectValue());
        if (entry->get("name").stdstringValue() == name) {
            reliability = std::pow(entry->get("reliability").doubleValue(), 1.0/pdbSize); // #DetCom-link-th root of the reliability parameter ;
            policy = entry->get("policy").intValue();
            break;
        }
    }
    for (int i = 0; i < pdbSize; i++) {
         auto vMap = check_and_cast<cValueMap *>(pdb_map->get(i).objectValue());
         //double reliability = std::pow(getReliability(flow->name), 1.0/pdbSize); // #DetCom-link-th root of the reliability parameter
         vMap->set("reliability", reliability);
         vMap->set("policy", policy);
    }
}


bool ExternalGateScheduleConfigurator::isDetComLink(cModule *source, cModule *target, DetComLinkType &detComLinkType) const {
    const char *translator = "d6g.devices.tsntranslator.TsnTranslator";
    detComLinkType = DetComLinkType::NO_DETCOM_LINK;
    if (strcmp(source->getNedTypeName(), translator) || (strcmp(target->getNedTypeName(), translator))) return false;
    if (!source->par("isDstt") && !target->par("isDstt")) return false;

    if (source->par("isDstt") && target->par("isDstt")) {detComLinkType = DetComLinkType::DSTT_DSTT; return true;}
    if (source->par("isDstt")) {detComLinkType = DetComLinkType::DSTT_NWTT; return true; }
    detComLinkType = DetComLinkType::NWTT_DSTT;
    return true;
}

void ExternalGateScheduleConfigurator::addEntryToPDBMap(cValueArray *pdb_map, DetComLinkType linkType, std::string nameNetworkNode, std::string nameNextNetworkNode) const {
    if (linkType == DetComLinkType::DSTT_DSTT) {
        ;
        //todo
    }else {
        std::string key = nameNetworkNode + getDescription(linkType);
        for (int i = 0; i<2; i++) {
            if (distribution->find(key) != distribution->end()) {
                    cValueMap *pdb = new cValueMap();
                    pdb_map->add(pdb);
                    pdb->set("histogram", key);
                    cValueMap *link = new cValueMap();
                    pdb->set("link", link);
                    link->set("currentNodeName", nameNetworkNode);
                    link->set("nextNodeName", nameNextNetworkNode);
                    break;
                }//endif
                key = nameNextNetworkNode + getDescription(linkType);
        }//endfor
    }
}


short ExternalGateScheduleConfigurator::getSwitchType(cModule* module) const {
    auto type = DeviceType::UNSPECIFIED;
    if (!strcmp(module->getModuleType()->getName(), "TsnSwitch")) {
        type = DeviceType::TSN_BRIDGE;
    }
    if (!strcmp(module->getModuleType()->getName(), "TsnTranslator")) {
        if (module->par("isDstt")) {type = DeviceType::DS_TT;}
        else {type = DeviceType::NW_TT;}
    }
    return (short)type;
}

std::string ExternalGateScheduleConfigurator::getDescription(DetComLinkType type) const { // for DetCom links
    switch (type) {
    case DetComLinkType::DSTT_NWTT:
        return "_Uplink";
    case DetComLinkType::NWTT_DSTT:
        return "_Downlink";
    case DetComLinkType::DSTT_DSTT:
        return "_Uplink_Downlink";
    case DetComLinkType::NO_DETCOM_LINK:
        return "_NoDetComLink";
    default:
        return "_Unknown";
    }
}


cValueMap *ExternalGateScheduleConfigurator::convertInputToJsonNetwork(const Input& input) const
{
    std::vector<std::string> deviceList;

    cValueMap *json = new cValueMap();
    cValueArray *jsonDevices = new cValueArray();
    json->set("end_devices", jsonDevices);
    for (auto device : input.devices) {
        cValueMap *jsonDevice = new cValueMap();
        jsonDevices->add(jsonDevice);
        jsonDevice->set("name", device->module->getFullName());
        jsonDevice->set("device_type", (short)DeviceType::END_DEVICE);
        jsonDevice->set("processing_delay", 0);
        jsonDevice->set("processing_delay_unit", "us");

        deviceList.push_back(std::string(device->module->getFullName()));
    }
    cValueArray *jsonSwitches = new cValueArray();
    json->set("switches", jsonSwitches);
    for (int i = 0; i < input.switches.size(); i++) {
        auto switch_ = input.switches[i];
        cValueMap *jsonSwitch = new cValueMap();
        jsonSwitches->add(jsonSwitch);
        // TODO KLUDGE this is a wild guess
        double guardBand = computeGuardBand(input);

        auto nameNetworkNode = expandNodeName(switch_->module);
        jsonSwitch->set("name", nameNetworkNode);

        deviceList.push_back(nameNetworkNode);

        auto jsonPorts = new cValueArray();
        jsonSwitch->set("ports", jsonPorts);
        for (int j = 0; j < switch_->ports.size(); j++) {
            auto port = switch_->ports[j];
            auto jsonPort = new cValueMap();
            jsonPorts->add(jsonPort);
            // KLUDGE: port name should not be unique in the network but only in the network node
            auto nameNode = expandNodeName(port->startNode->module);
            jsonPort->set("port_name", nameNode + "-" + port->module->getFullName());
            auto connectedToFullName = expandNodeName(port->endNode->module);
            jsonPort->set("connects_to", connectedToFullName);

            DetComLinkType detComLinkType;
            short link_type = isDetComLink(port->startNode->module, port->endNode->module, detComLinkType);
            jsonPort->set("detCom_link_descr", getDescription(detComLinkType));
            jsonPort->set("link_type", link_type);
            if (link_type)  jsonPort->set("multiple_subcarriers", true);

            jsonSwitch->set("processing_delay", 0);
            jsonSwitch->set("processing_delay_unit", "us");
            jsonSwitch->set("device_type", getSwitchType(port->startNode->module));

            jsonPort->set("propagation_delay", port->propagationTime.dbl() * 1000000); // timeToTravel
            jsonPort->set("propagation_delay_unit", "us"); // timeToTravelUnit
//            jsonPort->set("guardBandSize", guardBand);
//            jsonPort->set("guardBandSizeUnit", "bit");
            jsonPort->set("data_rate", bps(port->datarate).get() / 1000000); // portSpeed
            jsonPort->set("data_rate_size_unit", "bit"); // portSpeedSizeUnit
            jsonPort->set("data_rate_time_unit", "us"); // portSpeedTimeUnit
            jsonPort->set("schedule_type", "Hypercycle");
            //jsonPort->set("cycle_start", 0);
            //jsonPort->set("cycle_start_unit", "us");
           // jsonPort->set("max_slot_duration", gateCycleDuration.dbl() * 1000000); //hypercycle
           // jsonPort->set("max_slot_duration_unit", "us");
        }
    }
    generateNetworkNodesHashMap(deviceList);
    return json;
}


double ExternalGateScheduleConfigurator::computeGuardBand(const Input &input) const {
    static double guardBand = 0;
    for (auto flow : input.flows) {
        double v = b(flow->startApplication->packetLength).get();
        if (guardBand < v)  guardBand = v;
    }
    return guardBand;
}


void ExternalGateScheduleConfigurator::addFlows(Input& input) const {
    int flowIndex = 0;
    EV_DEBUG << "Computing flows from configuration" << EV_FIELD(configuration) << EV_ENDL;
    for (int k = 0; k < configuration->size(); k++) {
        auto entry = check_and_cast<cValueMap *>(configuration->get(k).objectValue());

        cModule *source = getModuleByPath(entry->get("source").stringValue());
        auto sourceNode = (Node *) topology->getNodeFor(source);

        cModule *destination = getModuleByPath(entry->get("destination").stringValue());
        auto destinationNode = (Node *) topology->getNodeFor(destination);

        int pcp = entry->get("pcp").intValue();
        int gateIndex = entry->get("gateIndex").intValue();
        b packetLength = b(entry->get("packetLength").doubleValueInUnit("b"));
        b cutthroughSwitchingHeaderSize = entry->containsKey("cutthroughSwitchingHeaderSize") ? b(entry->get("cutthroughSwitchingHeaderSize").doubleValueInUnit("b")) : b(0);
        simtime_t packetInterval = entry->get("packetInterval").doubleValueInUnit("s");
        simtime_t maxLatency = entry->containsKey("maxLatency") ? entry->get("maxLatency").doubleValueInUnit("s") : -1;
        simtime_t maxJitter = entry->containsKey("maxJitter") ? entry->get("maxJitter").doubleValueInUnit("s") : 0;
        bps datarate = packetLength / s(packetInterval.dbl());
        auto startDevice = input.getDevice(source);
        auto endDevice = input.getDevice(destination);
        auto startApplication = new Input::Application();
        auto startApplicationModule = startDevice->module->getModuleByPath((std::string(".") + std::string(entry->get("application").stringValue())).c_str());
        if (startApplicationModule == nullptr)
            throw cRuntimeError("Cannot find flow start application, path = %s", entry->get("application").stringValue());
        startApplication->module = startApplicationModule;
        startApplication->device = startDevice;
        startApplication->pcp = pcp;
        startApplication->packetLength = packetLength;
        startApplication->packetInterval = packetInterval;
        startApplication->maxLatency = maxLatency;
        startApplication->maxJitter = maxJitter;
        input.applications.push_back(startApplication);
        EV_DEBUG << "Adding flow from configuration" << EV_FIELD(source) << EV_FIELD(destination) << EV_FIELD(pcp) << EV_FIELD(packetLength) << EV_FIELD(packetInterval, packetInterval.ustr()) << EV_FIELD(datarate) << EV_FIELD(maxLatency, maxLatency.ustr()) << EV_FIELD(maxJitter, maxJitter.ustr()) << EV_ENDL;
        auto flow = new Input::Flow();
        flow->name = entry->containsKey("name") ? entry->get("name").stringValue() : (std::string("flow") + std::to_string(flowIndex++)).c_str();
        flow->gateIndex = gateIndex;
        flow->cutthroughSwitchingHeaderSize = cutthroughSwitchingHeaderSize;
        flow->startApplication = startApplication;
        flow->endDevice = endDevice;

        cValueArray *pathFragments;
        if (entry->containsKey("pathFragments"))
            pathFragments = check_and_cast<cValueArray*>(entry->get("pathFragments").objectValue());
        else {
            auto pathFragment = new cValueArray();
            for (auto node : computeShortestNodePath(sourceNode, destinationNode)) {
                auto nameNode = expandNodeName(node->module);
                pathFragment->add(nameNode);
            }
            pathFragments = new cValueArray();
            pathFragments->add(pathFragment);
        }
        for (int l = 0; l < pathFragments->size(); l++) {
            auto path = new Input::PathFragment();
            auto pathFragment = check_and_cast<cValueArray*>(pathFragments->get(l).objectValue());
            for (int m = 0; m < pathFragment->size(); m++) {
                for (auto networkNode : input.networkNodes) {
                    auto name = pathFragment->get(m).stdstringValue();
                    auto nameNode = expandNodeName(networkNode->module);
                    if (nameNode == name) {
                        if (m != pathFragment->size() - 1) {
                            auto startNode = networkNode;
                            auto endNodeName = pathFragment->get(m + 1).stdstringValue();
                            auto outputPort = *std::find_if(startNode->ports.begin(), startNode->ports.end(), [&](const auto &port) {
                                        auto nameNode = expandNodeName(port->endNode->module);
                                        return nameNode == endNodeName;
                                    });
                            path->outputPorts.push_back(outputPort);
                            path->inputPorts.push_back(outputPort->otherPort);
                        }
                        path->networkNodes.push_back(networkNode);
                        break;
                    } //endif
                } //endfor
            } //endfor
            flow->pathFragments.push_back(path);
        }//endfor
        if (!entry->containsKey("pathFragments"))
            delete pathFragments;
        input.flows.push_back(flow);
    }
    std::sort(input.flows.begin(), input.flows.end(), [] (const Input::Flow *r1, const Input::Flow *r2) {
        return r1->startApplication->pcp > r2->startApplication->pcp;
    });
}

void ExternalGateScheduleConfigurator::generateNetworkNodesHashMap(const std::vector<std::string>& strings) const {
    std::hash<std::string> hasher;
    for (const auto& str : strings) {
        size_t value = hasher(str);
        uint16_t short_hash = static_cast<uint16_t>(value);
        (*hashMap)[str] = short_hash;
    }
}


ExternalGateScheduleConfigurator::~ExternalGateScheduleConfigurator(){
    //delete configuration;
   // delete distribution;
    delete hashMap;
}

} //namespace

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

namespace d6g {

Define_Module(ExternalGateScheduleConfigurator);




void ExternalGateScheduleConfigurator::initialize(int stage) {
    if (stage == INITSTAGE_LOCAL) {
        gateCycleDuration = par("gateCycleDuration");
        configuration = check_and_cast<cValueArray*>(
                par("configuration").objectValue());

        // distribution = check_and_cast<cValueArray *>(par("distribution").objectValue());
    } else if (stage == INITSTAGE_GATE_SCHEDULE_CONFIGURATION) {

        ChangeMonitor *monitor = nullptr;;
        if (!configuration->size() && (monitor = check_and_cast<ChangeMonitor*>(
                getModuleByPath("monitor")))) {

           /* auto monitor = check_and_cast<ChangeMonitor*>(
                    getModuleByPath("monitor"));*/
            //configuration = monitor->getStreamConfigurations(); //destructor
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

void ExternalGateScheduleConfigurator::handleParameterChange(const char *name)
{
    if (!strcmp(name, "configuration")) {
        configuration = check_and_cast<cValueArray *>(par("configuration").objectValue());
        clearConfiguration();
        computeConfiguration();
        //configureGateScheduling();
        //configureApplicationOffsets();
    }
    if (!strcmp(name, "distribution")) {
       // distribution = check_and_cast<cValueArray *>(par("distribution").objectValue());

    }
}


void ExternalGateScheduleConfigurator::executeTSNsched(std::string inputFileName) const
{
    int result = std::system("python3 scripts/dummy_scheduler.py");
}

ExternalGateScheduleConfigurator::Output *ExternalGateScheduleConfigurator::computeGateScheduling(const Input& input) const
{
    std::string baseName = "test";
    std::string inputFileName = baseName + "streams.json";
    // TODO: std::string outputFileName = baseName + "-TSNsched-output.json";
    std::string outputFileName = "output.json";
    //std::remove(outputFileName.c_str());


    writeInputToFile(input, "streams.json", "network.json");
    writeDistributionsToFile("histograms.json");


    executeTSNsched(inputFileName);


    return new Output();//readOutputFromFile(input, outputFileName);
}


void ExternalGateScheduleConfigurator::writeDistributionsToFile(std::string fileName) const
{
    cValueMap *json = new cValueMap();
    cValueArray *jsonDistributions = new cValueArray();

    //auto monitor = check_and_cast<ChangeMonitor*>(getModuleByPath("monitor"));
    std::map<std::string, cValueArray*> *distr = distribution; //monitor->getDistributions();

    json->set("distributions", jsonDistributions);
    if (distr != nullptr) {
        for (const auto &pair : *distr) {
            const char *bridgeName = pair.first.c_str();
            cValueArray *histogram = pair.second;

            cValueMap *tmp = new cValueMap();
            tmp->set("name", bridgeName);
            tmp->set("data", histogram);

            jsonDistributions->add(cValue(tmp));
        }
    }
    this->write(fileName, json);

    //delete jsonDistributions;
    delete json;
}


void ExternalGateScheduleConfigurator::writeInputToFile(const Input& input, std::string fileNameStreams, std::string fileNameNetwork) const
{
    auto jsonStreams = convertInputToJsonStreams(input);
    this->write(fileNameStreams, jsonStreams);

    auto jsonNetwork = convertInputToJsonNetwork(input);
    this->write(fileNameNetwork, jsonNetwork);

    delete jsonStreams;
    delete jsonNetwork;
}

void ExternalGateScheduleConfigurator::write(std::string fileName, cValueMap *json) const
{
    std::ofstream stream;
    stream.open(fileName.c_str());
    if (stream.fail())
        throw cRuntimeError("Cannot open file %s", fileName.c_str());
    this->printJson(stream, cValue(json));
}


void ExternalGateScheduleConfigurator::printJson(std::ostream& stream, const cValue& value, int level) const
{
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
                    auto value = it->second;

                    auto sec2 =  value.isNumeric() && std::isinf(value.doubleValue()) ?
                            (value.doubleValue() > 0 ? cValue(1e308) : cValue(-1e308))  : value;
                    printJson(stream, sec2, level + 1);
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


cValueMap *ExternalGateScheduleConfigurator::convertInputToJsonStreams(const Input& input) const
{
    cValueMap *json = new cValueMap();

    cValueArray *jsonFlows = new cValueArray();
    json->set("flows", jsonFlows);
    for (auto flow : input.flows) {
        cValueMap *jsonFlow = new cValueMap();
        jsonFlows->add(jsonFlow);
        jsonFlow->set("name", flow->name);
        jsonFlow->set("type", "unicast");
        jsonFlow->set("source_device", flow->startApplication->device->module->getFullName());
        jsonFlow->set("fixed_priority", "true");
        jsonFlow->set("pcp", flow->gateIndex);
        jsonFlow->set("packet_periodicity", flow->startApplication->packetInterval.dbl() * 1000000);
        jsonFlow->set("packet_periodicity_unit", "us");
        jsonFlow->set("packet_size", b(flow->startApplication->packetLength).get());
        jsonFlow->set("packet_size_unit", "bit");
        jsonFlow->set("packet_loss", 0);
        jsonFlow->set("hard_constraint_time_latency", flow->startApplication->maxLatency.dbl() * 1000000);
        jsonFlow->set("hard_constraint_time_jitter", flow->startApplication->maxJitter.dbl() * 1000000);
        jsonFlow->set("hard_constraint_time_unit", "us");

        // ###################
        /*"pdb_map": [
            {
                "link": [
                    0,
                    15
                ],
                "reliability": 0.9999,
                "policy": 0,
                "histogram": "../data/histograms/downlink_histogram.json"
            }
        ],*/
        cValueArray *pdb_map = new cValueArray();

        jsonFlow->set("pdb_map", pdb_map);



        // ###################
        jsonFlow->set("weight", 1.0);
        jsonFlow->set("phase", 0);
        jsonFlow->set("objective_type", 3);

        cValueArray *endDevices = new cValueArray();
        jsonFlow->set("end_devices", endDevices);
        endDevices->add(cValue(flow->endDevice->module->getFullName()));

        cValueArray *hops = new cValueArray();
        jsonFlow->set("route", hops);
        for (int j = 0; j < flow->pathFragments.size(); j++) {
            auto pathFragment = flow->pathFragments[j];
            for (int k = 0; k < pathFragment->networkNodes.size() - 1; k++) {
                auto networkNode = pathFragment->networkNodes[k];
                auto nextNetworkNode = pathFragment->networkNodes[k + 1];
                cValueMap *hop = new cValueMap();
                hops->add(hop);

                auto fullNameNetworkNode = std::string(networkNode->module->getFullName());
                auto nameNetworkNode = (fullNameNetworkNode.find("nwtt") != std::string::npos || fullNameNetworkNode.find("dstt") != std::string::npos) ?
                        std::string( networkNode->module->getParentModule()->getName()) + "."
                        + std::string(networkNode->module->getFullName())
                        : std::string(networkNode->module->getFullName());

                hop->set("currentNodeName", nameNetworkNode);

                auto fullNextNameNetworkNode = std::string(nextNetworkNode->module->getFullName());
                auto nameNextNetworkNode = (fullNextNameNetworkNode.find("nwtt") != std::string::npos || fullNextNameNetworkNode.find("dstt") != std::string::npos) ?
                        std::string(nextNetworkNode->module->getParentModule()->getName()) + "."
                        + std::string(nextNetworkNode->module->getFullName())
                        : std::string(nextNetworkNode->module->getFullName());

                hop->set("nextNodeName", nameNextNetworkNode);

                //pdb map: find a distribution
                    std::string hist;
                   /* if (distribution->find(std::string(nameNetworkNode) + "_Downlink") != distribution->end()
                          && isDetComLink(networkNode->module, nextNetworkNode->module)) {
                        hist = std::string(nameNetworkNode)+"_Downlink";
                    }

                    else if (distribution->find(std::string(nameNextNetworkNode) + "_Uplink") != distribution->end()
                            && isDetComLink(nextNetworkNode->module, networkNode->module))
                    {
                        hist = std::string(nameNextNetworkNode) + "_Uplink";
                    }else continue;*/

                    if (hasDistribution(std::string(nameNetworkNode) + "_Downlink", hist) && isDetComLink(networkNode->module, nextNetworkNode->module)
                            || hasDistribution(std::string(nameNextNetworkNode) + "_Uplink", hist) && isDetComLink(nextNetworkNode->module, networkNode->module)) {


                    cValueMap *pdb = new cValueMap();
                    pdb_map->add(pdb);

                    pdb->set("reliability", 0.9999);
                    pdb->set("policy", 0);

                    pdb->set("histogram", hist);

                    cValueMap *link = new cValueMap();
                    pdb->set("link", link);
                    link->set("currentNodeName", nameNetworkNode);
                    link->set("nextNodeName", nameNextNetworkNode);
                }


            }
        }
    }
    return json;
}

bool ExternalGateScheduleConfigurator::isDetComLink(cModule *source, cModule *target) const {
    const char *translator = "d6g.devices.tsntranslator.TsnTranslator";
    if (!strcmp(source->getNedTypeName(), translator)
            && strcmp(target->getNedTypeName(), translator)) {

        TsnTranslator *translatorModule = dynamic_cast<TsnTranslator*>(source);
        if (translatorModule->par("isDstt")) return true;
    }
    return false;
}

bool ExternalGateScheduleConfigurator::hasDistribution(std::string key, std::string &hist) const {
    if (distribution->find(key) != distribution->end()) {
        hist = key;
        return true;
    }
    return false;
}


cValueMap *ExternalGateScheduleConfigurator::convertInputToJsonNetwork(const Input& input) const
{
    cValueMap *json = new cValueMap();
    cValueArray *jsonDevices = new cValueArray();
    json->set("devices", jsonDevices);
    for (auto device : input.devices) {
        cValueMap *jsonDevice = new cValueMap();
        jsonDevices->add(jsonDevice);
        jsonDevice->set("name", device->module->getFullName());
    }
    cValueArray *jsonSwitches = new cValueArray();
    json->set("switches", jsonSwitches);
    for (int i = 0; i < input.switches.size(); i++) {
        auto switch_ = input.switches[i];
        cValueMap *jsonSwitch = new cValueMap();
        jsonSwitches->add(jsonSwitch);
        // TODO KLUDGE this is a wild guess
        double guardBand = 0;
        for (auto flow : input.flows) {
            double v = b(flow->startApplication->packetLength).get();
            if (guardBand < v)
                guardBand = v;
        }
        auto jsonPorts = new cValueArray();

        //#####
        auto fullNameNetworkNode = std::string(switch_->module->getFullName());
        auto nameNetworkNode = (fullNameNetworkNode.find("nwtt") != std::string::npos || fullNameNetworkNode.find("dstt") != std::string::npos) ?
               std::string( switch_->module->getParentModule()->getName()) + "."
               + std::string(switch_->module->getFullName())
               : std::string(switch_->module->getFullName());
        //#####

        jsonSwitch->set("name", nameNetworkNode);
        jsonSwitch->set("processing_delay", 0);
        jsonSwitch->set("processing_delay_unit", "us");
        jsonSwitch->set("type", 1);

        jsonSwitch->set("ports", jsonPorts);
        for (int j = 0; j < switch_->ports.size(); j++) {
            auto port = switch_->ports[j];
            auto jsonPort = new cValueMap();
            jsonPorts->add(jsonPort);
            // KLUDGE: port name should not be unique in the network but only in the network node

            //##########
            auto fullNameNode = std::string(port->startNode->module->getFullName());
            auto nameNode = (fullNameNode.find("nwtt") != std::string::npos || fullNameNode.find("dstt") != std::string::npos) ?
                       std::string(port->startNode->module->getParentModule()->getName()) + "."
                       + std::string(port->startNode->module->getFullName())
                       : std::string(port->startNode->module->getFullName());

            //############

           // std::string nodeName = port->startNode->module->getFullName();
            jsonPort->set("port_name", nameNode + "-" + port->module->getFullName());

            //############
            auto connectedToName = std::string(port->endNode->module->getFullName());
            auto connectedToFullName = (connectedToName.find("nwtt") != std::string::npos || connectedToName.find("dstt") != std::string::npos) ?
                           std::string(port->endNode->module->getParentModule()->getName()) + "."
                           + std::string(port->endNode->module->getFullName())
                           : std::string(port->endNode->module->getFullName());

            //############

            jsonPort->set("connects_to", connectedToFullName);
            jsonPort->set("propagation_delay", port->propagationTime.dbl() * 1000000); // timeToTravel
            jsonPort->set("propagation_delay_unit", "us"); // timeToTravelUnit
//            jsonPort->set("guardBandSize", guardBand);
//            jsonPort->set("guardBandSizeUnit", "bit");
            jsonPort->set("data_rate", bps(port->datarate).get() / 1000000); // portSpeed
            jsonPort->set("data_rate_size_unit", "bit"); // portSpeedSizeUnit
            jsonPort->set("data_rate_time_unit", "us"); // portSpeedTimeUnit
            jsonPort->set("schedule_type", "Hypercycle");
            jsonPort->set("cycle_start", 0);
            jsonPort->set("cycle_start_unit", "us");
            jsonPort->set("max_slot_duration", gateCycleDuration.dbl() * 1000000); //hypercycle
            jsonPort->set("max_slot_duration_unit", "us");

        }
    }

    return json;
}



void ExternalGateScheduleConfigurator::addFlows(Input& input) const
{
    int flowIndex = 0;
    EV_DEBUG << "Computing flows from configuration" << EV_FIELD(configuration) << EV_ENDL;
    for (int k = 0; k < configuration->size(); k++) {
        auto entry = check_and_cast<cValueMap *>(configuration->get(k).objectValue());
     //   for (int i = 0; i < topology->getNumNodes(); i++) {
            cModule *source = getModuleByPath(entry->get("source").stringValue()); //sourceNode->module;
            auto sourceNode = (Node *) topology->getNodeFor(source); // (Node *)topology->getNode(i);




        //    for (int j = 0; j < topology->getNumNodes(); j++) {
                cModule *destination = getModuleByPath(entry->get("destination").stringValue());// destinationNode->module;
                auto destinationNode = (Node *) topology->getNodeFor(destination);; // (Node *)topology->getNode(j);

              //  PatternMatcher sourceMatcher(entry->get("source").stringValue(), true, false, false);
              //  PatternMatcher destinationMatcher(entry->get("destination").stringValue(), true, false, false);
               // if (sourceMatcher.matches(sourceNode->module->getFullPath().c_str()) &&
                //    destinationMatcher.matches(destinationNode->module->getFullPath().c_str()))
              //  {
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
                        pathFragments = check_and_cast<cValueArray *>(entry->get("pathFragments").objectValue());
                    else {
                        auto pathFragment = new cValueArray();
                        for (auto node : computeShortestNodePath(sourceNode, destinationNode)) {

                            //##########
                            auto fullNameNode = std::string(node->module->getFullName());
                            auto nameNode = (fullNameNode.find("nwtt") != std::string::npos || fullNameNode.find("dstt") != std::string::npos) ?
                                       std::string(node->module->getParentModule()->getName()) + "."
                                       + std::string(node->module->getFullName())
                                       : std::string(node->module->getFullName());

                            //############

                            pathFragment->add(nameNode);


                       // std::cout << "path " << std::string(node->module->getParentModule()->getFullName()) << endl;
                        }

                        pathFragments = new cValueArray();
                        pathFragments->add(pathFragment);
                    }
                    for (int l = 0; l < pathFragments->size(); l++) {
                        auto path = new Input::PathFragment();

                        auto pathFragment = check_and_cast<cValueArray *>(pathFragments->get(l).objectValue());
                        for (int m = 0; m < pathFragment->size(); m++) {
                            for (auto networkNode : input.networkNodes) {
                                auto name = pathFragment->get(m).stdstringValue();

                             //   int index = name.find('.');
                             //   auto nodeName = index != std::string::npos ? name.substr(0, index) : name;
                             //   auto interfaceName = index != std::string::npos ? name.substr(index + 1) : "";

                                //##########
                                auto fullNameNode = std::string(networkNode->module->getFullName());
                                auto nameNode = (fullNameNode.find("nwtt") != std::string::npos || fullNameNode.find("dstt") != std::string::npos) ?
                                              std::string(networkNode->module->getParentModule()->getName()) + "."
                                              + std::string(networkNode->module->getFullName())
                                              : std::string(networkNode->module->getFullName());
                                //##########

                                if (nameNode == name) {
                                    if (m != pathFragment->size() - 1) {
                                        auto startNode = networkNode;
                                        auto endNodeName = pathFragment->get(m + 1).stdstringValue();
                                       // int index = endNodeName.find('.');
                                      //  endNodeName = index != std::string::npos ? endNodeName.substr(0, index) : endNodeName;
                                        auto outputPort = *std::find_if(startNode->ports.begin(), startNode->ports.end(), [&] (const auto& port) {

                                            //###########
                                            auto fullNameNode = std::string(port->endNode->module->getFullName());
                                            auto nameNode = (fullNameNode.find("nwtt") != std::string::npos || fullNameNode.find("dstt") != std::string::npos) ?
                                                             std::string(port->endNode->module->getParentModule()->getName()) + "."
                                                             + std::string(port->endNode->module->getFullName())
                                                             : std::string(port->endNode->module->getFullName());
                                            //###########
                                            return nameNode == endNodeName;
                                                  //  && (interfaceName == "" || interfaceName == check_and_cast<NetworkInterface *>(port->module)->getInterfaceName());
                                        });
                                        path->outputPorts.push_back(outputPort);
                                        path->inputPorts.push_back(outputPort->otherPort);
                                    }
                                    path->networkNodes.push_back(networkNode);
                                    break;
                                }
                            }
                        }
                        flow->pathFragments.push_back(path);
                    }
                    if (!entry->containsKey("pathFragments"))
                        delete pathFragments;
                    input.flows.push_back(flow);
               // }
          //  }
     //   }
    }
    std::sort(input.flows.begin(), input.flows.end(), [] (const Input::Flow *r1, const Input::Flow *r2) {
        return r1->startApplication->pcp > r2->startApplication->pcp;
    });
}

ExternalGateScheduleConfigurator::~ExternalGateScheduleConfigurator(){
    //delete configuration;
   // delete distribution;

}

} //namespace

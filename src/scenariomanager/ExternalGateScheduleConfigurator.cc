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

namespace d6g {

Define_Module(ExternalGateScheduleConfigurator);

void ExternalGateScheduleConfigurator::initialize(int stage) {
  if (stage == INITSTAGE_LOCAL) {
    command = par("command").stdstringValue();

    networkFilePar = &par("networkFile");
    streamsFilePar = &par("streamsFile");
    histogramsFilePar = &par("histogramsFile");
    configurationFilePar = &par("configurationFile");

    // configuration =
    // check_and_cast<cValueArray*>(par("configuration").objectValue()); //
    // specified in .ini gateCycleDuration = par("gateCycleDuration");

    configurationComputedEvent = new ClockEvent("configuration_computed");
    configurationComputedEvent->setSchedulingPriority(0);

    hashMapNodeId = new std::map<std::string, uint16_t>();
  } else if (stage == INITSTAGE_GATE_SCHEDULE_CONFIGURATION) {
    // ChangeMonitor *monitor = nullptr;
    //! configuration->size() &&
    if ((monitor = check_and_cast<ChangeMonitor *>(
             getModuleByPath("monitor")))) { // query monitor if it exists
      // distributions = monitor->getDistributions();
      configuration = monitor->getStreamConfigurations();
    }
    computeConfiguration();
    configureGateScheduling();
    configureApplicationOffsets();

    clearConfiguration();
  } // end INITSTAGE_GATE_SCHEDULE_CONFIGURATION
}

void ExternalGateScheduleConfigurator::handleMessage(cMessage *msg) {
  if (msg == configurationComputedEvent) {
    // #########################################################
    std::cout << "Configurator (time from handleMessage): " << simTime()
              << " prio: " << msg->getSchedulingPriority() << " schedule: "
              << ((Output *)gateSchedulingOutput)->hasSchedule() << endl;
    // #########################################################
    if (((Output *)gateSchedulingOutput)->hasSchedule()) {
      configureGateScheduling();
      configureApplicationOffsets();
    }

  } else
    throw cRuntimeError("Unknown message type");
}

void ExternalGateScheduleConfigurator::handleParameterChange(const char *name) {
  if (!strcmp(name, "configuration")) {
    clearConfiguration();
    configuration = monitor->getStreamConfigurations();

    if (configurationComputedEvent->isScheduled()) {
      cancelEvent(configurationComputedEvent);
    }

    computeConfiguration();

    configurationComputedEvent->setSchedulingPriority(0);
    scheduleAt(commitTime, configurationComputedEvent);
    // #######################################################
    std::cout << "cycle: " << gateCycleDuration
              << "  Configurator (time from ParameterChange): "
              << simTime() + scheduleComputingTime << endl;
    // #######################################################
  }
  if (!strcmp(name, "gateCycleDuration")) {
    gateCycleDuration = par("gateCycleDuration");
  }
}

void ExternalGateScheduleConfigurator::printJson(
    std::ostream &stream, const cValue &value, int level) const { // not static
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
    } else if (auto map = dynamic_cast<cValueMap *>(object)) {
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
              newSecond = originalSecond.str();
            } else {
              newSecond =
                  std::isinf(originalSecond.doubleValue())
                      ? (originalSecond.doubleValue() > 0 ? cValue(1e308)
                                                          : cValue(-1e308))
                      : originalSecond;
            }
          }
          printJson(stream, newSecond, level + 1);
        }
        stream << "\n" << indent << "}";
      }
    } else
      throw cRuntimeError("Unknown object type");
  } else
    stream << value.str();
}

ExternalGateScheduleConfigurator::Output *
ExternalGateScheduleConfigurator::computeGateScheduling(
    const Input &input) const {
  writeStreamsToFile(input);
  writeNetworkToFile(input);
  writeDistributionsToFile();

  if (invokeScheduler()) {
    return (Output *)readOutputFromFile(input,
                                        configurationFilePar->stdstringValue());
  } else {
    // auto monitor =
    // check_and_cast<ChangeMonitor*>(getModuleByPath("monitor"));
    for (auto &application : input.applications) {
      auto appModule =
          (DynamicPacketSource *)(application->module->getSubmodule("source"));
      if (appModule->stopIfNotScheduled()) {
        // ##################################################
        std::cout << application->module->getFullName() << "   "
                  << appModule->getFullPath() << endl;
        // ##################################################
        cValueMap *element = appModule->getConfiguration();
        monitor->updateStreamConfigurations(element);
        delete element;
      } // endif
    } // endfor
    bubble(format("No schedule for the event at %.2f has been calculated.",
                  simTime().dbl() -
                      monitor->par("schedulerCallDelay").doubleValueInUnit("s"))
               .c_str());
    return new Output();
  }
}

bool ExternalGateScheduleConfigurator::invokeScheduler() const {
  bool configured = true;
  char currentDir[PATH_MAX];
  getcwd(currentDir, sizeof(currentDir));
  auto dir = getenv("LIBTSNDGM_ROOT");
  chdir(dir);
  std::string commandFromINI =
      format(command, networkFilePar->stdstringValue().c_str(),
             streamsFilePar->stdstringValue().c_str(),
             histogramsFilePar->stdstringValue().c_str(),
             configurationFilePar->stdstringValue().c_str());
  if (std::system(commandFromINI.c_str()) != 0) {
    // throw cRuntimeError(
    //  "Command execution failed, make sure LIBTSNDGM_ROOT is set and a
    //  scheduler tool is installed");
    configured = false;
  }
  chdir(currentDir);
  return configured;
}

// ########################################  WRITE
// ################################################
// #################################################################################################

void ExternalGateScheduleConfigurator::writeDistributionsToFile() const {
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
  }
  write(histogramsFilePar->stdstringValue(), json);

  delete json;
}

void ExternalGateScheduleConfigurator::writeStreamsToFile(
    const Input &input) const {
  auto jsonStreams = convertInputToJsonStreams(input);
  cValueMap *metaData = new cValueMap();
  jsonStreams->set("META", metaData);
  metaData->set("simulation_time", simTime().inUnit(SIMTIME_US));
  metaData->set("simulation_time_unit", "us");
  metaData->set("schedule_type", "Hypercycle");
  metaData->set("fixed_priority", monitor->isFixedPriority());
  write(streamsFilePar->stdstringValue(), jsonStreams);

  delete jsonStreams;
}

void ExternalGateScheduleConfigurator::writeNetworkToFile(
    const Input &input) const {
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

void ExternalGateScheduleConfigurator::write(std::string fileName,
                                             cValueMap *json) const {
  std::ofstream stream;
  auto path = fileName;
  stream.open(path.c_str());
  if (stream.fail())
    throw cRuntimeError("Cannot open file %s", fileName.c_str());
  this->printJson(stream, cValue(json));
}

// ###############################################################################################
// ##################################### CONVERT
// #################################################

cValueMap *ExternalGateScheduleConfigurator::convertInputToJsonStreams(
    const Input &input) const {
  cValueMap *json = new cValueMap();
  cValueArray *jsonFlows = new cValueArray();
  json->set("flows", jsonFlows);

  for (auto flow : input.flows) {
    cValueMap *jsonFlow = new cValueMap();
    jsonFlows->add(jsonFlow);
    jsonFlow->set("name", flow->name);
    jsonFlow->set("type", "unicast");

    Application *dynApplication = (Application *)(flow->startApplication);

    jsonFlow->set("source_device",
                  dynApplication->device->module->getFullName());
    jsonFlow->set("pcp", flow->gateIndex);
    jsonFlow->set("packet_periodicity",
                  dynApplication->packetInterval.dbl() * 1000000);
    jsonFlow->set("packet_periodicity_unit", "us");
    jsonFlow->set("packet_size", b(dynApplication->packetLength).get());
    jsonFlow->set("packet_size_unit", "bit");
    jsonFlow->set("hard_constraint_time_latency",
                  dynApplication->maxLatency.dbl() * 1000000);
    jsonFlow->set("hard_constraint_time_jitter",
                  dynApplication->maxJitter.dbl() * 1000000);
    jsonFlow->set("hard_constraint_time_unit", "us");
    // jsonFlow->set("weight", dynApplication->weight);
    jsonFlow->set("phase", dynApplication->phase.dbl());
    jsonFlow->set("phase_unit", "us");
    jsonFlow->set("objective_type", dynApplication->objectiveType);
    // jsonFlow->set("packet_loss", dynApplication->packetLoss);

    cValueArray *endDevices = new cValueArray();
    jsonFlow->set("end_devices", endDevices);
    endDevices->add(cValue(flow->endDevice->module->getFullName()));

    cValueArray *pdb_map = new cValueArray();
    jsonFlow->set("pdb_map", pdb_map);

    cValueArray *hops = new cValueArray();
    jsonFlow->set("route", hops);
    int wirelessCnt = 0;
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
        wirelessCnt += (int)addEntryToPDBMap(pdb_map, networkNode->module,
                                             nextNetworkNode->module);
      } // 3. for
    } // 2. for
    jsonFlow->set("wirelessPath", cValue((bool)wirelessCnt));
    if (wirelessCnt)
      jsonFlow->set("wirelessPathLinkCnt", wirelessCnt);

    int pdbSize = pdb_map->size();
    double reliability = std::pow(
        dynApplication->reliability,
        1.0 / pdbSize); // #DetCom-link-th root of the reliability parameter
    // int policy = dynApplication->policy;
    for (int i = 0; i < pdbSize; i++) {
      auto vMap = check_and_cast<cValueMap *>(pdb_map->get(i).objectValue());
      vMap->set("reliability", reliability);
      // vMap->set("policy", dynApplication->policy);
    } // endfor
  } // 1.for
  return json;
}

cValueMap *ExternalGateScheduleConfigurator::convertInputToJsonNetwork(
    const Input &input) const {
  cValueMap *json = new cValueMap();
  cValueArray *jsonDevices = new cValueArray();
  json->set("end_devices", jsonDevices);
  for (auto device : input.devices) {
    cValueMap *jsonDevice = new cValueMap();
    jsonDevices->add(jsonDevice);
    jsonDevice->set("name", device->module->getFullName());
    jsonDevice->set("device_type", (short)DeviceType::END_DEVICE);
    jsonDevice->set("processing_delay", 0); // temporary hard coded
    jsonDevice->set("processing_delay_unit", "us");

    (*hashMapNodeId)[std::string(device->module->getFullName())] =
        device->module->getId();
  }
  cValueArray *jsonSwitches = new cValueArray();
  json->set("switches", jsonSwitches);
  for (int i = 0; i < input.switches.size(); i++) {
    auto switch_ = input.switches[i];
    cValueMap *jsonSwitch = new cValueMap();
    jsonSwitches->add(jsonSwitch);

    auto nameNetworkNode = getExpandedNodeName(switch_->module);
    jsonSwitch->set("name", nameNetworkNode);

    (*hashMapNodeId)[nameNetworkNode] = switch_->module->getId();

    auto jsonPorts = new cValueArray();
    jsonSwitch->set("ports", jsonPorts);
    for (int j = 0; j < switch_->ports.size(); j++) {
      auto port = switch_->ports[j];
      auto jsonPort = new cValueMap();
      jsonPorts->add(jsonPort);

      auto nameNode = getExpandedNodeName(port->startNode->module);
      jsonPort->set("port_name", nameNode + "-" + port->module->getFullName());
      auto connectedToFullName = getExpandedNodeName(port->endNode->module);
      jsonPort->set("connects_to", connectedToFullName);

      DetComLinkType detComLinkType;
      short link_type = isDetComLink(port->startNode->module,
                                     port->endNode->module, detComLinkType);
      jsonPort->set("wireless_link_descr",
                    getDetComLinkDescription(detComLinkType));
      jsonPort->set("link_type", link_type);
      if (link_type)
        jsonPort->set("multiple_subcarriers", true);

      jsonSwitch->set("processing_delay", 0); // temporary hard coded
      jsonSwitch->set("processing_delay_unit", "us");
      jsonSwitch->set("device_type", getSwitchType(port->startNode->module));
      jsonPort->set("propagation_delay", port->propagationTime.dbl() * 1000000);
      jsonPort->set("propagation_delay_unit", "us");
      jsonPort->set("data_rate", bps(port->datarate).get() / 1000000);
      jsonPort->set("data_rate_size_unit", "bit");
      jsonPort->set("data_rate_time_unit", "us");

      auto queue = port->module->findModuleByPath(".macLayer.queue");
      jsonPort->set("num_queues",
                    strcmp(queue->getModuleType()->getName(), "PacketQueue")
                        ? queue->par("numTrafficClasses").intValue()
                        : 0);
    }
  }
  return json;
}

// #############################################################################################
// #############################################################################################

std::string
ExternalGateScheduleConfigurator::getExpandedNodeName(cModule *module) const {
  auto fullNameNetworkNode = std::string(module->getFullName());
  auto type = getSwitchType(module);
  auto nameNetworkNode =
      (1 < type && type < 4)
          ? std::string(module->getParentModule()->getName()) + "." +
                std::string(module->getFullName())
          : std::string(module->getFullName());
  return nameNetworkNode;
}

bool ExternalGateScheduleConfigurator::isDetComLink(
    cModule *source, cModule *target, DetComLinkType &detComLinkType) const {
  const char *translator = "d6g.devices.tsntranslator.TsnTranslator";
  detComLinkType = DetComLinkType::NO_DETCOM_LINK;
  if (strcmp(source->getNedTypeName(), translator) ||
      (strcmp(target->getNedTypeName(), translator)))
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

bool ExternalGateScheduleConfigurator::addEntryToPDBMap(cValueArray *pdb_map,
                                                        cModule *source,
                                                        cModule *target) const {
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

    if (linkType == DetComLinkType::DSTT_DSTT) {
      pdb_map->add(pdb);
      std::string egress =
          (nameNextNetworkNode.find('.') != std::string::npos)
              ? nameNextNetworkNode.substr(nameNextNetworkNode.find('.') + 1)
              : nameNextNetworkNode;
      pdb->set("key", nameNetworkNode + "-" + egress);

      auto expr1 =
          ((TsnTranslator *)source)->getDistributionExpression("delayUplink");
      auto expr2 =
          ((TsnTranslator *)target)->getDistributionExpression("delayDownlink");

      cMsgPar *convolveExpr = new cMsgPar("expression");
      cMsgPar *details = new cMsgPar("details");
      details->setStringValue(
          (std::string(source->getParentModule()->getName()) + "." +
           source->getFullName() + "-" + target->getFullName())
              .c_str());
      convolveExpr->setStringValue((expr1->str() + "+" + expr2->str()).c_str());

      std::cout << " details: " << details->str()
                << " expr: " << convolveExpr->stringValue() << endl;
      monitor->notify(this->getFullName(), convolveExpr, details);
      delete details;
      delete expr1;
      delete expr2;
      delete convolveExpr;

    } else {
      auto delayParam = {nameNetworkNode + getDetComLinkDescription(linkType),
                         nameNextNetworkNode +
                             getDetComLinkDescription(linkType)};
      for (auto key : delayParam) {
        auto distributions = monitor->getDistributions();
        if (distributions->find(key) != distributions->end()) {
          pdb_map->add(pdb);
          pdb->set("key", key);
          break;
        } // endif
      } // endfor
    }
    if (mapSize == pdb_map->size()) {
      delete pdb;
    }
    return true;
  } // endif

  return false;
}

short ExternalGateScheduleConfigurator::getSwitchType(cModule *module) const {
  auto type = DeviceType::UNSPECIFIED;
  if (!strcmp(module->getModuleType()->getName(), "TsnSwitch")) {
    type = DeviceType::TSN_BRIDGE;
  }
  if (!strcmp(module->getModuleType()->getName(), "TsnTranslator")) {
    if (module->par("isDstt")) {
      type = DeviceType::DS_TT;
    } else {
      type = DeviceType::NW_TT;
    }
  }
  return (short)type;
}

std::string ExternalGateScheduleConfigurator::getDetComLinkDescription(
    DetComLinkType type) const { // for DetCom links
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

void ExternalGateScheduleConfigurator::addFlows(Input &input) const {
  EV_DEBUG << "Computing flows from configuration" << EV_FIELD(configuration)
           << EV_ENDL;
  for (int k = 0; k < configuration->size(); k++) {
    auto entry =
        check_and_cast<cValueMap *>(configuration->get(k).objectValue());

    cModule *source = getModuleByPath(entry->get("source").stringValue());
    auto sourceNode = (Node *)topology->getNodeFor(source);
    auto startDevice = input.getDevice(source);

    cModule *destination =
        getModuleByPath(entry->get("destination").stringValue());
    auto destinationNode = (Node *)topology->getNodeFor(destination);
    auto endDevice = input.getDevice(destination);

    auto startApplication = new Application(); // new Input::Application();
    auto startApplicationModule = startDevice->module->getModuleByPath(
        (std::string(".") +
         std::string(entry->get("application").stringValue()))
            .c_str());
    if (startApplicationModule == nullptr)
      throw cRuntimeError("Cannot find flow start application, path = %s",
                          entry->get("application").stringValue());

    startApplication->module = startApplicationModule;
    startApplication->device = startDevice;
    startApplication->pcp = entry->get("pcp").intValue();
    startApplication->packetLength =
        b(entry->get("packetLength").doubleValueInUnit("b") +
          464); // add Header +58B
    startApplication->packetInterval =
        entry->get("packetInterval").doubleValueInUnit("s"); // packetInterval;
    startApplication->maxLatency =
        entry->containsKey("maxLatency")
            ? entry->get("maxLatency").doubleValueInUnit("s")
            : -1;
    startApplication->maxJitter =
        entry->containsKey("maxJitter")
            ? entry->get("maxJitter").doubleValueInUnit("s")
            : 0;
    // startApplication->packetLoss = entry->get("packetLoss").intValue();
    startApplication->objectiveType = entry->get("objectiveType").intValue();
    // startApplication->weight = entry->get("weight").doubleValue();
    simtime_t phase = entry->get("phase").doubleValueInUnit("us");
    startApplication->phase = phase <= 0 ? 0 : phase;
    // startApplication->policy = entry->get("policy").intValue();
    startApplication->reliability = entry->get("reliability").doubleValue();

    input.applications.push_back(startApplication);
    // EV_DEBUG << "Adding flow from configuration" << EV_FIELD(source) <<
    // EV_FIELD(destination) << EV_FIELD(pcp) << EV_FIELD(packetLength) <<
    // EV_FIELD(packetInterval, packetInterval.ustr()) << EV_FIELD(maxLatency,
    // maxLatency.ustr()) << EV_FIELD(maxJitter, maxJitter.ustr()) << EV_ENDL;

    auto flow = new Input::Flow();
    if (!entry->containsKey("name"))
      throw cRuntimeError("Cannot identify the flow!");
    flow->name = entry->get("name").stringValue();
    flow->gateIndex = entry->get("gateIndex").intValue();
    // flow->cutthroughSwitchingHeaderSize =
    // entry->containsKey("cutthroughSwitchingHeaderSize") ?
    // b(entry->get("cutthroughSwitchingHeaderSize").doubleValueInUnit("b")) :
    // b(0);
    flow->startApplication = startApplication;
    flow->endDevice = endDevice;

    cValueArray *pathFragments;
    if (entry->containsKey("pathFragments"))
      pathFragments = check_and_cast<cValueArray *>(
          entry->get("pathFragments").objectValue());
    else {
      pathFragments = new cValueArray();
      // auto pathFragment = new cValueArray();
      for (auto node : computeShortestNodePath(
               sourceNode, destinationNode)) { // berechne Route: vector
        auto nameNode = getExpandedNodeName(node->module);
        // pathFragment->add(nameNode);
        pathFragments->add(nameNode);
      }
      // pathFragments = new cValueArray();
      // pathFragments->add(pathFragment);
    }

    //   for (int l = 0; l < pathFragments->size(); l++) {
    auto path = new Input::PathFragment();
    //    auto pathFragment =
    //    check_and_cast<cValueArray*>(pathFragments->get(l).objectValue());
    for (int m = 0; m < pathFragments->size(); m++) { //
      for (auto networkNode : input.networkNodes) {
        auto name = pathFragments->get(m).stdstringValue(); //
        auto nameNode = getExpandedNodeName(networkNode->module);
        if (nameNode == name) {
          if (m != pathFragments->size() - 1) { //
            auto startNode = networkNode;
            auto endNodeName = pathFragments->get(m + 1).stdstringValue(); //
            auto outputPort =
                *std::find_if(startNode->ports.begin(), startNode->ports.end(),
                              [&](const auto &port) {
                                auto nameNode =
                                    getExpandedNodeName(port->endNode->module);
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
    //   }//endfor
    if (!entry->containsKey("pathFragments"))
      delete pathFragments;
    input.flows.push_back(flow);

  } // endfor
  /*std::sort(input.flows.begin(), input.flows.end(), [] (const Input::Flow *r1,
  const Input::Flow *r2) { return r1->startApplication->pcp >
  r2->startApplication->pcp;
  });*/
}

ExternalGateScheduleConfigurator::Input::Port *
ExternalGateScheduleConfigurator::
    getConfigurablePort( // LINK : "[source,target]" as string
        const Input &input, std::string &linkName) const {
  linkName = linkName.substr(1, linkName.size() - 2);
  std::stringstream ss(linkName);
  std::string source, target;
  std::getline(ss, source, ',');
  std::getline(ss, target);

  auto it =
      std::find_if(input.networkNodes.begin(), input.networkNodes.end(),
                   [&](Input::NetworkNode *switch_) {
                     return getExpandedNodeName(switch_->module) == source;
                   });
  if (it == input.networkNodes.end())
    throw cRuntimeError("Cannot find TSN device: %s", source.c_str());
  auto sourceNode = *it;

  it = std::find_if(input.networkNodes.begin(), input.networkNodes.end(),
                    [&](Input::NetworkNode *switch_) {
                      return getExpandedNodeName(switch_->module) == target;
                    });
  if (it == input.networkNodes.end())
    throw cRuntimeError("Cannot find connected TSN device: %s", target.c_str());
  auto targetNode = *it;

  auto jt = std::find_if(sourceNode->ports.begin(), sourceNode->ports.end(),
                         [&](Input::Port *port) {
                           return port->startNode == sourceNode &&
                                  port->endNode == targetNode;
                         });
  if (jt == sourceNode->ports.end())
    throw cRuntimeError("Cannot find port: %s", linkName.c_str());
  auto port = *jt;
  return port;
}

ExternalGateScheduleConfigurator::Output *
ExternalGateScheduleConfigurator::convertJsonToOutput(
    const Input &input, const cValueMap *json) const {
  auto output = new Output();
  auto jsonMETA =
      check_and_cast<cValueMap *>(json->get("META").objectValue()); // META data

  commitTime = SimTime(jsonMETA->get("commit_time").intValue(), SIMTIME_NS);
  scheduleComputingTime =
      SimTime(jsonMETA->get("computing_time").intValue(), SIMTIME_NS);
  gateCycleDuration =
      SimTime(jsonMETA->get("hypercycle").intValue(), SIMTIME_NS);
  std::cout << "cycle: " << gateCycleDuration
            << " computing_time : " << scheduleComputingTime << "  "
            << commitTime << " from CONFIGURATOR: " << std::endl;

  auto jsonGCL = check_and_cast<cValueMap *>(
      json->get("GCL").objectValue()); // Gate control lists

  for (auto &gcl : jsonGCL->getFields()) { // 1.for
    auto linkName = gcl.first;
    auto port = getConfigurablePort(input, linkName);
    auto &schedules = output->gateSchedules[port];
    auto queues = check_and_cast<cValueMap *>(gcl.second.objectValue());
    for (auto &queue : queues->getFields()) {
      auto queueIndex = std::stoi(queue.first.substr(1));
      cValueMap *queueMap =
          check_and_cast<cValueMap *>(queue.second.objectValue());
      auto schedule = new Schedule();
      schedule->port = port;
      schedule->gateIndex = queueIndex;
      schedule->cycleDuration = gateCycleDuration;
      schedule->open = queueMap->get("initial").intValue();

      auto dur = check_and_cast<cValueArray *>(
          queueMap->get("durations").objectValue());
      schedule->durations = new cValueArray();
      for (int i = 0; i < dur->size(); i++) {
        const cValue &value = (*dur)[i];
        cValue newValue = cValue(value.intValue(), "ns");
        schedule->durations->add(newValue);
      }
      dur = nullptr;
      schedule->offset =
          SimTime(queueMap->get("offset").intValue(), SIMTIME_NS);
      schedules.push_back(schedule);
    } // endfor
  } // 1.for
  auto talkers = check_and_cast<cValueMap *>( // APPLICATIONS
      json->get("TALKERS").objectValue());
  for (auto &field : talkers->getFields()) {
    const std::string &key = field.first;

    std::stringstream ss(key);
    std::string flowID, packetNr;
    std::getline(ss, flowID, '#');
    std::getline(ss, packetNr);

    auto flowIt =
        std::find_if(input.flows.begin(), input.flows.end(),
                     [&](Input::Flow *flow) { return flow->name == flowID; });
    if (flowIt == input.flows.end())
      throw cRuntimeError("Cannot find flow: %s", flowID.c_str());
    auto flow = *flowIt;
    auto application = flow->startApplication;
    // double firstSendingTime = field.second.intValue(); //ns
    auto startTime = SimTime(field.second.intValue(), SIMTIME_NS);
    // while (startTime < 0) startTime += application->packetInterval.dbl();
    if (output->applicationStartTimes.find(application) ==
        output->applicationStartTimes.end()) {
      output->applicationStartTimes[application] = startTime;
    } // endif
    output->applicationStartTimesArray[application].push_back(startTime);
  } // endfor

  return output;
}

void ExternalGateScheduleConfigurator::clearConfiguration() {

  deleteOldConfigurationPar();
  if (gateSchedulingInput == nullptr ||
      (Output *)gateSchedulingOutput == nullptr)
    return;
  if (topology != nullptr)
    topology->clear();

  delete gateSchedulingInput;
  gateSchedulingInput = nullptr;

  auto scheduleMap = gateSchedulingOutput->gateSchedules;
  for (auto it = scheduleMap.begin(); it != scheduleMap.end(); ++it) {
    std::vector<Output::Schedule *> &schedules = it->second;
    for (Output::Schedule *schedule : schedules) {
      Schedule *oldSchedule = (Schedule *)schedule;
      oldSchedule->~Schedule();
    } // endfor
  } // endfor
  delete (Output *)gateSchedulingOutput;
  gateSchedulingOutput = nullptr;
}

// TODO fix me...
void ExternalGateScheduleConfigurator::configureGateScheduling() {
  std::cout << "Configure Scheduling :  "
            << ((Output *)gateSchedulingOutput)->hasSchedule() << endl;

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
      auto gate = dynamic_cast<PeriodicGate *>(
          queue->getSubmodule("transmissionGate", gateIndex));
      gate->par("initiallyOpen") = newSchedule->open;
      cPar &offsetPar = gate->par("offset");
      // std::cout << "Index:  " << newSchedule->gateIndex << " offset: "
      //   << newSchedule->offset << endl;
      offsetPar.setValue(cValue(newSchedule->offset.inUnit(SIMTIME_NS), "ns"));

      // gate->par("offset") = 0.019975662; //newSchedule->offset.dbl();

      cPar &durationsPar = gate->par("durations");
      durationsPar.copyIfShared();
      durationsPar.setObjectValue(newSchedule->durations->dup());
    } // endfor
  } // endfor
}

void ExternalGateScheduleConfigurator::configureApplicationOffsets() {
  auto output = (Output *)gateSchedulingOutput;
  for (auto &it : output->applicationStartTimes) {
    auto startOffset = it.second;
    auto applicationModule = it.first->module;
    EV_DEBUG
        << "Setting initial packet production offset for application source"
        << EV_FIELD(applicationModule) << EV_FIELD(startOffset) << EV_ENDL;
    auto sourceModule = applicationModule->getSubmodule("source");
    const auto &offsets = output->applicationStartTimesArray[it.first];
    ((DynamicPacketSource *)sourceModule)->setNewConfiguration(offsets);
    // sourceModule->par("initialProductionOffset") = startOffset.dbl();
    // //logical: You may only change it once at the beginning
  }
}

void ExternalGateScheduleConfigurator::deleteOldConfigurationPar() {
  if (configuration != nullptr) {
    for (int i = 0; i < configuration->size(); i++) {
      auto entry = configuration->remove(i);
      delete entry.objectValue();
    }
    configuration->clear();
    delete configuration;
    configuration = nullptr;
  }
}

ExternalGateScheduleConfigurator::~ExternalGateScheduleConfigurator() {
  clearConfiguration();

  if (configuration != nullptr)
    delete configuration;

  delete hashMapNodeId;

  if (configurationComputedEvent != nullptr)
    drop(configurationComputedEvent);
  delete configurationComputedEvent;
}

} // namespace d6g

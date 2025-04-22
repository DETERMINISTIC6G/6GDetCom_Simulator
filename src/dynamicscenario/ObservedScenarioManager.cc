// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ObservedScenarioManager.h"

#include "inet/common/XMLUtils.h"

namespace d6g {

Define_Module(ObservedScenarioManager);

void ObservedScenarioManager::initialize() { ScenarioManager::initialize(); }

void ObservedScenarioManager::handleMessage(cMessage *msg)
{
    auto node = check_and_cast<ScenarioTimer *>(msg)->getXmlNode();
    std::string tag = node->getTagName();
    auto checkParamNotPermitted = [&](const char *param) {
        if (!strcmp(param, "initialProductionOffset")) {
            delete msg;
            throw cRuntimeError("Changing \"%s\" parameter in the script is not permitted.", param);
        }
    };
    if (tag == "at") {
        for (const cXMLElement *child = node->getFirstChild(); child; child = child->getNextSibling()) {
            const char *notPermittedParam = xmlutils::getMandatoryFilledAttribute(*child, "par");
            checkParamNotPermitted(notPermittedParam);
        }
    }
    if (tag == "set-param") {
        const char *notPermittedParam = xmlutils::getMandatoryFilledAttribute(*node, "par");
        checkParamNotPermitted(notPermittedParam);
    }
    processCommand(node);
    numDone++;
    if (msg->isSelfMessage()) {
        emit(DynamicScenarioObserver::scenarioEventSignal, msg);
    }
    delete msg;
}

} // namespace d6g

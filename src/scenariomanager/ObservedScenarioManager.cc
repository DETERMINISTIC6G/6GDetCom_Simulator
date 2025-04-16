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

#include "ObservedScenarioManager.h"
#include "inet/common/XMLUtils.h"

namespace d6g {

Define_Module(ObservedScenarioManager);

void ObservedScenarioManager::initialize() { ScenarioManager::initialize(); }

void ObservedScenarioManager::handleMessage(cMessage *msg)
{
    auto node = check_and_cast<ScenarioTimer*>(msg)->getXmlNode();
    std::string tag = node->getTagName();
    auto checkParamNotPermitted =
            [&](const char *param) {
                if (!strcmp(param, "initialProductionOffset")) {
                    delete msg;
                    throw cRuntimeError(
                            "Changing \"%s\" parameter in the script is not permitted.",
                            param);
                }
            };
    if (tag == "at") {
        for (const cXMLElement *child = node->getFirstChild(); child; child =
                child->getNextSibling()) {
            const char *notPermittedParam =
                    xmlutils::getMandatoryFilledAttribute(*child, "par");
            checkParamNotPermitted(notPermittedParam);
        }
    }
    if (tag == "set-param") {
        const char *notPermittedParam = xmlutils::getMandatoryFilledAttribute(
                *node, "par");
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

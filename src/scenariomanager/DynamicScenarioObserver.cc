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


#include "DynamicScenarioObserver.h"
#include "inet/common/scenario/ScenarioTimer_m.h"
#include "ChangeMonitor.h"

#include "inet/common/INETUtils.h"
#include "inet/common/XMLUtils.h"



namespace d6g {

const simsignal_t DynamicScenarioObserver::scenarioEventSignal =
        cComponent::registerSignal("scenario-event");
const simsignal_t DynamicScenarioObserver::parameterChangeSignal =
        cComponent::registerSignal("parameter-change-event");
const simsignal_t DynamicScenarioObserver::distributionChangeSignal =
        cComponent::registerSignal("distribution-event");

void DynamicScenarioObserver::receiveSignal(cComponent *source,
        simsignal_t signalID, cObject *obj, cObject *details) {

    ChangeMonitor *monitor = (dynamic_cast<ChangeMonitor*>(this->monitor));

    cMessage *msg = check_and_cast<cMessage*>(obj);
    if (signalID == scenarioEventSignal) {

        auto node = check_and_cast<ScenarioTimer*>(msg)->getXmlNode();

        EV << "Received message (scenariomanager): " << msg->getName() << " from source: " << source->getFullName() << endl;

    }
    if (signalID == parameterChangeSignal) {

        EV << "Received message (app): " << msg->getName() << " from source: " << source->getFullPath() << endl;


        DynamicPacketSource *sourceModule = dynamic_cast<DynamicPacketSource *>(source);
        if (sourceModule) {
           cValueMap* value = sourceModule->getConfiguration();
           EV << value->get("application").stringValue() << " !!  " <<
                   value->get("destination").stringValue() << "  !!  " <<  value->get("packetInterval").str()
                   << endl;

           delete value;
        }

        monitor->notify(source->getFullPath());



    }
    if (signalID == distributionChangeSignal) {

        EV << "Received message (distribution): " << msg->getName() << " from source: " << source->getFullPath() << endl;

        (dynamic_cast<ChangeMonitor*>(this->monitor))->notify(source->getFullPath());
    }

}

} /* namespace d6g */

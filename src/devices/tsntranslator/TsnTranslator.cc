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

#include "TsnTranslator.h"

namespace d6g {
//using namespace inet;

Define_Module(TsnTranslator);

void TsnTranslator::initialize()
{
    //cSimpleModule::initialize(stage);
    distributionChangeEvent = new cMessage("distribution-changed");
}

/*void TsnTranslator::handleMessage(cMessage *msg)
{
    cSimpleModule::handleMessage(msg);

}*/


void TsnTranslator::handleParameterChange(const char *name) {
    if (!strcmp(name, "delayDownlink") || !strcmp(name, "delayUplink") ) {
        cMsgPar *details = new cMsgPar("details");
        details->setStringValue(name);
        emit(DynamicScenarioObserver::distributionChangeSignal,
                distributionChangeEvent, details);
        delete details;
    }
}

TsnTranslator::~TsnTranslator() {
    delete distributionChangeEvent;
}

} /* namespace d6g */

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

#ifndef SCENARIOMANAGER_DYNAMICSCENARIOOBSERVER_H_
#define SCENARIOMANAGER_DYNAMICSCENARIOOBSERVER_H_

#include <omnetpp.h>
//#include "ChangeMonitor.h"
#include <cstdlib>
#include "inet/common/scenario/ScenarioTimer_m.h"




using namespace omnetpp;
using namespace inet;
using namespace inet::common;

namespace d6g {

class DynamicScenarioObserver: public cListener {

private :
    cModule *monitor = nullptr;


public:
      static const simsignal_t scenarioEventSignal;
      static const simsignal_t parameterChangeSignal;
      static const simsignal_t distributionChangeSignal;


public:

    DynamicScenarioObserver(cModule *monitor) : monitor(monitor) {};

    virtual void receiveSignal(cComponent *source, simsignal_t signalID,
            cObject *obj, cObject *details) override;



};

} /* namespace d6g */

#endif /* SCENARIOMANAGER_DYNAMICSCENARIOOBSERVER_H_ */

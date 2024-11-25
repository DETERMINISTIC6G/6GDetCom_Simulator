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

#ifndef __DYNAMIC_SCENARIO_DYNAMICPACKETSOURCE_H_
#define __DYNAMIC_SCENARIO_DYNAMICPACKETSOURCE_H_

#include <omnetpp.h>
#include "inet/queueing/source/ActivePacketSource.h"

using namespace omnetpp;
using namespace inet;
using namespace inet::queueing;

namespace d6g {

/**
 * TODO - Generated class
 */
class DynamicPacketSource: public ActivePacketSource {

protected:
    bool isStartedParameter = true;

protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleParameterChange(const char *name) override;

    virtual void scheduleProductionTimer(clocktime_t delay) override;
    virtual void scheduleProductionTimerAndProducePacket() override;

/*public:
    virtual ~DynamicPacketSource() {
        cancelAndDeleteClockEvent(productionTimer);
    }*/

};

} //namespace

#endif

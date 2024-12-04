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

#ifndef __DYNAMIC_SCENARIO_CHANGEMONITOR_H_
#define __DYNAMIC_SCENARIO_CHANGEMONITOR_H_

#include <omnetpp.h>
#include "DynamicScenarioObserver.h"
//#include "ObservedScenarioManager.h"
#include "../apps/dynamicsource/DynamicPacketSource.h"
#include "inet/common/clock/ClockUserModuleMixin.h"

#include "inet/common/scenario/ScenarioTimer_m.h"

using namespace omnetpp;
using namespace inet;
//using namespace inet::common;

namespace d6g {

/**
 * TODO - Generated class
 */
class ChangeMonitor : public ClockUserModuleMixin<cSimpleModule> // public cSimpleModule
{

  private:
    DynamicScenarioObserver *observer;
    ClockEvent *timer = nullptr;
    cPar *schedulerCallDelayParameter = nullptr;


  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    void subscribe();
    void prepaireChangesForProcessing();


  public:

    virtual void externalSchedulerCall();
    void notify(std::string source);
    ~ChangeMonitor() override;
};

} //namespace

#endif

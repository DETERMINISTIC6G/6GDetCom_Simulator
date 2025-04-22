// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __DYNAMIC_SCENARIO_OBSERVEDSCENARIOMANAGER_H_
#define __DYNAMIC_SCENARIO_OBSERVEDSCENARIOMANAGER_H_

#include <omnetpp.h>

#include "DynamicScenarioObserver.h"
#include "inet/common/scenario/ScenarioManager.h"
#include "inet/common/scenario/ScenarioTimer_m.h"

using namespace omnetpp;
using namespace inet;

namespace d6g {

class ObservedScenarioManager : public ScenarioManager
{

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

} // namespace d6g

#endif

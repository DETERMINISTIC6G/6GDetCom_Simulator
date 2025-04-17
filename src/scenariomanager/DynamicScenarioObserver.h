// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SCENARIOMANAGER_DYNAMICSCENARIOOBSERVER_H_
#define SCENARIOMANAGER_DYNAMICSCENARIOOBSERVER_H_

#include <omnetpp.h>

#include "inet/common/clock/ClockUserModuleMixin.h"

using namespace omnetpp;
using namespace inet;

namespace d6g {

class ChangeMonitor;

class DynamicScenarioObserver : public cListener
{

  private:
    ChangeMonitor *monitor = nullptr;

  public:
    static const simsignal_t scenarioEventSignal;
    static const simsignal_t parameterChangeSignal;
    static const simsignal_t distributionChangeSignal;

  private:
    void computeFromSamples(intval_t numberOfSamples, const cDynamicExpression &dynExpr, cValueArray &jsonBins);

  public:
    DynamicScenarioObserver(ChangeMonitor *monitor);

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    cValueArray *createHistogram(cDynamicExpression &dynExpr, cObject *details = nullptr);
};

} /* namespace d6g */

#endif /* SCENARIOMANAGER_DYNAMICSCENARIOOBSERVER_H_ */

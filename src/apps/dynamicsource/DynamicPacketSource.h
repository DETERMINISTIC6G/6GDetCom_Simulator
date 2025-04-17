// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __DYNAMIC_SCENARIO_DYNAMICPACKETSOURCE_H_
#define __DYNAMIC_SCENARIO_DYNAMICPACKETSOURCE_H_

#include <omnetpp.h>

#include "../../devices/tsntranslator/TsnTranslator.h"
#include "../../scenariomanager/DynamicScenarioObserver.h"
#include "../../scenariomanager/ExternalGateScheduleConfigurator.h"
#include "inet/applications/udpapp/UdpSocketIo.h"
#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/source/ActivePacketSource.h"

using namespace omnetpp;
using namespace inet;
using namespace inet::queueing;

namespace d6g {

class DynamicPacketSource : public ActivePacketSource
{

  private:
    /* Set new parameters quietly (without triggering effects)*/
    volatile bool ignoreChange = false;
    /* 'Pending' parameters are only adopted if the stream has been scheduled;
     * otherwise, stop the packet production */
    bool hasSchedulerPermission = false;

    bool pendingEnabledState;
    cPar *runningState = nullptr;

    ClockEvent *parameterChangeEvent = nullptr;
    std::string streamName = "";

    std::vector<simtime_t> offsets;
    size_t nextProductionIndex = 0;
    simtime_t firstFrameOffsetCurrent;
    simtime_t phase;

    friend class ExternalGateScheduleConfigurator;

  private:
    void computeProductionOffsets(const cValueArray *values);
    // These methods are used by ExternalGateScheduleConfigurator modules
    bool stopIfNotScheduled();
    void setNewConfiguration(const std::vector<simtime_t> &simtimeVector);

  protected:
    virtual void initialize(int stage) override;
    void incrementProductionOffset();
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleParameterChange(const char *name) override;

    virtual void scheduleProductionTimer(clocktime_t delay) override;
    virtual void scheduleProductionTimerAndProducePacket() override;

  public:
    virtual cValueMap *getConfiguration() const;

    ~DynamicPacketSource() override;
};

} // namespace d6g

#endif

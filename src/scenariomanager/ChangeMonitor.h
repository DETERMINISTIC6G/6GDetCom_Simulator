// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __DYNAMIC_SCENARIO_CHANGEMONITOR_H_
#define __DYNAMIC_SCENARIO_CHANGEMONITOR_H_

#include <omnetpp.h>

#include "../apps/dynamicsource/DynamicPacketSource.h"
#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/common/scenario/ScenarioTimer_m.h"
#include "inet/linklayer/configurator/gatescheduling/base/GateScheduleConfiguratorBase.h"

using namespace omnetpp;
using namespace inet;

namespace d6g {

class DynamicScenarioObserver;

class ChangeMonitor : public cSimpleModule // inet::ClockUserModuleMixin<cSimpleModule>
{

    friend class DynamicScenarioObserver;

  protected:
    class Mapping
    {
      public:
        std::string name;
        int pcp;
        int gateIndex;
        std::string application;
        std::string source;
        std::string destination;
        cValue packetLength;
        cValue packetInterval;
        cValue maxLatency;
        cValue maxJitter;
        double reliability;
        cValue phase;
        cValue customParams;

        friend std::ostream &operator<<(std::ostream &os, const Mapping &mapping)
        {
            os << "name: " << mapping.name << ", source: " << mapping.source << ", pcp: " << mapping.pcp
               << ", gateIndex: " << mapping.gateIndex << ", application: " << mapping.application
               << ", destination: " << mapping.destination;
            return os;
        }
    };

  private:
    DynamicScenarioObserver *observer;
    GateScheduleConfiguratorBase *gateScheduleConfigurator;

    ClockEvent *timer = nullptr;
    cPar *schedulerCallDelayParameter = nullptr;

    std::vector<Mapping> streamConfigurations;
    std::map<std::string, cValueArray *> *distributions = nullptr;
    std::vector<std::string> streamStopRequested;

    cValueArray *pcpMapping = nullptr;

  private:
    void prepareChangesForProcessing(int initialized);
    void configureInitStreamsAndDistributions();
    cValueArray *convertToCValueArray(const std::vector<Mapping> &configMappings) const;
    cValueMap *convertMappingToCValue(const Mapping &mapping) const;
    void addEntryToStreamConfigurations(cValueMap *element, int i);
    void scheduleTimer(std::string source, cObject *details = nullptr);

  protected:
    virtual int numInitStages() const override { return 2; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    void subscribeForDynamicChanges();
    int classify(int pcp);

  public:
    void updateStreamConfigurations(cValueMap *element);
    void updateDistributions(std::string key, cValueArray *element);
    void addApplicationsWithStopReqToOutput(std::vector<cModule *> &sources);
    void computeConvolutionAndUpdateDistributions(cModule *source, cModule *target);

    std::map<std::string, cValueArray *> *getDistributions() const;
    cValueArray *getStreamConfigurations() const;

    inline bool isFixedPriority()
    {
        int numTrafficClasses = par("globallyNumTrafficClasses").intValue();
        return numTrafficClasses > 0;
    };

    ~ChangeMonitor() override;
};

} // namespace d6g

#endif

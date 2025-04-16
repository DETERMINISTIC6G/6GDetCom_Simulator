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

#include "../apps/dynamicsource/DynamicPacketSource.h"
#include "inet/linklayer/configurator/gatescheduling/base/GateScheduleConfiguratorBase.h"

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/common/scenario/ScenarioTimer_m.h"

using namespace omnetpp;
using namespace inet;

namespace d6g {

class DynamicScenarioObserver;

class ChangeMonitor : public cSimpleModule//inet::ClockUserModuleMixin<cSimpleModule>
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

    inline bool isFixedPriority() {
        int numTrafficClasses = par("globallyNumTrafficClasses").intValue();
        return numTrafficClasses > 0;
    };

    ~ChangeMonitor() override;
};

} // namespace d6g

#endif

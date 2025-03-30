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
//#include <zmq.hpp>

#include "../apps/dynamicsource/DynamicPacketSource.h"
#include "inet/linklayer/configurator/gatescheduling/base/GateScheduleConfiguratorBase.h"

//#include "inet/linklayer/configurator/gatescheduling/common/TSNschedGateScheduleConfigurator.h"
#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/common/scenario/ScenarioTimer_m.h"

using namespace omnetpp;
using namespace inet;
//using namespace inet::common;

namespace d6g {

class DynamicScenarioObserver;

class  ChangeMonitor : public  inet::ClockUserModuleMixin<cSimpleModule>{

protected:
    class Mapping {
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
        //std::string pathFragments;
        double reliability;
        //int policy;
        cValue phase;
        //int packetLoss;
        int objectiveType;
        //double weight;

        friend std::ostream& operator<<(std::ostream &os, const Mapping &mapping) {
            os << "name: " << mapping.name
               << ", source: " << mapping.source
               << ", pcp: "  << mapping.pcp
               << ", gateIndex: " << mapping.gateIndex
               << ", application: " << mapping.application
               << ", destination: " << mapping.destination;
            return os;
        }
    };

  private:
    DynamicScenarioObserver *observer;
    GateScheduleConfiguratorBase *gateScheduleConfigurator;

    ClockEvent *timer = nullptr;
    cPar *schedulerCallDelayParameter = nullptr;
    int flowIndex = 0;
    bool fixedPriority;

    //zmq::message_t testMsg;

    std::vector<Mapping> streamConfigurations;
    std::map<std::string, cValueArray*> *distributions = nullptr;
    std::vector<std::string> streamWantsToStop;

    cValueArray *pcpMapping = nullptr;

  private:
    void prepareChangesForProcessing(int initialized);
    void configureInitStreamsAndDistributions();
    cValueArray* convertToCValueArray(const std::vector<Mapping>& configMappings);
    cValueMap *convertMappingToCValue(const Mapping& mapping);
    void addEntryToStreamConfigurations(cValueMap *element, int i);
    void addEntriesToDistributionsFor(TsnTranslator *translator);

  protected:
    virtual int numInitStages() const override {return 2;}
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    void subscribeForDynamicChanges();
    int classify(int pcp);

  public:
    void updateStreamConfigurations(cValueMap* element);
    void updateDistributions(std::string key,  cValueArray* element);

    void addApplicationsWithStopReq(std::vector<cModule *> &sources);
    std::map<std::string, cValueArray*> *getDistributions();
    cValueArray* getStreamConfigurations();
    inline bool isFixedPriority() {return fixedPriority;}

    void scheduleTimer(std::string source, cObject *details=nullptr);
    void computeConvolution(cModule *source, cModule *target);

    ~ChangeMonitor() override;
};


} //namespace

#endif

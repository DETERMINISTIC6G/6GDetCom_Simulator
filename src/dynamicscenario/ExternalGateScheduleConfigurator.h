// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __DYNAMIC_SCENARIO_EXTERNALGATESCHEDULECONFIGURATOR_H_
#define __DYNAMIC_SCENARIO_EXTERNALGATESCHEDULECONFIGURATOR_H_

#include <omnetpp.h>

#include <filesystem>
#include <sstream>

#include "../dynamicscenario/ChangeMonitor.h"
#include "inet/linklayer/configurator/gatescheduling/common/TSNschedGateScheduleConfigurator.h"
#include "inet/queueing/gate/PeriodicGate.h"

using namespace omnetpp;
using namespace inet;

namespace d6g {

class ExternalGateScheduleConfigurator : public TSNschedGateScheduleConfigurator
{
    enum class DeviceType { END_DEVICE = 0, TSN_BRIDGE = 1, DS_TT = 2, NW_TT = 3, UNSPECIFIED = 4 };

    enum class DetComLinkType { DSTT_NWTT, NWTT_DSTT, DSTT_DSTT, NO_DETCOM_LINK };

    class Schedule : public Output::Schedule
    {
      public:
        cValueArray *durations;
        simtime_t offset;

      public:
        ~Schedule()
        {
            if (durations != nullptr) {
                delete durations;
            }
            durations = nullptr;
        }
    };

    class Application : public Input::Application
    {
      public:
        double reliability;
        simtime_t phase = 0;
        cValueMap *customParams = nullptr;

      public:
        ~Application()
        {
            if (customParams != nullptr)
                delete customParams;
        }
    };

    class Output : public GateScheduleConfiguratorBase::Output
    {
      public:
        std::map<Input::Application *, std::vector<simtime_t>> applicationStartTimesArray;
        std::vector<cModule *> appsInputAndWithStopReq;

      public:
        bool hasSchedule() { return gateSchedules.size(); }

      public:
        ~Output() { ; }
    };

    template <typename... Args> std::string format(const std::string &fmt, Args... args) const
    {
        size_t size = snprintf(nullptr, 0, fmt.c_str(), args...) + 1;
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, fmt.c_str(), args...);
        return std::string(buf.get(), buf.get() + size - 1);
    }

  private:
    std::map<std::string, uint16_t> *hashMapNodeId;
    ChangeMonitor *monitor = nullptr;

    std::string command;

    cPar *networkFilePar = nullptr;
    cPar *streamsFilePar = nullptr;
    cPar *histogramsFilePar = nullptr;
    cPar *configurationFilePar = nullptr;

    ClockEvent *configurationComputedEvent = nullptr;

    mutable simtime_t commitTime = 0;
    mutable simtime_t gateCycleDuration = 0;
    std::filesystem::path schedulerRoot;

  protected:
    /*extend GateScheduleConfiguratorBase*/
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual void clearConfiguration() override;
    virtual void addFlows(Input &input) const override;
    virtual void configureGateScheduling() override;
    virtual void configureApplicationOffsets() override;
    virtual Output *computeGateScheduling(const Input &input) const override;

    /*extend TSNschedGateScheduleConfigurator*/
    virtual Output *convertJsonToOutput(const Input &input, const cValueMap *json) const override;
    virtual void printJson(std::ostream &stream, const cValue &value, int level = 0) const;

    /*ExternalGateScheduleConfigurator*/
    virtual bool isDetComLink(cModule *source, cModule *target, DetComLinkType &detComLinkType) const;

    virtual inline std::string getExpandedNodeName(cModule *module) const;
    inline std::string getDetComLinkDescription(DetComLinkType type) const;
    inline short getDeviceType(cModule *mod) const;
    Input::Port *getConfigurablePort(const Input &input, std::string &linkName) const;
    Input::NetworkNode *findConfigurableNetworkNode(const Input &input, std::string source) const;

    void invokeScheduler() const;

  private:
    void inline write(std::string fileName, cValueMap *json) const;
    void writeDistributionsToFile() const;
    void writeStreamsToFile(const Input &input) const;
    void writeNetworkToFile(const Input &input) const;

    bool addEntryToPDBMap(cValueArray *pdb_map, cModule *source, cModule *target) const;
    void inline parseString(std::string s, std::string &leftString, std::string &rightString, char sep) const;
    /*Create separate JSON files for Streams and Network and Distributions */
    cValueMap *convertInputToJsonStreams(const Input &input) const;
    cValueMap *convertInputToJsonNetwork(const Input &input) const;
    cValueMap *convertJsonDevice(Input::NetworkNode *device) const;

    void deleteOldConfigurationPar();

  public:
    virtual void executeTSNsched(std::string fileName) = delete;
    ~ExternalGateScheduleConfigurator() override;
};

} // namespace d6g

#endif

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

#ifndef __DYNAMIC_SCENARIO_EXTERNALGATESCHEDULECONFIGURATOR_H_
#define __DYNAMIC_SCENARIO_EXTERNALGATESCHEDULECONFIGURATOR_H_

#include <filesystem>
#include <omnetpp.h>

#include <sstream>

#include "ChangeMonitor.h"
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
        ~Application() {
            if (customParams != nullptr)
                delete customParams;
        }
    };

    class Output : public GateScheduleConfiguratorBase::Output
    {
      public:
        std::map<Input::Application *, std::vector<simtime_t>> applicationStartTimesArray;
        std::vector<cModule *> appsInputAndWithStopReq;
        std::map<cModule*, std::vector<cValueArray *>> psfpSchedules; // maps flow to schedules per psfp

      public:
        bool hasSchedule() { return gateSchedules.size(); }

      public:
        ~Output()
        {
            ;
        }
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
    int getPsfpGate(cValueMap *classifierMap, cValueArray *decoderMap, int pcp) const;

    void invokeScheduler() const;
    virtual void configurePsfpGateScheduling();

  private:
    void inline write(std::string fileName, cValueMap *json) const;
    void writeDistributionsToFile() const;
    void writeStreamsToFile(const Input &input) const;
    void writeNetworkToFile(const Input &input) const;

    bool addEntryToPDBMap(cValueArray *pdb_map, cModule *source, cModule *target) const;
    void parseString(std::string s, std::string &leftString, std::string &rightString, char sep)  const;
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

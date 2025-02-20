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

#include <omnetpp.h>
#include "inet/linklayer/configurator/gatescheduling/common/TSNschedGateScheduleConfigurator.h"

using namespace omnetpp;

using namespace inet;
//using namespace inet::common;

namespace d6g {

/**
 * TODO - Generated class
 */
class ExternalGateScheduleConfigurator : public TSNschedGateScheduleConfigurator
{
    enum class DeviceType {
        END_DEVICE = 0, TSN_BRIDGE = 1, DS_TT = 2, NW_TT = 3, UNSPECIFIED = 4
    };

    enum class DetComLinkType {
        DSTT_NWTT, NWTT_DSTT, DSTT_DSTT, NO_DETCOM_LINK
    };





private:
    std::map<std::string, cValueArray*> *distribution;
    std::map<std::string, uint16_t> *hashMap;

    std::string fileNameStreams;
    std::string fileNameNetwork;
    std::string fileNameDistribution;

    std::string outputFileName;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

    /*GateScheduleConfiguratorBase*/
    virtual void addFlows(Input& input) const override;


    /*ExternalGateScheduleConfigurator*/
    virtual void printJson(std::ostream& stream, const cValue& value, int level = 0) const;
    virtual bool isDetComLink(cModule *source, cModule *target, DetComLinkType &detComLinkType) const;

    /*Create separate JSON files for Streams and Network and Distributions */
    virtual cValueMap *convertInputToJsonStreams(const Input& input) const;
    virtual cValueMap *convertInputToJsonNetwork(const Input& input) const;
    virtual void writeDistributionsToFile() const;
    virtual void writeStreamsToFile(const Input& input) const;
    virtual void writeNetworkToFile(const Input& input) const;

    std::string getDescription(DetComLinkType type) const;
    std::string expandNodeName(cModule *module) const;


    /* TSNschedGateScheduleConfigurator*/
    virtual void executeTSNsched(std::string fileName)  const override;
    virtual Output *computeGateScheduling(const Input& input) const override;
    virtual short getSwitchType(cModule* mod) const;


  private:
    void write(std::string fileName, cValueMap *json) const;
    void generateNetworkNodesHashMap(const std::vector<std::string>& strings) const;
    void setReliabilityAndPolicyToPDBMapEntry(cValueArray *pdb_map, std::string name) const;
    void addEntryToPDBMap(cValueArray *pdb_map, DetComLinkType linkType, std::string nameNetworkNode, std::string nameNextNetworkNode) const;
    double computeGuardBand(const Input &input) const;

  public:
    ~ExternalGateScheduleConfigurator() override;

};

} //namespace

#endif

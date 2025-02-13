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


private:
    std::map<std::string, cValueArray*> *distribution;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

    /*GateScheduleConfiguratorBase*/
    virtual void addFlows(Input& input) const override;


    /*ExternalGateScheduleConfigurator*/
    virtual void printJson(std::ostream& stream, const cValue& value, int level = 0) const;
    virtual bool isDetComLink(cModule *source, cModule *target) const;
    virtual bool hasDistribution(std::string key, std::string &hist) const;
    /*
     * Create separate JSON files for Streams and Network and Distributions
     * */
    virtual void writeInputToFile(const Input& input, std::string fileNameStreams, std::string fileNameNetwork) const;
    virtual cValueMap *convertInputToJsonStreams(const Input& input) const;
    virtual cValueMap *convertInputToJsonNetwork(const Input& input) const;
    virtual void writeDistributionsToFile(std::string fileName) const;


    /* TSNschedGateScheduleConfigurator*/
    virtual void executeTSNsched(std::string fileName)  const override;

    virtual Output *computeGateScheduling(const Input& input) const override;

  private:
    virtual void write(std::string fileName, cValueMap *json) const;

  public:
    ~ExternalGateScheduleConfigurator() override;

};

} //namespace

#endif

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

#ifndef DEVICES_TSNTRANSLATOR_TSNTRANSLATOR_H_
#define DEVICES_TSNTRANSLATOR_TSNTRANSLATOR_H_

#include <omnetpp/cmodule.h>
#include <omnetpp/ccomponent.h>

#include "../../dynamicscenario/DynamicScenarioObserver.h"

namespace d6g {
using namespace omnetpp;

class TsnTranslator : public cModule {

private :
    cMessage *distributionChangeEvent = nullptr;

protected:
    virtual void initialize() override;
    virtual void handleParameterChange(const char *name) override;

public:
    virtual cDynamicExpression* getDistributionExpression(const char *delay);
    ~TsnTranslator() override;
};


} /* namespace d6g */

#endif /* DEVICES_TSNTRANSLATOR_TSNTRANSLATOR_H_ */

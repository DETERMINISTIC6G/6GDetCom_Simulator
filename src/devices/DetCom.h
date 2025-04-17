// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DEVICES_DETCOM_H_
#define DEVICES_DETCOM_H_

#include <omnetpp/cmodule.h>
#include "inet/common/InitStages.h"
#include "inet/common/InitStageRegistry.h"
#include "inet/networklayer/configurator/base/L3NetworkConfiguratorBase.h"

namespace d6g {
using namespace omnetpp;
using namespace inet;

class DetCom : public cModule {
public:
    int numInitStages() const override { return NUM_INIT_STAGES; }
protected:
    void initialize(int stage) override;

    cModule *getDstt(cXMLElement *delayEntity) const;


};

} /* namespace d6g */

#endif /* DEVICES_DETCOM_H_ */

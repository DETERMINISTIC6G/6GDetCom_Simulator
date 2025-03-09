// This file is part of Deliverable D4.4 [D44PLACEHOLDER]
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __DETERMINISTIC6G_RESIDENCETIMECALCULATOR_H_
#define __DETERMINISTIC6G_RESIDENCETIMECALCULATOR_H_

#include <omnetpp.h>
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFlowBase.h"
#include "inet/common/clock/ClockUserModuleMixin.h"

using namespace omnetpp;

namespace d6g {
using namespace inet;
using namespace inet::queueing;
/**
 * TODO - Generated class
 */
class ResidenceTimeCalculator : public ClockUserModuleMixin<PacketFlowBase>, public TransparentProtocolRegistrationListener
{
  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
    virtual cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} //namespace

#endif

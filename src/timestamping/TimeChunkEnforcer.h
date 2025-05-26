// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __DETERMINISTIC6G_TIMECHUNKENFORCER_H_
#define __DETERMINISTIC6G_TIMECHUNKENFORCER_H_

#include <omnetpp.h>

#include "../utils/InterfaceFilterMixin.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFlowBase.h"

using namespace omnetpp;

namespace d6g {
using namespace inet;
using namespace inet::queueing;

/**
 * TODO - Generated class
 */
class TimeChunkEnforcer : public InterfaceFilterMixin<PacketFlowBase>, public TransparentProtocolRegistrationListener
{
  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
    cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} // namespace d6g

#endif

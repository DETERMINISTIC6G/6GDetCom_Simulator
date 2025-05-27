// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


#ifndef __DETERMINISTIC6G_TTDDELAYERQUEUE_H
#define __DETERMINISTIC6G_TTDDELAYERQUEUE_H

#include "inet/queueing/queue/PacketQueue.h"
#include "inet/common/clock/ClockUserModuleMixin.h"

namespace d6g {
using namespace inet;
using namespace queueing;

class TTDelayerQueue : public ClockUserModuleMixin<PacketQueue>
{
  public:
    virtual void pushPacket(Packet *packet, const cGate *gate) override;
};
} // namespace d6g

#endif


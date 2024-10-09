//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


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


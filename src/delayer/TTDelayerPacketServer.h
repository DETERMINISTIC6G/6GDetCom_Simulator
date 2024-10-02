//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __DETERMINISTIC6G_DELAYERTTDELAYERSERVER_H
#define __DETERMINISTIC6G_DELAYERTTDELAYERSERVER_H

#include "../utils/InterfaceFilterMixin.h"
#include "inet/queueing/server/PacketServer.h"

namespace d6g {
using namespace inet;
using namespace queueing;

class TTDelayerPacketServer : public InterfaceFilterMixin<PacketServer>
{
  protected:
    cPar *delayParameter;

  protected:
    virtual void initialize(int stage) override;
    virtual void scheduleProcessingTimer() override;
    void setDelay(cPar *delay);
    clocktime_t computeDelay(Packet *packet) const;

};
} // namespace inet

#endif


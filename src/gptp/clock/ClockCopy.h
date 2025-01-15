//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __D6G_CLOCKCOPY_H
#define __D6G_CLOCKCOPY_H

#include "inet/common/clock/ClockUserModuleBase.h"

namespace d6g {

using namespace inet;

class ClockCopy : public ClockUserModuleBase, public virtual IClock, public cListener
{
  protected:
    virtual void initialize(int stage) override;

  public:
    virtual clocktime_t getClockTime() const override { return clock->getClockTime(); };
    virtual clocktime_t computeClockTimeFromSimTime(simtime_t time) const override { return clock->computeClockTimeFromSimTime(time); };
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t time) const override { return clock->computeSimTimeFromClockTime(time); };
    virtual void scheduleClockEventAt(clocktime_t time, ClockEvent *event) override { clock->scheduleClockEventAt(time, event); };
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) override { clock->scheduleClockEventAfter(delay, event); };
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override { return clock->cancelClockEvent(event); };
    virtual void handleClockEvent(ClockEvent *event) override { clock->handleClockEvent(event); };
    virtual int numInitStages() const override { return NUM_INIT_STAGES; };

    virtual void receiveSignal(cComponent *source, int signal, const simtime_t& time, cObject *details) override;
    virtual void receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details) override;
};

} // namespace d6g

#endif

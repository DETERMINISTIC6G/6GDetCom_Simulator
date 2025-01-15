//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "ClockCopy.h"

#include "inet/clock/base/ClockBase.h"

namespace d6g {

Define_Module(ClockCopy);

void ClockCopy::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        auto clockComponent = check_and_cast<cComponent *>(clock.get());
        clockComponent->subscribe(ClockBase::timeChangedSignal, this);
        clockComponent->subscribe(ClockBase::timeJumpedSignal, this);
    }
}

void ClockCopy::receiveSignal(cComponent *source, int signal, const simtime_t &time, cObject *details)
{
    if (signal == ClockBase::timeChangedSignal) {
        if (check_and_cast<IClock *>(source) == clock.get())
            emit(ClockBase::timeChangedSignal, time, details);
        else
            throw cRuntimeError("Signal from unknown source %s", source->getFullPath().c_str());
    }
}

void ClockCopy::receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details)
{
    if (signal == ClockBase::timeJumpedSignal) {
        if (check_and_cast<IClock *>(source) == clock.get())
            emit(ClockBase::timeJumpedSignal, this, details);
        else
            throw cRuntimeError("Signal from unknown source %s", source->getFullPath().c_str());
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace d6g

// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "TTDelayer.h"

namespace d6g {
Define_Module(TTDelayer);

void TTDelayer::initialize(int stage)
{
    InterfaceFilterMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        setDelay(&par("delay"));
    }
}

clocktime_t TTDelayer::computeDelay(Packet *packet) const
{
    clocktime_t delay;
    if (!matchesInterfaceConfiguration(packet)) {
        delay = 0;
    } else {
        delay = delayParameter->doubleValue();
    }

    return delay;
}

void TTDelayer::setDelay(cPar *delay) { delayParameter = delay; }

void TTDelayer::handleParameterChange(const char *parname)
{
    if (!strcmp(parname, "delay")) {
        setDelay(&par("delay"));
    }
}
} // namespace d6g

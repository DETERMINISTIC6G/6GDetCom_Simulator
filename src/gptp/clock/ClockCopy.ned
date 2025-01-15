//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package d6g.gptp.clock;

import inet.clock.contract.IClock;

//
// This module is always in sync with the clock it copys.
// Useful inside a MultiClock, e.g. to mimic the behavior the detComClock inside a TsnTranslator.
//
//
simple ClockCopy like IClock
{
    parameters:
        string clockModule = default("");
        @signal[timeChanged](type=simtime_t);
        @signal[timeJumped]();
}


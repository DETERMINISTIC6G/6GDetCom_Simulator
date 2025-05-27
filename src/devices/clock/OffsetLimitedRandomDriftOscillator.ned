// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


package d6g.devices.clock;

import inet.clock.oscillator.RandomDriftOscillator;

//
// This module behaves like the RandomDriftOscillator but ensures that the drift does not exceed a certain limit
// from the simulation time.
//
simple OffsetLimitedRandomDriftOscillator extends RandomDriftOscillator
{
    parameters:
        @class(OffsetLimitedRandomDriftOscillator);
        double maxOffset @unit(s) = default(0s); // Max offset to simulation time until the drift is adjusted
        double maxDriftRateAdjustment @unit(ppm) = default(0ppm); // Max drift rate adjustment per changeInterval
}

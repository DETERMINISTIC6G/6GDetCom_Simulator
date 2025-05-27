// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "OffsetLimitedRandomDriftOscillator.h"

namespace d6g {
using namespace inet;

Define_Module(OffsetLimitedRandomDriftOscillator);

void OffsetLimitedRandomDriftOscillator::initialize(int stage)
{
    RandomDriftOscillator::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        maxOffsetParameter = &par("maxOffset");
        maxDriftRateAdjustment = ppm(par("maxDriftRateAdjustment"));
        // Get my clock which is my parent
        clock = dynamic_cast<OscillatorBasedClock *>(getParentModule());
    }
}

void OffsetLimitedRandomDriftOscillator::handleMessage(cMessage *message)
{
    if (message == changeTimer) {
        auto maxOffset = maxOffsetParameter->doubleValue();
        auto changeInterval = SimTime(changeIntervalParameter->doubleValue());
        auto changeIntervalClockTime = SIMTIME_AS_CLOCKTIME(changeInterval);

        auto currSimTimeAsClockTime = SIMTIME_AS_CLOCKTIME(simTime());
        auto currClockTime = clock->getClockTime();
        auto currOffsetToSimTime = currClockTime - currSimTimeAsClockTime;

        clocktime_t offsetToMaxOffset;
        if (currOffsetToSimTime > 0) {
            offsetToMaxOffset = maxOffset - currOffsetToSimTime;
        }
        else {
            offsetToMaxOffset = maxOffset + currOffsetToSimTime;
        }

        auto driftRateInSecPerSec = std::abs(driftRate.get() / 1e6);
        ClockTime timeToReachMaxOffset;
        if (driftRateInSecPerSec == 0) {
            timeToReachMaxOffset = ClockTime::getMaxTime();
        } else {
            timeToReachMaxOffset = offsetToMaxOffset / driftRateInSecPerSec;
        }

        auto intervalsToReachMaxOffset = timeToReachMaxOffset / changeIntervalClockTime;
        auto intervalsToBringDriftRateToZero = std::abs((driftRate / maxDriftRateAdjustment).get());

        ppm driftRateAdjustmentNow;
        if (currOffsetToSimTime > 0) {
            driftRateAdjustmentNow = -maxDriftRateAdjustment;
        }
        else {
            driftRateAdjustmentNow = maxDriftRateAdjustment;
        }

        ppm newDriftRate;
        bool movesAway = currOffsetToSimTime > 0 && driftRate < ppm(0) || currOffsetToSimTime < 0 && driftRate > ppm(0);
        if ((!movesAway && intervalsToBringDriftRateToZero >= intervalsToReachMaxOffset) || offsetToMaxOffset < 0) {
            driftRateChangeTotal += driftRateAdjustmentNow;
            newDriftRate = initialDriftRate + driftRateChangeTotal;
            if (currOffsetToSimTime > 0 && newDriftRate < driftRateAdjustmentNow) {
                newDriftRate = driftRateAdjustmentNow;
                driftRateChangeTotal = newDriftRate - initialDriftRate;
            } else if (currOffsetToSimTime < 0 && newDriftRate > driftRateAdjustmentNow) {
                newDriftRate = driftRateAdjustmentNow;
                driftRateChangeTotal = newDriftRate - initialDriftRate;
            }
        }
        else {
            driftRateChangeTotal += ppm(driftRateChangeParameter->doubleValue());
            driftRateChangeTotal = std::max(driftRateChangeTotal, driftRateChangeLowerLimit);
            driftRateChangeTotal = std::min(driftRateChangeTotal, driftRateChangeUpperLimit);
            newDriftRate = initialDriftRate + driftRateChangeTotal;
        }

        setDriftRate(newDriftRate);

        scheduleAfter(changeInterval, changeTimer);
    }
    else
        throw cRuntimeError("Unknown message");
}

} // namespace d6g

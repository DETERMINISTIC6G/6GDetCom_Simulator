// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DelayReplayer.h"

#include <inet/common/Units.h>
#include <omnetpp.h>

#include <algorithm>

#include "fstream"
#include "inet/common/XMLUtils.h"

namespace d6g {
Define_Module(DelayReplayer);

void DelayReplayer::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        const char *csvfile = par("csvFilename").stringValue();
        offset = par("offset");
        timestampOffset = par("timestampOffset").doubleValue();
        readCSV(csvfile);
        if (mode == CYCLE && offset > 0 && !delays.empty()) {
            int offsetToApply = offset % delays.size(); // Ensure offset doesn't exceed vector size
            std::advance(delayIterator, offsetToApply);
        }
    }
}

void DelayReplayer::setOffset(cValue &offset)
{
    delayIterator = delays.begin();
    this->offset = offset;
    int offsetToApply = this->offset % delays.size(); // Ensure offset doesn't exceed vector size
    std::advance(delayIterator, offsetToApply);
}

void DelayReplayer::setTimestampOffset(cValue &timestampOffset)
{
    this->timestampOffset = timestampOffset.doubleValueInUnit("s");
}

void DelayReplayer::readCSV(const char *filename)
{
    delays.clear();
    std::string line;
    std::ifstream infile(filename);
    cDynamicExpression delayExpr, startExpr;
    std::string delayTimeStr, startTimeStr;
    cValue delayValue, startValue;

    if (!infile) {
        throw cRuntimeError("File '%s' not found", filename);
    }
    while (getline(infile, line)) {
        if (mode == NONE) {
            if (line.find(',') != std::string::npos) {
                mode = TIME_BASED;
            }
            else {
                mode = CYCLE;
            }
        }

        if (mode == CYCLE) {
            delayExpr.parse(line.c_str());
            delayValue = delayExpr.evaluate(this);
            startValue = cValue(-1, "s");
            delays.emplace_back(startValue, delayValue);
        }
        else if (mode == TIME_BASED) {
            std::stringstream str(line);
            getline(str, delayTimeStr, ',');
            getline(str, startTimeStr, ',');

            delayExpr.parse(delayTimeStr.c_str());
            delayValue = delayExpr.evaluate(this);
            startExpr.parse(startTimeStr.c_str());
            startValue = startExpr.evaluate(this);

            delays.emplace_back(startValue, delayValue);
        }
    }
    delayIterator = delays.begin();
}

cValue DelayReplayer::getRand()
{
    if (mode == CYCLE) {
        if (delayIterator == delays.end()) {
            delayIterator = delays.begin();
        }
        cValue value = delayIterator->delayTime;
        ++delayIterator;
        return value;
    }
    else if (mode == TIME_BASED) {
        // Add timestampOffset to current simulation time
        auto value = getDelayFromTargetValue(simTime().dbl() + timestampOffset);
        return value;
    }
    else {
        throw cRuntimeError("No operation mode selected");
    }
}

cValue DelayReplayer::getDelayFromTargetValue(double targetTime) const
{
    // binary search
    auto it = std::upper_bound(delays.begin(), delays.end(), targetTime, [](double tTime, const DelayEntry &entry) {
        return tTime < entry.startTime.doubleValueInUnit("s");
    });

    if (it == delays.begin()) {
        throw cRuntimeError("Target time is before the start of any delay period");
    }

    --it; // Move iterator to the correct interval

    if (targetTime >= it->startTime.doubleValueInUnit("s")) {
        return it->delayTime;
    }
    else {
        throw cRuntimeError("No suitable delay interval found for the target time");
    }
}
} /* namespace d6g */

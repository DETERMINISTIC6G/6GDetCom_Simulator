// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "DelayReplayer.h"
#include <algorithm>
#include <omnetpp.h>
#include "fstream"
#include "inet/common/XMLUtils.h"

namespace d6g {

Define_Module(DelayReplayer);

void DelayReplayer::initialize(int stage) {
    if (stage == INITSTAGE_LOCAL) {
        delays.clear();
        const char *csvfile = par("csvFilename").stringValue();
        readCSV(csvfile);
        delayIterator = delays.begin();
    }
}

void DelayReplayer::readCSV(const char *filename) {
    std::string line;
    std::ifstream infile(filename);
    cDynamicExpression delayExpr, startExpr;
    std::string delayTimeStr, startTimeStr;
    cValue delayValue, startValue;

    if (!infile) {
        throw cRuntimeError("File '%s' not found", filename);
    }
    while (getline(infile, line)) {
        std::stringstream str(line);
        getline(str, delayTimeStr, ',');
        getline(str, startTimeStr, ',');

        delayExpr.parse(delayTimeStr.c_str());
        delayValue = delayExpr.evaluate(this);
        startExpr.parse(startTimeStr.c_str());
        startValue = startExpr.evaluate(this);

        delays.push_back(DelayEntry(startValue, delayValue));
    }
    delayIterator = delays.begin();
    for (const auto &delay : delays) {
        EV << "Start time: " << delay.startTime.str() << ", Delay time: "
                << delay.delayTime.str() << endl;
    }
    selfCheck();
}

cValue DelayReplayer::getRand() {
    if (delayIterator == delays.end()) {
        delayIterator = delays.begin();
    }
    cValue value = delayIterator->delayTime;
    EV << value.str() << endl;
    ++delayIterator;
    return value;
}

cValue DelayReplayer::getDelayFromTargetValue(double targetTime) const{
    // binary search
    auto it = std::upper_bound(delays.begin(), delays.end(), targetTime,
            [](double tTime, const DelayEntry& entry) {
                return tTime < entry.startTime.doubleValueInUnit("s");
            });

    if (it == delays.begin()) {
        throw cRuntimeError(
                "Target time is before the start of any delay period");
    }

    --it; // Move iterator to the correct interval

    if (targetTime >= it->startTime.doubleValueInUnit("s")) {
        return it->delayTime;
    } else {
        throw cRuntimeError(
                "No suitable delay interval found for the target time");
    }
}

void DelayReplayer::selfCheck() const {
    EV << "DelayReplayer self check" << endl;

    double startTime = delays.front().startTime.doubleValueInUnit("s");
    double timeIncrement = 1.0; // 1 second increment
    double endTime = 15.0; // End time for self-check

    for (double time = startTime; time <= endTime; time += timeIncrement) {
        try {
            cValue delay = getDelayFromTargetValue(time);
            EV << "Target: " << std::to_string(time) << "s  --  Delay: " << delay.str()
                    << endl;
        } catch (const cRuntimeError &e) {
            throw cRuntimeError("Self check failed for target %s: %s",
                    std::to_string(time).c_str(), e.what());
        }
    }
    EV << "DelayReplayer self check completed successfully" << endl;
}

DelayReplayer::DelayReplayer() {
}

DelayReplayer::~DelayReplayer() {
}

} /* namespace d6g */

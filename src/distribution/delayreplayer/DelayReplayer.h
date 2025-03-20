// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DISTRIBUTION_DELAYREPLAYER_DELAYREPLAYER_H_
#define DISTRIBUTION_DELAYREPLAYER_DELAYREPLAYER_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "../contract/IRandomNumberProvider.h"

using namespace omnetpp;

namespace d6g {
using namespace inet;

/**
 * DelayReplayer is a module that provides a random number based on a CSV file.
 * For more information about the module, see the NED documentation.
 */
class INET_API DelayReplayer : public cSimpleModule, public IRandomNumberProvider{
    /**
     * DelayEntry is a class that represents a delay in the CSV file.
     */
public:
    class INET_API DelayEntry{
    public:
        cValue startTime; ///<start time
        cValue delayTime; ///<delay time

        DelayEntry(const cValue& start, const cValue& delay)
            : startTime(start), delayTime(delay) {}

    };
private:
    std::vector<DelayEntry> delays; ///< Vector to store the delays
    std::vector<DelayEntry>::iterator delayIterator; ///< Iterator to traverse through the delays.

protected:
    virtual void initialize(int stage) override;

public:
    /*!
     * Read delays from the CSV file
     *
     * @param filename the name of the file to read the delays from.
     */
    void readCSV(const char* filename);

    /*!
     * Get a random number from the delayreplayer.
     *
     * @return random delay time from the delayreplayer
     */
    cValue getRand() override;

    /*!
     * Get the target's delay time from the delayreplayer.
     *
     * @param targetTime the target time to get the delay time for.
     *
     * @return the target's delay time from the delayreplayer
     */
    cValue getDelayFromTargetValue(double targetTime) const;

    DelayReplayer();
    virtual ~DelayReplayer();

private:
    /*!
     * Self check to make sure that the delayreplayer is correct
     *
     * This was used for debugging purposes, not needed anymore hopefully but keeping it just in case.
     */
    void selfCheck() const;
};

} /* namespace d6g */

#endif /* DISTRIBUTION_DELAYREPLAYER_DELAYREPLAYER_H_ */

// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DETERMINISTIC6G_HISTOGRAM_H_
#define DETERMINISTIC6G_HISTOGRAM_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "../contract/IRandomNumberProvider.h"

using namespace omnetpp;

namespace d6g {
using namespace inet;

/**
 * Histogram is a module that provides a random number based on a histogram configuration.
 * The histogram configuration is provided in the NED file as a parameter.
 * For more information on the histogram configuration, see the NED doc.
 */
class Histogram : public cSimpleModule, public IRandomNumberProvider {
    /**
     * BinEntry is a class that represents a bin in the histogram.
     */
public:
    class BinEntry {
    public:
        explicit BinEntry(cXMLElement *binEntity, cModule *context = nullptr);

        // Left boundary
        cValue leftBoundary = -INFINITY; ///< Left boundary
        cValue rightBoundary = INFINITY; ///< Right boundary
        int count = 0; ///< Count of elements in the bin
        int accumulatedCount = 0; ///< Accumulated count from beginning of the histogram including this bin
    };

private:
    std::vector<BinEntry *> bins; ///< Vector to store the bins
    int totalCount = 0; ///< Total count of elements in all bins

protected:
    void initialize(int stage) override;
public:
    void parseHistogramConfig(cXMLElement *histogramEntity);

public:
    /*!
     * Get total count of elements in all bins
     *
     * @return total count of elements in all bins
     */
    int getTotalCount() const;

    /*!
     * Get number of bins
     *
     * @return number of bins
     */
    size_t getNumberBins() const;

    /*!
     * Get a random bin, based on the number of elements in each bin
     *
     * @return random bin
     */
    BinEntry *randomBin() const;

    /*!
     * Get a random number from the histogram.
     * This function uses the randomBin() function to get a random bin, and then returns a random number from that bin's range.
     *
     * @return random number from the histogram
     */
    cValue getRand() override;

    BinEntry *getBinFromTargetValue(int target) const;

private:
    /*!
     * Self check to make sure that the histogram is correct
     *
     * This was used for debugging purposes, not needed anymore hopefully but keeping it just in case.
     */
    void selfCheck() const;
};

} //namespace

#endif

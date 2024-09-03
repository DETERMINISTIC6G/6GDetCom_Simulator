// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "Histogram.h"
#include "inet/common/XMLUtils.h"
#include <algorithm>

namespace d6g {

using namespace inet::xmlutils;

/// Test for D6G
Define_Module(Histogram);
void Histogram::initialize(int stage) {
    // Parse the histogram configuration
    if (stage == INITSTAGE_LOCAL) {
        cXMLElement *histogramEntity = par("histogramConfig").xmlValue();
        parseHistogramConfig(histogramEntity);
    }
}

void Histogram::parseHistogramConfig(cXMLElement *histogramEntity) {
    if (strcmp(histogramEntity->getTagName(), "histogram") != 0) {
        throw cRuntimeError("Cannot read histogram configuration, unaccepted '%s' entity at %s",
                            histogramEntity->getTagName(),
                            histogramEntity->getSourceLocation());
    }

    cXMLElementList binEntities = histogramEntity->getChildrenByTagName("bin");
    BinEntry *previousBinEntry = nullptr;

    for (auto &binEntity: binEntities) {
        auto *currentBinEntry = new BinEntry(binEntity, this);

        totalCount += currentBinEntry->count;
        currentBinEntry->accumulatedCount = totalCount;

        if (previousBinEntry != nullptr) {
            previousBinEntry->rightBoundary = currentBinEntry->leftBoundary;
        }

        // add to vector
        bins.push_back(currentBinEntry);

        previousBinEntry = currentBinEntry;
    }

    if (previousBinEntry != nullptr) {
        previousBinEntry->rightBoundary = std::numeric_limits<double>::infinity();
    }

    // Print the bins
    for (const auto *bin: bins) {
        EV << "Left boundary: " << bin->leftBoundary.str()
           << "  --  Right boundary: " << bin->rightBoundary.str()
           << "  --  Count: " << bin->count
           << "  --  Accumulated count: " << bin->accumulatedCount << endl;
    }
}

/**
 * Self check to make sure that the histogram is correct
 *
 * This was used for debugging purposes, not needed anymore hopefully but keeping it just in case.
 */
void Histogram::selfCheck() const {
    EV << "Histogram self check" << endl;
    for (int i = 0; i < totalCount; i++) {
        auto bin = getBinFromTargetValue(i);
        /*EV << "Target: " << i << "  --  Bin: " << bin->leftBoundary.str() << " - " << bin->rightBoundary.str() << " - "
           << bin->accumulatedCount << endl;*/
        if (bin->accumulatedCount <= i) {
            throw cRuntimeError("Histogram self check failed for target %d, accumulatedCount: %d", i,
                                bin->accumulatedCount);
        }
    }
}

int Histogram::getTotalCount() const {
    return totalCount;
}

size_t Histogram::getNumberBins() const {
    return bins.size();
}

Histogram::BinEntry *Histogram::randomBin() const {
    if (totalCount == 0) {
        throw cRuntimeError("No bins to select from");
    }

    auto target = intrand(totalCount);
    auto bin = getBinFromTargetValue(target);
    EV_DEBUG << "Random bin: " << bin->leftBoundary.str() << " - " << bin->rightBoundary.str() << " - "
             << bin->accumulatedCount << endl;
    return bin;
}

Histogram::BinEntry *Histogram::getBinFromTargetValue(int target) const {
    // Perform binary search to find the bin that contains the target
    auto bin = std::upper_bound(bins.begin(), bins.end(), target,
                                [](int value, const BinEntry *bin) {
                                    return value < bin->accumulatedCount;
                                });

    if (bin != bins.end()) {
        return *bin;
    } else {
        throw cRuntimeError(
                "random number greater than actual total count. This should never happen, check your totalCount calculation");
    }
}

cValue Histogram::getRand() {
    auto bin = randomBin();
    auto unit = bin->leftBoundary.getUnit();
    auto leftBoundaryDbl = bin->leftBoundary.doubleValueInUnit(unit);
    auto rightBoundaryDbl = bin->rightBoundary.doubleValueInUnit(unit);
    auto rnd = uniform(leftBoundaryDbl, rightBoundaryDbl);
    return cValue(rnd, unit);
}

Histogram::BinEntry::BinEntry(cXMLElement *binEntity, cModule *context) {
    const char *lowAttr = binEntity->getAttribute("low");
    if (!lowAttr) {
        throw cRuntimeError("Bin entry without low attribute at %s", binEntity->getSourceLocation());
    }
    cDynamicExpression lowerBoundPar = cDynamicExpression();
    lowerBoundPar.parse(lowAttr);
    this->leftBoundary = lowerBoundPar.evaluate(context);
    this->count = atoi(binEntity->getNodeValue());
}

} //namespace

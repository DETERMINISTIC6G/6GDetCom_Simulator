//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Histogram.h"
#include "inet/common/XMLUtils.h"
#include "inet/networklayer/configurator/base/L3NetworkConfiguratorBase.h"

namespace d6g {

using namespace inet::xmlutils;

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

    // Check the bins
    for (const auto *bin: bins) {
        EV << "Left boundary: " << bin->leftBoundary.str()
           << "  --  Right boundary: " << bin->rightBoundary.str()
           << "  --  Count: " << bin->count << endl;
    }

    // Testing
    EV << "Random 1: " << getRand().str() << endl;
    EV << "Random 2: " << getRand().str() << endl;
    EV << "Random 3: " << getRand().str() << endl;
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

    // Generate a random number between 0 and totalCount
    auto randomValue = intrand(totalCount);

    //Binary Search
    return BinarySearch(randomValue);
}

Histogram::BinEntry *Histogram::BinarySearch(int target) const {
    int low = 0;
    int high = bins.size() - 1;

    while (low <= high) {
        int mid = low + ((high - low) / 2);
        if (target == bins[mid]->accumulatedCount){
            return bins[mid];
        }
        else if (target < bins[mid]->accumulatedCount){
            high = mid - 1;
        }
        else{
            low = mid + 1;
        }
    }

    if (low < bins.size()) {
        return bins[low];
    }

    throw cRuntimeError("random number greater than actual total count. This should never happen, check your totalCount calculation");
}


cValue Histogram::getRand() const {
    auto bin = randomBin();
    auto unit = bin->leftBoundary.getUnit();
    auto leftBoundaryDbl = bin->leftBoundary.doubleValueInUnit(unit);
    auto rightBoundaryDbl = bin->rightBoundary.doubleValueInUnit(unit);
    auto rnd = uniform(leftBoundaryDbl, rightBoundaryDbl);
    return cValue(rnd, unit);
}

Histogram::BinEntry::BinEntry(cXMLElement *binEntity, cModule *context) {
    const char *lowAttr = binEntity->getAttribute("low");
    cDynamicExpression lowerBoundPar = cDynamicExpression();
    lowerBoundPar.parse(lowAttr);
    this->leftBoundary = lowerBoundPar.evaluate(context);
    this->count = atoi(binEntity->getNodeValue());
}

} //namespace

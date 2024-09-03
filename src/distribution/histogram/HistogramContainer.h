// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DETERMINISTIC6G_HISTOGRAMCONTAINER_H_
#define DETERMINISTIC6G_HISTOGRAMCONTAINER_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "../contract/IRandomNumberProvider.h"
#include "../histogram/Histogram.h"

using namespace omnetpp;

namespace d6g {
using namespace inet;

/**
 * The HistogramContainer class serves as a container for multiple Histogram instances,
 * allowing for management and access to various histograms using keys (typically their names).
 * This class is particularly useful in simulations of the real-world scenarios,
 * enabling the representation of complex or varied statistical patterns.
 *
 * For detailed usage and configuration, refer to the NED doc.
 */
class HistogramContainer : public cSimpleModule, public IRandomNumberProvider {

private:
    std::map<std::string, Histogram *> histograms; ///< Map associating string keys with Histogram instances.


protected:
    virtual void initialize(int stage) override;

    /*!
     * Create and load a Histogram instance from a file.
     *
     * @param fileName the name of the file containing the Histogram data.
     *
     * @return Histogram data according to the XML file.
     */
    static Histogram *loadHistogramFromFile(const char *fileName);
public:
    /*!
     * Retrieves a Histogram instance based on the given key.
     *
     * @param key the key of the Histogram instance to be retrieved.
     *
     * @return the Histogram instance based on the given key.
     */
    Histogram *getHistogram(const std::string &key) const;

    /*!
     * Get a random number from the specific Histogram.
     *
     * @param key the key of the Histogram instance to be retrieved.
     *
     * @return random number from the specific Histogram.
     */
    cValue getRand(const std::string &key) override;

    // Destructor
    ~HistogramContainer() override;
};

} /* namespace d6g */

#endif /* DETERMINISTIC6G_HISTOGRAMCONTAINER_H_ */

// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DISTRIBUTION_DELAYREPLAYER_DELAYREPLAYERCONTAINER_H_
#define DISTRIBUTION_DELAYREPLAYER_DELAYREPLAYERCONTAINER_H_

#include <omnetpp.h>
#include "inet/common/INETDefs.h"
#include "../contract/IRandomNumberProvider.h"
#include "../delayreplayer/DelayReplayer.h"

namespace d6g {

/**
 * The DelayReplayerContainer class is a container for multiple DelayReplayer instances.
 * It allows managing and accessing various DelayReplayers by keys (usually names).
 * This class is particularly useful in simulations of the real-world scenarios,
 * enabling the representation of complex or varied statistical patterns.
 *
 * For more information about the module, see the NED documentation.
 */
class INET_API DelayReplayerContainer : public cSimpleModule, public IRandomNumberProvider {
private:
    std::map<std::string, DelayReplayer *> delayreplayers;

protected:
    virtual void initialize(int stage) override;

    /*!
     * Create and load a DelayReplayer instance from a file.
     *
     * @param fileName the name of the file to load the DelayReplayer from.
     *
     * @return DelayReplayer data according to the CSV file.
     */
    static DelayReplayer *loadDelayReplayerFromFile(const char *fileName);

public:
    /*!
     * Retrieves a DelayReplayer instance based on the given key.
     *
     * @param key the key of the DelayReplayer instance to retrieve.
     *
     * @return the DelayReplayer instance based on the given key.
     */
    DelayReplayer *getDelayreplayer(const std::string &key) const;


    /*!
     * Get a random delay time from the specific delayreplayer.
     *
     * @return random delay time from the specific delayreplayer.
     */
    cValue getRand(const std::string &key) override;

    ~DelayReplayerContainer() override;
};

} /* namespace d6g */

#endif /* DISTRIBUTION_DELAYREPLAYER_DELAYREPLAYERCONTAINER_H_ */

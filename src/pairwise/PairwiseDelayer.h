/*
 * PKDelay.h
 *
 *  Created on: Jun 22, 2023
 *      Author: dan
 */

#ifndef PAIRWISEDELAYER_H_
#define PAIRWISEDELAYER_H_

#include "inet/common/INETDefs.h"
#include "inet/queueing/base/PacketDelayerBase.h"

namespace pkdelay {
using namespace inet;

using namespace queueing;

class INET_API PairwiseDelayer : public PacketDelayerBase {
    static const short PKDELAY_ACTIVATE_KIND = 3495;

    class INET_API DelayEntry {
    public:
        DelayEntry(cXMLElement *delayEntity, cModule *context);

        int in = -1;
        int out = -1;
        double activateAt = 0;
        cDynamicExpression delay;

        ~DelayEntry() = default;
    };

private:
    std::map<int, std::map<int, DelayEntry *>> delays;

protected:
    void initialize(int stage) override;
    clocktime_t computeDelay(Packet *packet) const override;
    virtual void handleMessage(cMessage *message) override;
    void activateEntry(DelayEntry *delayEntry);
};


} // PKDelayer

#endif /* PAIRWISEDELAYER_H_ */

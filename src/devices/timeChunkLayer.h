#ifndef __INET_timeChunkLayer_H
#define __INET_timeChunkLayer_H




#include "inet/clock/common/ClockTime.h"
#include "inet/clock/contract/ClockTime.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/common/clock/ClockUserModuleBase.h"


namespace inet {

    class INET_API timeChunkLayer : public ClockUserModuleBase, cListener
{
    protected:
    std::map<uint16_t, clocktime_t> ingressTimeMap; // <sequenceId,ingressTime>

    protected:
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif

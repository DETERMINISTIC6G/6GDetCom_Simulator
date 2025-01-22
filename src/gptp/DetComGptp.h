//
// @authors: Enkhtuvshin Janchivnyambuu
//           Henning Puttnies
//           Peter Danielis
//           University of Rostock, Germany
//

#ifndef __DETERMINISTIC6G_DETCOMGPTP_H_
#define __DETERMINISTIC6G_DETCOMGPTP_H_

#include "../utils/InterfaceFilterMixin.h"
#include "inet/linklayer/ieee8021as/Gptp.h"

namespace d6g {
using namespace inet;

class DetComGptp : public InterfaceFilterMixin<Gptp>
{
  protected:
    std::set<int> detComInterfaces;

    clocktime_t detComIngressTimestamp5G = -1; // When the gPTP Sync messages switches from the wired to the 5G part of
                                               // the network (ingress into the 5G system)
    clocktime_t detComEgressTimestamp5G =
        -1; // When the gPTP Sync messages switches from the 5G part to the wired part of the network
    clocktime_t detComIngressTimestamp5GRcvd = -1; // The received detComIngressTimestamp from the sender of the sync

    clocktime_t detComIngressTimestampGptp = -1; // Same as above but in TSN clock domain
    clocktime_t detComEgressTimestampGptp = -1;  // Same as above but in TSN clock domain

    clocktime_t detComEgressTimestampGptpPrev = -1; // To store the 5G egress timestamp for 5G domain and TSN domain
    clocktime_t detComEgressTimestamp5GPrev = -1;   // To store the 5G egress timestamp for 5G domain and TSN domain
    ModuleRefByPar<IClock> detComClock;

    double clock5GRateRatio = 1.0;
    bool useC5Grr = false; // use clock 5G rate ratio

  protected:
    void initialize(int stage) override;
    virtual void processFollowUp(Packet *packet, const GptpFollowUp *gptp) override;
    virtual void processSync(Packet *packet, const GptpSync *gptp);
    virtual void sendFollowUp(int portId, const GptpSync *sync, const clocktime_t &syncEgressTimestampOwn) override;
    virtual void synchronize() override;
    virtual void sendSync() override;
    virtual void handleClockJump(ServoClockBase::ClockJumpDetails *clockJumpDetails) override;
    virtual void scheduleMessageOnTopologyChange() override;
  virtual void executeBmca() override;
};

} // namespace d6g

#endif

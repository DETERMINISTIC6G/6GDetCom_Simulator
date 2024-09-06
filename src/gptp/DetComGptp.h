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
    clocktime_t syncIngressTimestampDetCom = -1;     // ingress time of Sync at slave (this node)

  protected:
    void initialize(int stage) override;
    virtual void processFollowUp(Packet *packet, const GptpFollowUp *gptp) override;
    virtual void processSync(Packet *packet, const GptpSync *gptp) override;
    virtual void sendFollowUp(int portId, const GptpSync *sync, const clocktime_t &syncEgressTimestampOwn) override;
};

} // namespace d6g

#endif

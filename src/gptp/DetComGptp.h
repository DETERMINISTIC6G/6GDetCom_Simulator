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
    void initialize(int stage) override;
    virtual void sendPdelayReq() override;
};

} // namespace d6g

#endif

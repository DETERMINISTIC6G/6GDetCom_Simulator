// This file is part of Deliverable D4.4 [D44PLACEHOLDER]
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef __DETERMINISTIC6G_DETAILEDDELAYER_H_
#define __DETERMINISTIC6G_DETAILEDDELAYER_H_

#include <omnetpp.h>

#include "../../utils/InterfaceFilterMixin.h"
#include "inet/clock/contract/IClock.h"
#include "inet/queueing/base/PacketDelayerBase.h"

using namespace omnetpp;
using namespace inet;
using namespace inet::queueing;

namespace d6g {

class PdcDelayer : public InterfaceFilterMixin<PacketDelayerBase>
{
    ~PdcDelayer() override;

  protected:
    struct Mapping {
        cDynamicExpression *pdc = nullptr;
        cDynamicExpression *jitter = nullptr;

        ~Mapping()
        {
            delete pdc;
            delete jitter;
        }
    };

  private:
    cPar *defaultPdc = nullptr;
    cPar *defaultJitter = nullptr;
    // std::set<int> indInterfaces;
    // std::set<int> reqInterfaces;
    std::map<std::string, Mapping *> mappings;

  protected:
    void initialize(int stage) override;

    /**!
     * Compute the delay for the given packet.
     * Uses the InterfaceInd and InterfaceReq tags to determine the input and output interfaces.
     *
     * @param packet Packet to delay
     * @return delay for the packet
     */
    clocktime_t computeDelay(Packet *packet) const override;

    void setDefaultPdc(cPar *delay);

    void setDefaultJitter(cPar *jitter);

    // void addInterfacesToSet(std::set<int> &set, const char *interfaceList);

    void handleParameterChange(const char *parname) override;

    void configureMappings();
};

} // namespace d6g

#endif

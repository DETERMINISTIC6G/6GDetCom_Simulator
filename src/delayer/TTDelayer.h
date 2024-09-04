// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


#ifndef __DETERMINISTIC6G_DETAILEDDELAYER_H_
#define __DETERMINISTIC6G_DETAILEDDELAYER_H_

#include <omnetpp.h>
#include "inet/queueing/base/PacketDelayerBase.h"

using namespace omnetpp;
using namespace inet;
using namespace inet::queueing;

namespace d6g {

class TTDelayer : public PacketDelayerBase {
private:
    cPar *delayParameter = nullptr;
    std::set<int> indInterfaces;
    std::set<int> reqInterfaces;

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

    void setDelay(cPar *delay);

    void addInterfacesToSet(std::set<int> &set, const char *interfaceList);

    void handleParameterChange(const char *parname) override;

    void addResidenceTimeTag(Packet *pPacket, double delay) const;
};

} //namespace

#endif

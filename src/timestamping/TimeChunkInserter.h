// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef DEVICES_TSNTRANSLATOR_TIMECHUNKINSERTER_H_
#define DEVICES_TSNTRANSLATOR_TIMECHUNKINSERTER_H_

#include "inet/common/ProtocolUtils.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace d6g {

using namespace inet;
using namespace inet::queueing;

enum EtherType5G { ETHERTYPE_5G_TIME_TAG = 0x820F };

class TimeChunkInserter : public PacketFlowBase
{
  public:
    static const Protocol timeTagProtocol;

  protected:
    const Protocol *nextProtocol = nullptr;
    std::set<int> reqInterfaces;

  protected:
    virtual void initialize(int stage) override;

    virtual void processPacket(Packet *packet) override;
    void addInterfacesToSet(std::set<int> &set, const char *interfaceType);
};

} // namespace d6g

#endif

// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef DEVICES_TSNTRANSLATOR_TIMECHUNKINSERTER_H_
#define DEVICES_TSNTRANSLATOR_TIMECHUNKINSERTER_H_

#include "inet/queueing/base/PacketFlowBase.h"
#include "inet/common/ProtocolUtils.h"

namespace d6g {


    using namespace inet;
    using namespace inet::queueing;

    class TimeChunkInserter : public PacketFlowBase {
    public:
        static const Protocol timeTag;

    protected:
        const Protocol *nextProtocol = nullptr;

    protected:
        virtual void initialize(int stage) override;
        virtual void processPacket(Packet *packet) override;

    };

} // namespace inet

#endif


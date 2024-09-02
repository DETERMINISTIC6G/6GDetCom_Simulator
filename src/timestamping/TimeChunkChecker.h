// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef DEVICES_TSNTRANSLATOR_TIMECHUNKCHECKER_H_
#define DEVICES_TSNTRANSLATOR_TIMECHUNKCHECKER_H_

#include "inet/queueing/base/PacketFilterBase.h"
#include "inet/common/IProtocolRegistrationListener.h"


namespace d6g {

    using namespace inet;
    using namespace inet::queueing;


    class TimeChunkChecker : public PacketFilterBase {
    protected:
        virtual void processPacket(Packet *packet) override;
        virtual bool matchesPacket(const Packet *packet) const override;
    };

} // namespace inet

#endif


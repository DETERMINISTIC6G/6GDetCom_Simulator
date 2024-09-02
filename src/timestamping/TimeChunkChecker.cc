// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "TimeChunkChecker.h"
#include "TimeChunk_m.h"

#include "TimeTagDetCom_m.h"


namespace d6g {
    Define_Module(TimeChunkChecker);

    void TimeChunkChecker::processPacket(Packet *packet) {
        Enter_Method("processPacket");
        auto timeChunk = packet->popAtBack<TimeChunk>(B(16));

        auto timeTag = packet->addTagIfAbsent<DetComIngressTime>();
        timeTag->setReceptionStarted(ClockTime(timeChunk->getReceptionStarted(), SIMTIME_NS));
        timeTag->setReceptionEnded(ClockTime(timeChunk->getReceptionEnded(), SIMTIME_NS));
    }

    bool TimeChunkChecker::matchesPacket(const Packet *packet) const {
        // TODO: This shouldn't be needed
        return true;
    }

} // namespace inet

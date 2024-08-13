// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "TimeChunkChecker.h"
#include "TimeChunk_m.h"

#include "inet/common/ProgressTag_m.h"

#include "inet/networklayer/common/TimeTag_m.h"


namespace d6g {


Define_Module(TimeChunkChecker);


void TimeChunkChecker::processPacket(Packet *packet) {
    Enter_Method("processPacket");
    auto ingressTime = packet->peekAtBack<TimeChunk>();

    packet->addTag<IngressTimeInd>()->setReceptionStarted(ingressTime->getReceptionStarted());
    packet->addTag<IngressTimeInd>()->setReceptionEnded(ingressTime->getReceptionEnded());


}

} // namespace inet


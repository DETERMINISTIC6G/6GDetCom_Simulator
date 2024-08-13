// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "TimeChunkInserter.h"
#include "TimeChunk_m.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProgressTag_m.h"

#include "inet/networklayer/common/TimeTag_m.h"


namespace d6g {


Define_Module(TimeChunkInserter);


void TimeChunkInserter::processPacket(Packet *packet) {
    Enter_Method("processPacket");
    auto ingressTime = packet->getTag<IngressTimeInd>()->getReceptionStarted();
    auto ingressTimeData = makeShared<TimeChunk>();
    ingressTimeData->setReceptionStarted(ingressTime.inUnit(SIMTIME_NS));
    packet->insertAtBack(ingressTimeData);
}

//void TimeChunkInserter::checkChunk(Packet *packet) {
//    Enter_Method("checkChunk");
//    ingressTimeData = packet->popAtBack<ByteCountChunk>(4);
//    packet->addTag<IngressTimeInd>(); //not correct
//}

} // namespace inet


// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "TimeChunkInserter.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProgressTag_m.h"

namespace inet {

//Define_Module(InterpacketGapInserter);
Define_Module(TIMECHUNKINSERTER);


void TimeChunkInserter::insertChunk(Packet *packet) {
    Enter_Method("insertChunk");
    ingressTime = packet->getTag<IngressTimeInd>()->getReceptionStarted();

    auto ingressTimeData = makeShared<ByteCountChunk>(B(4),
                                                      ingressTime); // chunk's type should be configured, and convert data type first?
    packet->insertAtBack(ingressTimeData);
}

void TimeChunkInserter::checkChunk(Packet *packet) {
    Enter_Method("checkChunk");
    ingressTimeData = packet->popAtBack<ByteCountChunk>(4);
    packet->addTag<IngressTimeInd>(); //not correct
}

} // namespace inet


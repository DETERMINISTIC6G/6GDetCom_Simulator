// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "TimeChunkInserter.h"
#include "TimeChunk_m.h"

#include "inet/networklayer/common/TimeTag_m.h"
#include "inet/common/ProtocolTag_m.h"


namespace d6g {
    Define_Module(TimeChunkInserter);

    using namespace inet;

    void TimeChunkInserter::processPacket(Packet *packet) {
        Enter_Method("processPacket");

        if (auto ingressTag = packet->findTag<IngressTimeInd>()) {
            auto ingressTimeChunk = makeShared<TimeChunk>();
            ingressTimeChunk->setReceptionStarted(0);
            ingressTimeChunk->setReceptionEnded(1);
            packet->insertAtBack(ingressTimeChunk);
        }
    }

    void TimeChunkInserter::initialize(int stage) {
        PacketFlowBase::initialize(stage);
    }


} // namespace inet


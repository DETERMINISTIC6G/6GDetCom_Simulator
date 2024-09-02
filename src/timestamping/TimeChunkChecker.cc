// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "TimeChunkChecker.h"
#include "TimeChunkInserter.h"
#include "TimeChunk_m.h"
#include "inet/linklayer/common/EtherType_m.h"

#include "TimeTagDetCom_m.h"


namespace d6g {
    Define_Module(TimeChunkChecker);

    void TimeChunkChecker::initialize(int stage) {
        PacketFilterBase::initialize(stage);
        if (stage == INITSTAGE_LINK_LAYER)
            registerProtocol(TimeChunkInserter::timeTagProtocol, nullptr, inputGate);
    }

    void TimeChunkChecker::processPacket(Packet *packet) {
        Enter_Method("processPacket");
        auto timeChunk = packet->popAtFront<TimeChunk>(B(18));
        appendEncapsulationProtocolInd(packet, &TimeChunkInserter::timeTagProtocol);

        auto timeTag = packet->addTagIfAbsent<DetComIngressTime>();
        timeTag->setReceptionStarted(timeChunk->getReceptionStarted());
        timeTag->setReceptionEnded(timeChunk->getReceptionEnded());

        auto typeOrLength = timeChunk->getTypeOrLength();
        const Protocol *protocol;
        if (isIeee8023Length(typeOrLength))
            protocol = &Protocol::ieee8022llc;
        else
            protocol = ProtocolGroup::getEthertypeProtocolGroup()->getProtocol(typeOrLength);
        auto packetProtocolTag = packet->addTagIfAbsent<PacketProtocolTag>();
        packetProtocolTag->setFrontOffset(b(0));
        packetProtocolTag->setBackOffset(b(0));
        packetProtocolTag->setProtocol(protocol);
        packet->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(protocol);
    }

    bool TimeChunkChecker::matchesPacket(const Packet *packet) const
    {
        const auto& header = packet->peekAtFront<TimeChunk>();
        auto typeOrLength = header->getTypeOrLength();
        if (!isIeee8023Length(typeOrLength) && ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(typeOrLength) == nullptr)
            return false;
        else {
            return true;
        }
    }

} // namespace inet

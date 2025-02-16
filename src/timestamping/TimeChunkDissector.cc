//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "TimeChunkDissector.h"

#include "DetComTimeChunk_m.h"
#include "TimeChunkInserter.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "inet/linklayer/common/EtherType_m.h"

namespace d6g {
using namespace inet;

Register_Protocol_Dissector(&TimeChunkInserter::timeTagProtocol, TimeChunkDissector);

void TimeChunkDissector::dissect(Packet *packet, const Protocol *protocol, ICallback &callback) const
{
    const auto &header = packet->popAtFront<DetComTimeChunk>();
    callback.startProtocolDataUnit(protocol);
    callback.visitChunk(header, protocol);
    int typeOrLength = header->getTypeOrLength();
    if (isEth2Type(typeOrLength)) {
        auto payloadProtocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(typeOrLength);
        callback.dissectPacket(packet, payloadProtocol);
    }
    else {
        auto ethEndOffset = packet->getFrontOffset() + B(typeOrLength);
        auto trailerOffset = packet->getBackOffset();
        packet->setBackOffset(ethEndOffset);
        callback.dissectPacket(packet, &Protocol::ieee8022llc);
        packet->setBackOffset(trailerOffset);
    }
    callback.endProtocolDataUnit(protocol);
}

} // namespace d6g

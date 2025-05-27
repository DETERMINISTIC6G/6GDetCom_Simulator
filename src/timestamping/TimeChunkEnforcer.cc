// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "TimeChunkEnforcer.h"

#include "DetComTimeTag_m.h"
#include "TimeChunkInserter.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace d6g {

Define_Module(TimeChunkEnforcer);

void TimeChunkEnforcer::initialize(int stage)
{
    InterfaceFilterMixin::initialize(stage);
}

void TimeChunkEnforcer::processPacket(Packet *packet) {
    auto ingressTag = packet->findTag<DetComIngressTimeTag>();
    if (ingressTag && matchesInterfaceConfiguration(packet)) {
        ensureEncapsulationProtocolReq(packet, &TimeChunkInserter::timeTagProtocol);
    } else {
        removeEncapsulationProtocolReq(packet, &TimeChunkInserter::timeTagProtocol);
    }

    setDispatchProtocol(packet);
    handlePacketProcessed(packet);
}

cGate *TimeChunkEnforcer::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

} // namespace d6g

//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

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

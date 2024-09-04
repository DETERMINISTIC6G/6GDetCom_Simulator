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

#include "TimeChunkInserter.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

namespace d6g {

Define_Module(TimeChunkEnforcer);

void TimeChunkEnforcer::initialize(int stage)
{
    PacketFlowBase::initialize(stage);
}
void TimeChunkEnforcer::addInterfacesToSet(std::set<int> &set, const char *interfaceType)
{
    // Check if context has submodule with name interfaceType
    auto node = getContainingNode(this);
    if (!node->hasSubmoduleVector(interfaceType)) {
        throw cRuntimeError("No submodule with name '%s' found in '%s'", interfaceType, node->getFullPath().c_str());
    }

    // Get submodule vector with name interfaceType
    for (int i = 0; i < node->getSubmoduleVectorSize(interfaceType); i++) {
        auto *interface = dynamic_cast<NetworkInterface *>(node->getSubmodule(interfaceType, i));
        if (interface == nullptr) {
            throw cRuntimeError("Submodule with name '%s' is not a NetworkInterface", interfaceType);
        }
        set.insert(interface->getInterfaceId());
    }
}

void TimeChunkEnforcer::processPacket(Packet *packet) {
    ensureEncapsulationProtocolReq(packet, &TimeChunkInserter::timeTagProtocol);
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

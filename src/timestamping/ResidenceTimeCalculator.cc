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

#include "ResidenceTimeCalculator.h"
#include "inet/common/clock/ClockUserModuleMixinImpl.h"

#include "DetComTimeTag_m.h"

namespace inet {
template class ClockUserModuleMixin<inet::queueing::PacketFlowBase>;
}

namespace d6g {
Define_Module(ResidenceTimeCalculator);

void ResidenceTimeCalculator::initialize(int stage) { ClockUserModuleMixin::initialize(stage); }

void ResidenceTimeCalculator::processPacket(Packet *packet)
{
    auto detComIngressTag = packet->findTag<DetComIngressTimeTag>();
    if (!detComIngressTag) {
        throw cRuntimeError("DetComIngressTimeTag not found in packet");
    }

    auto residenceTimeTag = packet->addTag<DetComResidenceTimeTag>();
    auto currentTime = getClockTime();
    auto residenceTime = currentTime - detComIngressTag->getReceptionStarted();
    residenceTimeTag->setResidenceTime(residenceTime);
}

cGate *ResidenceTimeCalculator::getRegistrationForwardingGate(cGate *gate)
{
    if (gate == outputGate)
        return inputGate;
    else if (gate == inputGate)
        return outputGate;
    else
        throw cRuntimeError("Unknown gate");
}

} // namespace d6g

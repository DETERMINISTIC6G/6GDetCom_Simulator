// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#undef INET_IMPORT

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

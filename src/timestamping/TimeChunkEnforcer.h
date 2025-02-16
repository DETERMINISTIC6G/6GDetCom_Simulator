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

#ifndef __DETERMINISTIC6G_TIMECHUNKENFORCER_H_
#define __DETERMINISTIC6G_TIMECHUNKENFORCER_H_

#include <omnetpp.h>

#include "../utils/InterfaceFilterMixin.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/queueing/base/PacketFlowBase.h"

using namespace omnetpp;

namespace d6g {
using namespace inet;
using namespace inet::queueing;

/**
 * TODO - Generated class
 */
class TimeChunkEnforcer : public InterfaceFilterMixin<PacketFlowBase>, public TransparentProtocolRegistrationListener
{
  protected:
    virtual void initialize(int stage) override;
    virtual void processPacket(Packet *packet) override;
    cGate *getRegistrationForwardingGate(cGate *gate) override;
};

} // namespace d6g

#endif

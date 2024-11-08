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

#ifndef __DETERMINISTIC6G_DETAILEDDELAYER_H_
#define __DETERMINISTIC6G_DETAILEDDELAYER_H_

#include <omnetpp.h>

#include "../../utils/InterfaceFilterMixin.h"
#include "inet/queueing/base/PacketDelayerBase.h"
#include "inet/clock/contract/IClock.h"

using namespace omnetpp;
using namespace inet;
using namespace inet::queueing;

namespace d6g {

class PdcDelayer : public InterfaceFilterMixin<PacketDelayerBase>
{

  protected:
    class Mapping
    {
      public:
        std::string stream;
        double pdc = 0;
        double jitter = 0;
    };
    IClock *clock;

  private:
    cPar *delayParameter = nullptr;
    cPar *jitterParameter = nullptr;
    //std::set<int> indInterfaces;
    //std::set<int> reqInterfaces;
    std::vector<Mapping> mappings;

  protected:
    void initialize(int stage) override;

    /**!
     * Compute the delay for the given packet.
     * Uses the InterfaceInd and InterfaceReq tags to determine the input and output interfaces.
     *
     * @param packet Packet to delay
     * @return delay for the packet
     */
    clocktime_t computeDelay(Packet *packet) const override;

    void setDelay(cPar *delay);

    void setJitter(cPar *jitter);

    //void addInterfacesToSet(std::set<int> &set, const char *interfaceList);

    void handleParameterChange(const char *parname) override;

    void configureMappings();

};

} // namespace d6g

#endif

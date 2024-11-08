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

#include "PdcDelayer.h"
#include "../../timestamping/DetComTimeTag_m.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"

using namespace omnetpp;
using namespace inet;
namespace d6g {

Define_Module(PdcDelayer);

void PdcDelayer::initialize(int stage)
{
    PacketDelayerBase::initialize(stage);
    InterfaceFilterMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        clock = check_and_cast<IClock *>(getModuleByPath(par("clockModule").stringValue()));
        setDelay(&par("delay"));
        setJitter(&par("jitter"));
        configureMappings();
    }

}


clocktime_t PdcDelayer::computeDelay(Packet *packet) const
{

    if (!matchesInterfaceConfiguration(packet)) {
        return 0;
    }

    //auto detComIngressTag = packet->findTag<DetComIngressTimeTag>();

    auto residenceTimeTag = packet->findTag<DetComResidenceTimeTag>();
    auto streamIdTag = packet->findTag<StreamInd>();

    clocktime_t pdc = delayParameter->doubleValue();
    clocktime_t jitter = jitterParameter->doubleValue();

    if (residenceTimeTag != nullptr) {
        clocktime_t residenceTime = residenceTimeTag->getResidenceTime();
        if (streamIdTag != nullptr) {
            auto streamID = streamIdTag->getStreamName();
            for (auto &mapping : mappings) {
                if (!strcmp(mapping.stream.c_str(), streamID)) {
                    pdc = mapping.pdc;
                    jitter = mapping.jitter;
                    break;
                }
            }
        }
        if (residenceTime > pdc - jitter) {
            return 0;
        } else {
            clocktime_t maxDeadline = pdc - residenceTime;
            clocktime_t minDeadline = maxDeadline - jitter;
            return uniform(minDeadline, maxDeadline);
        }
    } else {
        return 0;
    }
}

void PdcDelayer::setDelay(cPar *delay) { delayParameter = delay; }

void PdcDelayer::setJitter(cPar *jitter) { jitterParameter = jitter; }

void PdcDelayer::handleParameterChange(const char *parname)
{
    if (!strcmp(parname, "delay")) {
        setDelay(&par("delay"));
    }
    if (!strcmp(parname, "jitter")) {
            setJitter(&par("jitter"));
        }
    if (!strcmp(parname, "mapping")) {
        configureMappings();
    }
}

void PdcDelayer::configureMappings()
{
    auto mappingParameter = check_and_cast<cValueArray *>(par("mapping").objectValue());

    mappings.resize(mappingParameter->size());
    for (int i = 0; i < mappingParameter->size(); i++) {
        auto element = check_and_cast<cValueMap *>(mappingParameter->get(i).objectValue());
        Mapping &mapping = mappings[i];
        mapping.stream = element->get("stream").stringValue();
        mapping.pdc = element->containsKey("pdc") ? simtime_t::parse(element->get("pdc")).dbl() : delayParameter->doubleValue();
        mapping.jitter = element->containsKey("jitter") ? simtime_t::parse(element->get("jitter")).dbl() : jitterParameter->doubleValue();
    }
}

} /* namespace d6g */

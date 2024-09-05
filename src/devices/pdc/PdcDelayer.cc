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

#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"

#include "../../timestamping/DetComTimeTag_m.h"

#include "inet/protocolelement/redundancy/StreamTag_m.h"

using namespace omnetpp;
using namespace inet;
namespace d6g {

Define_Module(PdcDelayer);

void PdcDelayer::initialize(int stage)
{
    PacketDelayerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        clock = check_and_cast<IClock *>(getModuleByPath(par("clockModule").stringValue()));
        setDelay(&par("delay"));
        configureMappings();
    }

    if (stage == INITSTAGE_LAST) {
        auto indInterfaceTypes = check_and_cast<cValueArray *>(par("indInterfaceTypes").objectValue());
        for (int i = 0; i < indInterfaceTypes->size(); i++) {
            addInterfacesToSet(indInterfaces, indInterfaceTypes->get(i).stringValue());
        }

        auto reqInterfaceTypes = check_and_cast<cValueArray *>(par("reqInterfaceTypes").objectValue());
        for (int i = 0; i < reqInterfaceTypes->size(); i++) {
            addInterfacesToSet(reqInterfaces, reqInterfaceTypes->get(i).stringValue());
        }

        EV << "TTDelayer: " << getFullPath() << " === ";
        EV << " indInterfaces: ";
        for (auto interfaceId : indInterfaces) {
            EV << interfaceId << " ";
        }
        EV << " ===  reqInterfaces: ";
        for (auto interfaceId : reqInterfaces) {
            EV << interfaceId << " ";
        }
        EV << endl;
    }
}

void PdcDelayer::addInterfacesToSet(std::set<int> &set, const char *interfaceType)
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

clocktime_t PdcDelayer::computeDelay(Packet *packet) const
{
    auto context = getContainingNode(this);

    bool indInterfaceMatch = false;
    bool reqInterfaceMatch = false;

    if (indInterfaces.empty() || !packet->hasTag<InterfaceInd>()) {
        indInterfaceMatch = true;
    }
    if (!indInterfaceMatch) {
        auto interfaceInd = packet->getTag<InterfaceInd>();
        indInterfaceMatch = indInterfaces.find(interfaceInd->getInterfaceId()) != indInterfaces.end();
    }

    if (reqInterfaces.empty() || !packet->hasTag<InterfaceReq>()) {
        reqInterfaceMatch = true;
    }
    if (!reqInterfaceMatch) {
        auto interfaceReq = packet->getTag<InterfaceReq>();
        reqInterfaceMatch = reqInterfaces.find(interfaceReq->getInterfaceId()) != reqInterfaces.end();
    }

    if (!indInterfaceMatch || !reqInterfaceMatch) {
        return 0;
    }

    clocktime_t timeDifference;
    auto creationTimeTag = packet->findTag<DetComIngressTimeTag>();
    auto residenceTime = packet->findTag<DetComResidenceTimeTag>();

    if (creationTimeTag != nullptr && residenceTime != nullptr) {
        clocktime_t timestamp = creationTimeTag->getReceptionEnded();
        clocktime_t currentTime = clock->getClockTime();
        //clocktime_t currentTime = residenceTime->getResidenceTime();
        timeDifference = currentTime - timestamp;

        double pdc = delayParameter->doubleValue();

        auto streamIdTag = packet->findTag<StreamInd>();
        if (streamIdTag != nullptr) {
            auto streamID = streamIdTag->getStreamName();
            for (auto &mapping : mappings) {
                if (!strcmp(mapping.stream.c_str(), streamID)) {
                    //const_cast<cPar &>(par("delay")).setDoubleValue(mapping.pdc);
                    pdc = mapping.pdc;
                    break;
                }
            }
        }
        if (timeDifference.dbl() > pdc) {
            return 0;
        } else {
            return pdc - timeDifference.dbl();
        }
    } else {
        return 0;
    }
}

void PdcDelayer::setDelay(cPar *delay) { delayParameter = delay; }

void PdcDelayer::handleParameterChange(const char *parname)
{
    if (!strcmp(parname, "delay")) {
        setDelay(&par("delay"));
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
        mapping.pdc = element->containsKey("pdc") ? simtime_t::parse(element->get("pdc")).dbl() : 0;
    }
}

} /* namespace d6g */

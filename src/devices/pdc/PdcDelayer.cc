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
//#include "inet/common/TimeTag_m.h"

//#include "../timestamping/DetComTimeTag_m.h"
//#include "../timestamping/TimeChunkInserter.h"
#include "inet/common/ProtocolUtils.h"
#include "inet/networklayer/common/TimeTag_m.h"

//#include "inet/common/DirectionTag_m.h"

//#include "inet/common/ProtocolTag_m.h"
//#include "inet/common/SequenceNumberTag_m.h"
//#include "inet/linklayer/common/DropEligibleTag_m.h"
//#include "inet/linklayer/common/InterfaceTag_m.h"
//#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/common/PcpTag_m.h"
#include "inet/linklayer/common/UserPriorityTag_m.h"
#include "inet/linklayer/common/VlanTag_m.h"
//#include "inet/protocolelement/cutthrough/CutthroughTag_m.h"
#include "inet/protocolelement/redundancy/StreamTag_m.h"
//#include "inet/protocolelement/shaper/EligibilityTimeTag_m.h"
//#include "inet/linklayer/ieee8021q/Ieee8021qTagHeader_m.h"
#include "inet/common/packet/Packet.h"

using namespace omnetpp;
using namespace inet;
namespace d6g {

Define_Module(PdcDelayer);

void PdcDelayer::initialize(int stage)
{
    PacketDelayerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        clock = check_and_cast<IClock*>(getModuleByPath(par("clockModule").stringValue()));
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

void PdcDelayer::addInterfacesToSet(std::set<int>& set, const char *interfaceType) {
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
    //#####
   auto& tags = packet->getTags();
   for (int i=0; i<tags.getNumTags(); i++) {
       EV << "Tag in PDC Layer: " << tags.getTag(i) << endl;
   }


    clocktime_t timeDifference;
    auto CreationTimeTag = packet->findTag<inet::IngressTimeTag>();
    if (CreationTimeTag != nullptr) {
        clocktime_t timestamp = CreationTimeTag->getReceptionStarted(); //getCreationTime();
        EV << "Timestamp: " << timestamp << endl;
        clocktime_t currentTime = clock->getClockTime();
        timeDifference = currentTime - timestamp.dbl();


        //packet->removeTag<inet::CreationTimeTag>();

        auto streamIdTag = packet->findTag<StreamInd>();
        if (streamIdTag != nullptr) {
            auto streamID = streamIdTag->getStreamName();
            EV << "Stream ID: " << streamID << endl;
            for (auto &mapping : mappings) {
                if (!strcmp(mapping.stream.c_str(), streamID)) {
                    const_cast<cPar&>(par("delay")).setDoubleValue(mapping.pdc);

                    EV << "Delay aus Mapping: " << mapping.pdc << endl;
                    break;
                }
            }
        }
        if (timeDifference.dbl() > delayParameter->doubleValue()) {
            return 0;
        } else {
            return (delayParameter->doubleValue()) - timeDifference.dbl();
        }
    } else {
        return 0;
    }
}

void PdcDelayer::setDelay(cPar *delay) {
    delayParameter = delay;
}

void PdcDelayer::handleParameterChange(const char *parname) {
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
        Mapping& mapping = mappings[i];
        //mapping.vlanId = element->containsKey("vlan") ? element->get("vlan").intValue() : -1;
        mapping.pcp = element->containsKey("pcp") ? element->get("pcp").intValue() : -1;
        mapping.stream = element->get("stream").stringValue();
        mapping.pdc = element->containsKey("pdc") ? simtime_t::parse(element->get("pdc")).dbl() : 0;

    }
}

} /* namespace d6g */

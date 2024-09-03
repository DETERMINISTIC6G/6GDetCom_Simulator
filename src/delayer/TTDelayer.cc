// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "TTDelayer.h"

#include "../timestamping/TimeChunkInserter.h"
#include "inet/common/ProtocolUtils.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/common/TimeTag_m.h"

namespace d6g {
Define_Module(TTDelayer);

void TTDelayer::initialize(int stage)
{
    PacketDelayerBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        setDelay(&par("delay"));
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

void TTDelayer::addInterfacesToSet(std::set<int> &set, const char *interfaceType)
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

clocktime_t TTDelayer::computeDelay(Packet *packet) const
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
    //####
    auto& tags = packet->getTags();
       for (int i=0; i<tags.getNumTags(); i++) {
           EV << "Tag in Delay Layer: " << tags.getTag(i) << endl;
       }

    auto tag = packet->addTagIfAbsent<inet::CreationTimeTag>();
    tag->setCreationTime(simTime());
    EV << "Timestamp created: " << tag->getCreationTime() << endl;
    //####
    return delayParameter->doubleValue();
}

void TTDelayer::setDelay(cPar *delay) { delayParameter = delay; }

void TTDelayer::handleParameterChange(const char *parname)
{
    if (!strcmp(parname, "delay")) {
        setDelay(&par("delay"));
    }
}

void TTDelayer::processPacket(Packet *packet, simtime_t sendingTime)
{
    PacketDelayerBase::processPacket(packet, sendingTime);
    ensureEncapsulationProtocolReq(packet, &TimeChunkInserter::timeTagProtocol);
    setDispatchProtocol(packet);
}
} // namespace d6g

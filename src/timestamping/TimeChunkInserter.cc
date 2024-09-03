// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "TimeChunkInserter.h"
#include "DetComTimeChunk_m.h"

#include "inet/networklayer/common/TimeTag_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/linklayer/common/InterfaceTag_m.h"


namespace d6g {
    Define_Module(TimeChunkInserter);

    using namespace inet;

    const Protocol TimeChunkInserter::timeTagProtocol("timeTag5G", "5G Internal Time Tag");

    void TimeChunkInserter::initialize(int stage) {
        PacketFlowBase::initialize(stage);
        if (stage == INITSTAGE_LOCAL) {
            const char *nextProtocolAsString = par("nextProtocol");
            if (*nextProtocolAsString != '\0')
                nextProtocol = Protocol::getProtocol(nextProtocolAsString);

            auto protocol = ProtocolGroup::getEthertypeProtocolGroup()->findProtocol(ETHERTYPE_5G_TIME_TAG);
            if (!protocol) {
                ProtocolGroup::getEthertypeProtocolGroup()->addProtocol(ETHERTYPE_5G_TIME_TAG, &timeTagProtocol);
            }
        }
        else if (stage == INITSTAGE_LINK_LAYER)
            registerService(timeTagProtocol, inputGate, nullptr);
        else if (stage == INITSTAGE_LAST) {
            auto reqInterfaceTypes = check_and_cast<cValueArray *>(par("reqInterfaceTypes").objectValue());
            for (int i = 0; i < reqInterfaceTypes->size(); i++) {
                addInterfacesToSet(reqInterfaces, reqInterfaceTypes->get(i).stringValue());
            }
            EV << " ===  reqInterfaces: ";
            for (auto interfaceId : reqInterfaces) {
                EV << interfaceId << " ";
            }
            EV << endl;
        }
    }

    void TimeChunkInserter::addInterfacesToSet(std::set<int> &set, const char *interfaceType)
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

    void TimeChunkInserter::processPacket(Packet *packet) {
        Enter_Method("processPacket");

        auto interfaceReqTag = packet->getTag<InterfaceReq>();
        auto interfaceReq = reqInterfaces.find(interfaceReqTag->getInterfaceId());

        auto ingressTag = packet->findTag<IngressTimeTag>();
        if (ingressTag && interfaceReq != reqInterfaces.end()) {
            auto ingressTimeChunk = makeShared<DetComTimeChunk>();
            ingressTimeChunk->setReceptionStarted(ingressTag->getReceptionStarted());
            ingressTimeChunk->setReceptionEnded(ingressTag->getReceptionEnded());

            auto& packetProtocolTag = packet->getTagForUpdate<PacketProtocolTag>();
            auto protocol = packetProtocolTag->getProtocol();
            if (protocol == &Protocol::ieee8022llc)
                ingressTimeChunk->setTypeOrLength(packet->getByteLength());
            else
                ingressTimeChunk->setTypeOrLength(ProtocolGroup::getEthertypeProtocolGroup()->findProtocolNumber(protocol));
            packet->insertAtFront(ingressTimeChunk);
            packetProtocolTag->setProtocol(&timeTagProtocol);
            packetProtocolTag->setFrontOffset(b(0));
        }
        removeDispatchProtocol(packet, &timeTagProtocol);
        setDispatchProtocol(packet, nextProtocol != nullptr ? nextProtocol : &Protocol::ethernetMac);
    }


} // namespace inet


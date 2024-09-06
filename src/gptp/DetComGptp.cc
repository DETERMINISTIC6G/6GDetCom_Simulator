#include "DetComGptp.h"

#include "../timestamping/DetComTimeTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

namespace d6g {

Define_Module(DetComGptp);

void DetComGptp::initialize(int stage)
{
    InterfaceFilterMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        if (gptpNodeType != BRIDGE_NODE) {
            throw cRuntimeError("DetComGptp is only supported for bridge nodes");
        }
    }
    if (stage == INITSTAGE_LINK_LAYER) {
        detComInterfaces.clear();
        auto detComInterfaceTypes = check_and_cast<cValueArray *>(par("detComInterfaceTypes").objectValue());
        for (int i = 0; i < detComInterfaceTypes->size(); i++) {
            addInterfacesToSet(detComInterfaces, detComInterfaceTypes->get(i).stringValue());
        }

        if (idMatchesSet(slavePortId, detComInterfaces)) {
            cancelClockEvent(selfMsgDelayReq);
        }
    }
}

void DetComGptp::processFollowUp(Packet *packet, const GptpFollowUp *gptp)
{
    auto gptpNow = gptp;
    auto interfaceInd = packet->findTag<InterfaceInd>();
    if (!interfaceInd) {
        throw cRuntimeError("InterfaceInd tag not found in packet");
    }
    if (idMatchesSet(interfaceInd->getInterfaceId(), detComInterfaces)) {
        // If meesage was received from a detCom interface, we need to update the correction field
        // to reflect the residence time
        auto correctionField = gptp->getCorrectionField();
        auto detComResidenceTimeTag = packet->findTag<DetComResidenceTimeTag>();
        if (!detComResidenceTimeTag) {
            throw cRuntimeError("DetComResidenceTimeTag not found in packet");
        }
        correctionField += detComResidenceTimeTag->getResidenceTime();
        auto gptpDup = gptp->dup();
        gptpDup->setCorrectionField(correctionField);
        gptpNow = gptpDup;
    }
    Gptp::processFollowUp(packet, gptpNow);
}


void DetComGptp::processSync(Packet *packet, const GptpSync *gptp)
{
    auto interfaceInd = packet->findTag<InterfaceInd>();
    if (!interfaceInd) {
        throw cRuntimeError("InterfaceInd tag not found in packet");
    }
    if (!idMatchesSet(interfaceInd->getInterfaceId(), detComInterfaces)) {
        // When message is not received from a detCom interface (i.e. it is received from outside the detCom node),
        // we need to store the detCom ingress time
        auto detComTimeTag = packet->findTag<DetComIngressTimeTag>();
        if (!detComTimeTag) {
            throw cRuntimeError("DetComIngressTimeTag not found in packet");
        }
        syncIngressTimestampDetCom = detComTimeTag->getReceptionStarted();
    }
    Gptp::processSync(packet, gptp);
}

void DetComGptp::sendFollowUp(int portId, const GptpSync *sync, const clocktime_t &syncEgressTimestampOwn)
{
    if (!idMatchesSet(portId, detComInterfaces)) {
        Gptp::sendFollowUp(portId, sync, syncEgressTimestampOwn);
        return;
    }

    auto packet = new Packet("GptpFollowUp");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto gptp = makeShared<GptpFollowUp>();
    gptp->setDomainNumber(domainNumber);
    gptp->setPreciseOriginTimestamp(preciseOriginTimestamp);
    gptp->setSequenceId(sync->getSequenceId());

    // If the message is sent to a detCom interface, don't calculate residence time, but keep correction field
    // and set the DetComIngressTimeTag
    auto detComTimeTag = packet->addTag<DetComIngressTimeTag>();
    detComTimeTag->setReceptionStarted(syncIngressTimestampDetCom);
    gptp->setCorrectionField(gptp->getCorrectionField());

    emit(correctionFieldEgressSignal, gptp->getCorrectionField().asSimTime());
    gptp->getFollowUpInformationTLVForUpdate().setRateRatio(gmRateRatio);
    packet->insertAtFront(gptp);

    EV_INFO << "############## SEND FOLLOW_UP ################################" << endl;
    EV_INFO << "Correction Field              - " << gptp->getCorrectionField() << endl;
    EV_INFO << "gmRateRatio                   - " << gmRateRatio << endl;
    EV_INFO << "meanLinkDelay                 - " << meanLinkDelay << endl;

    sendPacketToNIC(packet, portId);
}
} // namespace d6g

#include "DetComGptp.h"

#include "../timestamping/DetComTimeTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

namespace d6g {

Define_Module(DetComGptp);

void DetComGptp::initialize(int stage)
{
    InterfaceFilterMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        useC5Grr = par("useC5Grr"); // par can't be used
        detComClock.reference(this, "detComClockModule", true);
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
    if (stage == INITSTAGE_LAST) {
        detComEgressTimestamp5G = detComClock->getClockTime();
        detComEgressTimestampGptp = clock->getClockTime();
    }
}


void DetComGptp::scheduleMessageOnTopologyChange() {
    Gptp::scheduleMessageOnTopologyChange();
    if (selfMsgDelayReq && slavePortId != -1 && idMatchesSet(slavePortId, detComInterfaces)) {
        // Don't send DelayReq messages on DetCom interfaces (between TsnTranslators)
        cancelClockEvent(selfMsgDelayReq);
    }
}

void DetComGptp::processSync(Packet *packet, const GptpSync *gptp)
{
    auto indInterface = packet->getTag<InterfaceInd>()->getInterfaceId();
    if (idMatchesSet(indInterface, detComInterfaces)) {
        // Rcvd. from DetCom interface, set correct ingress times
        detComEgressTimestamp5GPrev = detComEgressTimestamp5G;
        detComEgressTimestampGptpPrev = detComEgressTimestampGptp;

        detComEgressTimestamp5G = detComClock->getClockTime();
        detComEgressTimestampGptp = clock->getClockTime();

        EV_INFO << "detComEgressTimestamp5G          - " << detComEgressTimestamp5G << endl;
        EV_INFO << "detComEgressTimestampGptp        - " << detComEgressTimestampGptp << endl;

        if (!useC5Grr || detComEgressTimestamp5GPrev == -1 || detComEgressTimestampGptpPrev == -1){
            clock5GRateRatio = 1.0;
            EV_INFO << "IF--===============================================" << endl;
            EV_INFO << "detComEgressTimestamp5GPrev          - " << detComEgressTimestamp5GPrev << endl;
            EV_INFO << "detComEgressTimestampGptpPrev        - " << detComEgressTimestampGptpPrev << endl;
        }
        else {
            EV_INFO << "ELSE--===============================================" << endl;
            EV_INFO << "detComEgressTimestamp5GPrev          - " << detComEgressTimestamp5GPrev << endl;
            EV_INFO << "detComEgressTimestampGptpPrev        - " << detComEgressTimestampGptpPrev << endl;
            clock5GRateRatio = (detComEgressTimestamp5G - detComEgressTimestamp5GPrev) /
                               (detComEgressTimestampGptp - detComEgressTimestampGptpPrev);
            EV_INFO << "Clock 5G Rate Ratio ================================= : " << clock5GRateRatio << endl;
        }
    }
    Gptp::processSync(packet, gptp);
}

void DetComGptp::processFollowUp(Packet *packet, const GptpFollowUp *gptp)
{
    auto indInterface = packet->getTag<InterfaceInd>()->getInterfaceId();
    if (idMatchesSet(indInterface, detComInterfaces)) {
        detComIngressTimestamp5GRcvd = packet->getTag<DetComIngressTimeTag>()->getReceptionStarted();
    }
    else {
        detComIngressTimestamp5GRcvd = -1;
    }
    EV_INFO << "############## PROCESS FOLLOW_UP DetCom #####################################" << endl;
    EV_INFO << "detComIngressTimestamp5GRcvd     - " << detComIngressTimestamp5GRcvd << endl;
    Gptp::processFollowUp(packet, gptp);
}

void DetComGptp::synchronize()
{
    /************** Time synchronization *****************************************
     * Local time is adjusted using peer delay, correction field, residence time *
     * and packet transmission time based departure time of Sync message from GM *
     *****************************************************************************/

    EV_INFO << "############## SYNC #####################################" << endl;

    simtime_t now = simTime();
    clocktime_t oldLocalTimeAtTimeSync = clock->getClockTime();
    emit(timeDifferenceSignal, CLOCKTIME_AS_SIMTIME(oldLocalTimeAtTimeSync) - now);

    clocktime_t residenceTime = CLOCKTIME_ZERO;
    clocktime_t newTime = CLOCKTIME_ZERO;
    if (detComIngressTimestamp5GRcvd != -1) {
        // Received from DetCom interface, residence time is DetComResidenceTime
        auto detComResidenceTime = (detComEgressTimestamp5G - detComIngressTimestamp5GRcvd) / clock5GRateRatio; // change detcom residence time to local domain
        auto localResidenceTime = oldLocalTimeAtTimeSync - detComEgressTimestampGptp;
        residenceTime = detComResidenceTime + localResidenceTime;
        EV_INFO << "detComResidenceTime          - " << detComResidenceTime << endl;
        EV_INFO << "localResidenceTime           - " << localResidenceTime << endl;
        EV_INFO << "residenceTime                - " << residenceTime << endl;
            newTime = preciseOriginTimestamp + correctionField + residenceTime;
    }
    else {
        // Received from normal (e.g. eth) port, calculate as always
        residenceTime = oldLocalTimeAtTimeSync - syncIngressTimestamp;
        newTime = preciseOriginTimestamp + correctionField + gmRateRatio * (meanLinkDelay + residenceTime);
        calculateGmRatio();
    }

    ASSERT(gptpNodeType != MASTER_NODE);

    // preciseOriginTimestamp and correctionField are in the grandmaster's time base
    // meanLinkDelay and residence time are in the local time base
    // Thus, we need to multiply the meanLinkDelay and residenceTime with the gmRateRatio

    auto servoClock = check_and_cast<ServoClockBase *>(clock.get());

    // Only change the oscillator if we have new information about our nrr
    // TODO: We should change this to a clock servo model in the future anyways!
    //    ppm newOscillatorCompensation;
    //    if (!hasNewRateRatioForOscillatorCompensation) {
    //        newOscillatorCompensation = unit(piControlClock->getOscillatorCompensation());
    //    }
    //    else {
    //        newOscillatorCompensation =
    //            unit(gmRateRatio * (1 + unit(piControlClock->getOscillatorCompensation()).get()) - 1);
    //        hasNewRateRatioForOscillatorCompensation = false;
    //    }
    servoClock->adjustClockTo(newTime);
    //    EV_INFO << "newOscillatorCompensation " << newOscillatorCompensation << endl;

    newLocalTimeAtTimeSync = clock->getClockTime();

    updateSyncStateAndRescheduleSyncTimeout(servoClock);

    /************** Rate ratio calculation *************************************
     * It is calculated based on interval between two successive Sync messages *
     ***************************************************************************/

    EV_INFO << "LOCAL TIME BEFORE SYNC     - " << oldLocalTimeAtTimeSync << endl;
    EV_INFO << "LOCAL TIME AFTER SYNC      - " << newLocalTimeAtTimeSync << endl;
    EV_INFO << "CALCULATED NEW TIME        - " << newTime << endl;
    if (servoClock->referenceClockModule != nullptr) {
        auto referenceClockTime = servoClock->referenceClockModule->getClockTime();
        auto diffReferenceToOldLocal = oldLocalTimeAtTimeSync - referenceClockTime;
        auto diffReferenceToNewTime = newTime - referenceClockTime;
        EV_INFO << "REFERENCE CLOCK TIME       - " << referenceClockTime << endl;
        EV_INFO << "DIFF REFERENCE TO OLD TIME - " << diffReferenceToOldLocal << endl;
        EV_INFO << "DIFF REFERENCE TO NEW TIME - " << diffReferenceToNewTime << endl;
    }
    EV_INFO << "CURRENT SIMTIME            - " << now << endl;
    EV_INFO << "ORIGIN TIME SYNC           - " << preciseOriginTimestamp << endl;
    EV_INFO << "PREV ORIGIN TIME SYNC      - " << preciseOriginTimestampLast << endl;
    EV_INFO << "SYNC INGRESS TIME          - " << syncIngressTimestamp << endl;
    EV_INFO << "SYNC INGRESS TIME LAST     - " << syncIngressTimestampLast << endl;
    EV_INFO << "RESIDENCE TIME             - " << residenceTime << endl;
    EV_INFO << "CORRECTION FIELD           - " << correctionField << endl;
    EV_INFO << "PROPAGATION DELAY          - " << meanLinkDelay << endl;
    EV_INFO << "TIME DIFFERENCE TO SIMTIME - " << CLOCKTIME_AS_SIMTIME(newLocalTimeAtTimeSync) - now << endl;
    EV_INFO << "NEIGHBOR RATE RATIO        - " << neighborRateRatio << endl;
    EV_INFO << "RECIEVED RATE RATIO        - " << receivedRateRatio << endl;
    EV_INFO << "GM RATE RATIO              - " << gmRateRatio << endl;

    syncIngressTimestampLast = syncIngressTimestamp;
    preciseOriginTimestampLast = preciseOriginTimestamp;

    emit(receivedRateRatioSignal, receivedRateRatio);
    emit(gmRateRatioSignal, gmRateRatio);
    emit(localTimeSignal, CLOCKTIME_AS_SIMTIME(newLocalTimeAtTimeSync));
    emit(timeDifferenceSignal, CLOCKTIME_AS_SIMTIME(newLocalTimeAtTimeSync) - now);
}
void DetComGptp::sendSync()
{
    detComIngressTimestamp5G = detComClock->getClockTime();
    detComIngressTimestampGptp = clock->getClockTime();

    if (isGM()) {
        // When we are selected as the GM, we use the 5G time as the origin timestamp
        preciseOriginTimestamp = detComClock->getClockTime();
    }

    auto packet = new Packet("GptpSync");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto gptp = makeShared<GptpSync>();
    gptp->setDomainNumber(domainNumber);

    gptp->setSequenceId(sequenceId++);
    // Correction field for Sync message is zero for two-step mode
    // See Table 11-6 in IEEE 802.1AS-2020
    // Change when implementing CMLDS
    gptp->setCorrectionField(CLOCKTIME_ZERO);
    packet->insertAtFront(gptp);

    for (auto port : masterPortIds) {
        if (idMatchesSet(slavePortId, detComInterfaces) && idMatchesSet(port, detComInterfaces)) {
            // When we are a slave to another TsnTranslator, we don't send the Sync message to other TsnTranlators
            continue;
        }
        sendPacketToNIC(packet->dup(), port);
    }
    delete packet;

    // The sendFollowUp(portId) called by receiveSignal(), when GptpSync sent
}

void DetComGptp::sendFollowUp(int portId, const GptpSync *sync, const clocktime_t &syncEgressTimestampOwn)
{
    auto packet = new Packet("GptpFollowUp");
    packet->addTag<MacAddressReq>()->setDestAddress(GPTP_MULTICAST_ADDRESS);
    auto gptp = makeShared<GptpFollowUp>();
    gptp->setDomainNumber(domainNumber);
    gptp->setPreciseOriginTimestamp(preciseOriginTimestamp);
    gptp->setSequenceId(sync->getSequenceId());

    clocktime_t residenceTime;
    if (detComIngressTimestamp5GRcvd != -1) {
        if (idMatchesSet(portId, detComInterfaces)) {
            // ttInterface -> ttInterface (not supported)
            throw cRuntimeError("DetCom interface to DetCom interface communication is not supported");
        }
        // ttInterface -> ethInterface
        // Packet was received from DetCom interface, residence time calulation needs to include DetComResidenceTime

        auto detComResidenceTime = (detComEgressTimestamp5G - detComIngressTimestamp5GRcvd) / clock5GRateRatio;
        auto localResidenceTime = syncEgressTimestampOwn - detComEgressTimestampGptp;
        residenceTime = localResidenceTime + detComResidenceTime;

        EV_INFO << "############## SEND FOLLOW_UP ################################" << endl;
        EV_INFO << "SYNC EGRESS                   - " << syncEgressTimestampOwn << endl;
        EV_INFO << "detCom egress gPTP            - " << detComEgressTimestampGptp << endl;
        EV_INFO << "detCom egress 5G              - " << detComEgressTimestamp5G << endl;
        EV_INFO << "detCom ingress rcvd.          - " << detComIngressTimestamp5GRcvd << endl;
        auto newCorrectionField = correctionField + residenceTime;
        gptp->setCorrectionField(newCorrectionField);
    }
    else if (idMatchesSet(portId, detComInterfaces)) {
        if (isGM()) {
            // local -> ttInterface
            residenceTime = detComIngressTimestampGptp - preciseOriginTimestamp;
            gptp->setCorrectionField(residenceTime);
        } else {
            // ethInterface -> ttInterface
            residenceTime = detComIngressTimestampGptp - syncIngressTimestamp;
            auto newCorrectionField = correctionField + gmRateRatio * (meanLinkDelay + residenceTime);
            gptp->setCorrectionField(newCorrectionField);
        }

        auto ingressTimeTag = packet->addTag<DetComIngressTimeTag>();
        ingressTimeTag->setReceptionStarted(detComIngressTimestamp5G);
        ingressTimeTag->setReceptionEnded(detComIngressTimestamp5G);

        // But we also need to set the detComIngressTime
    }
    else {
        if (isGM()) {
            // local -> ethInterface
            residenceTime = syncEgressTimestampOwn - preciseOriginTimestamp;
            gptp->setCorrectionField(residenceTime);
        } else {
            // ethInterface -> ethInterface
            residenceTime = syncEgressTimestampOwn - syncIngressTimestamp;
            auto newCorrectionField = correctionField + gmRateRatio * (meanLinkDelay + residenceTime);
            gptp->setCorrectionField(newCorrectionField);
        }
    }

    emit(residenceTimeSignal, residenceTime.asSimTime());
    emit(correctionFieldEgressSignal, gptp->getCorrectionField().asSimTime());
    gptp->getFollowUpInformationTLVForUpdate().setRateRatio(gmRateRatio);
    packet->insertAtFront(gptp);

    EV_INFO << "############## SEND FOLLOW_UP ################################" << endl;
    EV_INFO << "Correction Field              - " << gptp->getCorrectionField() << endl;
    EV_INFO << "gmRateRatio                   - " << gmRateRatio << endl;
    EV_INFO << "meanLinkDelay                 - " << meanLinkDelay << endl;
    EV_INFO << "residenceTime                 - " << residenceTime << endl;

    sendPacketToNIC(packet, portId);
}

void DetComGptp::handleClockJump(ServoClockBase::ClockJumpDetails *clockJumpDetails)
{
    EV_INFO << "############## Adjust local timestamps #################################" << endl;
    EV_INFO << "BEFORE:" << endl;
    EV_INFO << "detCom ingress gptp          - " << detComIngressTimestampGptp << endl;
    EV_INFO << "detCom egress gptp           - " << detComEgressTimestampGptp << endl;
    EV_INFO << "detCom egress gptp prev      - " << detComEgressTimestampGptpPrev << endl;

    auto timeDiff = clockJumpDetails->newClockTime - clockJumpDetails->oldClockTime;
    adjustLocalTimestamp(detComIngressTimestampGptp, timeDiff);
    adjustLocalTimestamp(detComEgressTimestampGptp, timeDiff);
    adjustLocalTimestamp(detComEgressTimestampGptpPrev, timeDiff);

    EV_INFO << "AFTER:" << endl;
    EV_INFO << "detCom ingress gptp          - " << detComIngressTimestampGptp << endl;
    EV_INFO << "detCom egress gptp           - " << detComEgressTimestampGptp << endl;
    EV_INFO << "detCom egress gptp prev      - " << detComEgressTimestampGptpPrev << endl;
    Gptp::handleClockJump(clockJumpDetails);
}
} // namespace d6g

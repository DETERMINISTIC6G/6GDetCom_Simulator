// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "TimeChunkInserter.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/ProgressTag_m.h"

namespace inet {

//Define_Module(InterpacketGapInserter);
Define_Module(TIMECHUNKINSERTER);

void TimeChunkInserter::initialize(int stage)
{
    ClockUserModuleMixin::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        durationPar = &par("duration");
        timer = new ClockEvent("IfgTimer");
        progress = new ClockEvent("ProgressTimer");
        WATCH(packetStartTime);
        WATCH(packetEndTime);
    }
    // KLUDGE: this runs after the clock stage, the clocks must be initialized
    else if (stage == INITSTAGE_CLOCK + 1) {
        packetEndTime = par("initialChannelBusy") ? getClockTime() : getClockTime().setRaw(INT64_MIN / 2); // INT64_MIN / 2 to prevent overflow
    }
    else if (stage == INITSTAGE_LAST) {
        if (packetEndTime + durationPar->doubleValue() > getClockTime()) {
            double interpacketGapDuration = durationPar->doubleValue();
            rescheduleClockEventAt(packetEndTime + interpacketGapDuration, timer);
            emit(interpacketGapStartedSignal, interpacketGapDuration);
        }
    }
}



void TimeChunkInserter::insertChunk(Packet *packet) {
    Enter_Method("insertChunk");
    ingressTime = packet->getTag<IngressTimeInd>()->getReceptionStarted();

    auto ingressTimeData = makeShared<ByteCountChunk>(B(4),
                                                      ingressTime); // chunk's type should be configured, and convert data type
    packet->insertAtBack(ingressTimeData);
}

void TimeChunkInserter::checkChunk(Packet *packet) {
    Enter_Method("checkChunk");


}

} // namespace inet


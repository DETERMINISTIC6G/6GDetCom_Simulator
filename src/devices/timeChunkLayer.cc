#include "timeChunkLayer.h"

//#include "TimeTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/linklayer/ieee8021as/GptpPacket_m.h"


namespace d6g {
    using namespace inet;

    Define_Module(timeChunkLayer);

    void timeChunkLayer::receiveSignal(cComponent *source, simsignal_t simSignal, cObject *obj, cObject *details){
        auto signal = check_and_cast<cPacket *>(obj);
        auto packet = check_and_cast_nullable<Packet *>(signal->getEncapsulatedPacket());

        if (!packet)
            return;

        auto transmissionId = signal->getTransmissionId();

        if (simSignal == receptionStartedSignal) {
            // Adding this tag directly to the packet is not possible, because this is a different copy of the packet than
            // the one that will be received by the upper layer.
            auto ingressTime = clock->getClockTime();
            ingressTimeMap[transmissionId] = ingressTime;
        }
        else if (simSignal == receptionEndedSignal) {
            auto tag = packet->addTagIfAbsent<IngressTimeInd>();
            tag->setReceptionEnded(getClockTime());
            if (ingressTimeMap.find(transmissionId) != ingressTimeMap.end()) {
                tag->setReceptionStarted(ingressTimeMap[signal->getTransmissionId()]);
                ingressTimeMap.erase(signal->getTransmissionId());
            } else {
                tag->setReceptionStarted(CLOCKTIME_ZERO);
            }
        }
    }

    void timeChunkLayer::initialize(int stage) {
        ClockUserModuleBase::initialize(stage);
        if (stage == INITSTAGE_LINK_LAYER) {
            auto networkInterface = getContainingNicModule(this);  // parent module
            networkInterface->subscribe(receptionStartedSignal, this);
            networkInterface->subscribe(receptionEndedSignal, this);
        }
    }

    void timeChunkLayer::convertTagToChunk(packet_t *packet) {
        // read packet's tag
        //add chunk
        //remove tag
        // TODO - Generated method body
    }
    void
} // namespace inet

//
// Created by haugls on 9/3/24.
//

#ifndef DETERMINISTIC6G_TTPACKETDIRECTIONREVERSER_H
#define DETERMINISTIC6G_TTPACKETDIRECTIONREVERSER_H

#include "inet/linklayer/ethernet/common/PacketDirectionReverser.h"

namespace d6g {
using namespace inet;

class TTPacketDirectionReverser : public PacketDirectionReverser
{
  protected:
    virtual void processPacket(Packet *packet) override;
};

}



#endif // DETERMINISTIC6G_TTPACKETDIRECTIONREVERSER_H

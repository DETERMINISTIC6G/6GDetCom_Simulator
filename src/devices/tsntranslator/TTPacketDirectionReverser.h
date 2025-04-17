// This file is part of Deliverable D4.4 [D44PLACEHOLDER]
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

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

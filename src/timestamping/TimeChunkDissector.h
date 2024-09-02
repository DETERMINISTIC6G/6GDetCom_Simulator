//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE8021QTAGEPDPROTOCOLDISSECTOR_H
#define __INET_IEEE8021QTAGEPDPROTOCOLDISSECTOR_H

#include "inet/common/packet/dissector/ProtocolDissector.h"

namespace d6g {
using namespace inet;

class INET_API TimeChunkDissector : public DefaultProtocolDissector
{
  public:
    virtual void dissect(Packet *packet, const Protocol *protocol, ICallback &callback) const override;
};

} // namespace d6g

#endif

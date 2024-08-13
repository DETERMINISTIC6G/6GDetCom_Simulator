//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.physicallayer.wired.ethernet.EthernetPhyConstants;

import inet.common.TagBase;
import inet.clock.contract.ClockTime;

namespace d6g;
//
// This is a base class for all IEEE 802.3 Ethernet PHY headers defined the IEEE
// Std 802.3-2018, IEEE Standard for Ethernet.
//
class TimeChunk extends inet::FieldsChunk
{
//    chunkLength = 4;
    int64_t receptionStarted;
    int64_t receptionEnded;
}



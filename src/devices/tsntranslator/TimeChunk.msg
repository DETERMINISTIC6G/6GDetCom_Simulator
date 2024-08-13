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

class TimeChunk extends inet::FieldsChunk
{
//    chunkLength = 4;
    int64_t receptionStarted;
    int64_t receptionEnded;
}



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

cplusplus {{
using namespace inet;
}}

class TimeChunk extends inet::FieldsChunk
{
    chunkLength = B(16);
    int64_t receptionStarted;
    int64_t receptionEnded;
}

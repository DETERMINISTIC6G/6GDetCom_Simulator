// This file is part of Deliverable D4.4 [D44PLACEHOLDER]
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import inet.physicallayer.wired.ethernet.EthernetPhyConstants;

import inet.common.TagBase;
import inet.clock.contract.ClockTime;

namespace d6g;

cplusplus {{
using namespace inet;
}}

class DetComTimeChunk extends inet::FieldsChunk
{
    chunkLength = B(18);
    inet::clocktime_t receptionStarted;
    inet::clocktime_t receptionEnded;
    uint16_t typeOrLength;
}

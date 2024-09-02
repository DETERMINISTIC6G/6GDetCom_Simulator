//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "TimeChunkSerializer.h"
#include "TimeChunk_m.h"
#include "inet/common/packet/serializer/ChunkSerializerRegistry.h"


namespace d6g {
    using namespace inet;

Register_Serializer(TimeChunk, TimeChunkSerializer);

void TimeChunkSerializer::serialize(MemoryOutputStream& stream, const Ptr<const Chunk>& chunk) const
{
    const auto& header = staticPtrCast<const TimeChunk>(chunk);
    stream.writeUint16Be(header->getTypeOrLength());
    stream.writeUint64Be(header->getReceptionStarted().raw());
    stream.writeUint64Be(header->getReceptionEnded().raw());
}

const Ptr<Chunk> TimeChunkSerializer::deserialize(MemoryInputStream& stream) const
{
    const auto& header = makeShared<TimeChunk>();
    header->setTypeOrLength(stream.readUint16Be());
    header->setReceptionStarted(stream.readUint64Be());
    header->setReceptionEnded(stream.readUint64Be());
    return header;
}
} // namespace inet


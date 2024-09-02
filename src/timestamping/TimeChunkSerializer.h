//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE8021QTAGHEADERSERIALIZER_H
#define __INET_IEEE8021QTAGHEADERSERIALIZER_H

#include "inet/common/packet/serializer/FieldsChunkSerializer.h"
#include "inet/common/packet/printer/ProtocolPrinter.h"

namespace d6g {
    using namespace inet;

    class TimeChunkSerializer : public FieldsChunkSerializer {
    protected:
        virtual void serialize(MemoryOutputStream &stream, const Ptr<const Chunk> &chunk) const override;

        virtual const Ptr<Chunk> deserialize(MemoryInputStream &stream) const override;

    public:
        TimeChunkSerializer() : FieldsChunkSerializer() {}
    };

} // namespace inet

#endif


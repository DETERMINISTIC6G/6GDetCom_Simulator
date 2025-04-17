// This file is part of Deliverable D4.4 [D44PLACEHOLDER]
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


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


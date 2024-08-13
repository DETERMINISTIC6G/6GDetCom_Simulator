// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef DEVICES_TSNTRANSLATOR_TIMECHUNKINSERTER_H_
#define DEVICES_TSNTRANSLATOR_TIMECHUNKINSERTER_H_

#include "inet/common/clock/ClockUserModuleMixin.h"
#include "inet/queueing/base/PacketPusherBase.h"

namespace d6g {

using namespace inet;
Define_Module(TIMECHUNKINSERTER);

class TimeChunkInserter
{
  protected:
    clocktime_t ingressTime;

  protected:
    virtual void insertChunk(Packet *packet);
    virtual void checkChunk(Packet *packet);

};

} // namespace inet

#endif


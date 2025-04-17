// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "InterfaceFilterMixin.h"

#include "inet/queueing/base/PacketDelayerBase.h"
#include "inet/queueing/base/PacketFlowBase.h"

namespace d6g {
template class InterfaceFilterMixin<inet::queueing::PacketDelayerBase>;
template class InterfaceFilterMixin<inet::queueing::PacketFlowBase>;
}

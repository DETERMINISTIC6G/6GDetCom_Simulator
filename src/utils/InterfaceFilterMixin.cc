//
// Created by haugls on 9/5/24.
//

#include "InterfaceFilterMixin.h"

#include "inet/queueing/base/PacketDelayerBase.h"
#include "inet/queueing/base/PacketFlowBase.h"
#include "inet/linklayer/ieee8021as/Gptp.h"

namespace d6g {
template class InterfaceFilterMixin<inet::queueing::PacketDelayerBase>;
template class InterfaceFilterMixin<inet::queueing::PacketFlowBase>;
//template class InterfaceFilterMixin<inet::Gptp>;
}
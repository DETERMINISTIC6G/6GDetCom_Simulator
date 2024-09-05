#include "DetComGptp.h"

namespace d6g {

Define_Module(DetComGptp);

void DetComGptp::initialize(int stage) {
    InterfaceFilterMixin::initialize(stage);
}

void DetComGptp::sendPdelayReq() {
    // This node is part of the DetCom node.
    // Ensure pDelayReq is only sent to reqInterfaceTypes
    if (idMatchesSet(slavePortId, reqInterfaces)) {
        Gptp::sendPdelayReq();
    } else {
        EV << "Not sending pDelayReq to interface " << slavePortId << " as it is not in reqInterfaceTypes" << endl;
    }
}
} // namespace inet

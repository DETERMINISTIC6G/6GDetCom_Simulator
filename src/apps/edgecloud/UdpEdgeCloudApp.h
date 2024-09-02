// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DETERMINISTIC6G_UDPEDGECLOUDAPP_H_
#define DETERMINISTIC6G_UDPEDGECLOUDAPP_H_

#include <omnetpp.h>
#include "inet/applications/udpapp/UdpEchoApp.h"

using namespace omnetpp;
using namespace inet;
namespace d6g {

/**
 * UDPEdgeCloudApp is based on the UdpEchoApp. It is used to forward packets to another node (or back) and delay them.
 * See the NED file for more info.
 */
class UdpEdgeCloudApp : public UdpEchoApp {
protected:
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

protected:
    L3Address destAddress;
    int destPort;
    cPar *delayParameter = nullptr;
};

} //namespace

#endif

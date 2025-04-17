// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.timestamping;

import d6g.timestamping.TimeChunkEnforcer;
import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketDelayer;


module DetComTimestampingLayer like IProtocolLayer
{
    parameters:
        @display("i=block/layer");
        // volatile double pdc @mutable @unit(s) = default(0s); // pdc: const delay
        // object pdcMapping @mutable = default([]); // per stream delay

        // Arrives at tt (ingress, from other UE or NWTT) --> delay --> bridging logic --> eth (egress)
        ingress.typename = default("");
        egress.typename = default("");
         // We do not know the egress interface at the ingress delayer, so only use indInterfaceType
        // ingress.indInterfaceTypes = ["tt"];

        // ingress.delay = default(this.pdc);
        // ingress.mapping = default(dup(this.pdcMapping));

    // ingress.clockModule = default(has6gClock ? "^.^.^.clock" : "^.pdcclock");
    //ingress.clockModule = default("^.pdcclock");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        residenceTimeCalculator: ResidenceTimeCalculator {
            @display("p=200,150;i=block/blackboard");
        }
        timeChunkEnforcer: TimeChunkEnforcer {
            @display("p=500,150;i=block/blackboard");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> timeChunkEnforcer.in;
        timeChunkEnforcer.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> residenceTimeCalculator.in;
        residenceTimeCalculator.out --> { @display("m=n"); } --> upperLayerOut;
}

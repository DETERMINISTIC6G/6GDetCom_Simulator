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

// TODO: DESCRIPTION
// This module realize the timestamping.
// It defines two input gates (from upperlayer and lowerlayer) and two output gates.
// The submodule residenceTimeCalculator is used to calculate the residence time of the data frame in this layer.
// The submodule timeChunkEnforcer is used to constrain the handle and transport speed forcely, to ensure it follows time gap and time chunk.
module DetComTimestampingLayer like IProtocolLayer
{
    parameters:
        @display("i=block/layer");
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

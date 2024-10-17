//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

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

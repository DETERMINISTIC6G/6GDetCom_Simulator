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

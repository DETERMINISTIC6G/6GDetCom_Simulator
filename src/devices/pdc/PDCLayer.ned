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

package d6g.devices.pdc;

import d6g.timestamping.TimeChunkEnforcer;
import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.contract.IPacketDelayer;


module PDCLayer like IProtocolLayer
{
    parameters:
        @display("i=block/layer");
         volatile double pdcDefault @mutable @unit(s) = default(0s); // const default delay
         object pdcMapping @mutable = default([]); // per stream delay

        // Arrives at tt (ingress, from other UE or NWTT) --> delay --> bridging logic --> eth (egress)
        // We do not know the egress interface at the ingress delayer, so only use indInterfaceType
        pdcDelayer.indInterfaceTypes = ["tt"];
		pdcDelayer.delay = default(this.pdcDefault);
        pdcDelayer.mapping = default(dup(this.pdcMapping));
       	pdcDelayer.clockModule = default("^.^.^.detComClock");
        
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        pdcDelayer: <default("PdcDelayer")> like IPacketDelayer {
            @display("p=200,150;i=block/blackboard");
                
        }
        timeChunkEnforcer: TimeChunkEnforcer {
            @display("p=500,150;i=block/blackboard");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> timeChunkEnforcer.in;
        timeChunkEnforcer.out --> { @display("m=s"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=s"); } --> pdcDelayer.in;
        pdcDelayer.out --> { @display("m=n"); } --> upperLayerOut;
}

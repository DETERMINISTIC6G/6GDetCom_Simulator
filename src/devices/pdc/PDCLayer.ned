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

import inet.protocolelement.processing.IProcessingDelayLayer;
import inet.protocolelement.processing.ProcessingDelayLayer;


module PDCLayer extends ProcessingDelayLayer like IProcessingDelayLayer
{
    parameters:
        @display("i=block/layer");
        volatile double pdcDefault @mutable @unit(s) = default(0s); // const default delay
        object pdcMapping @mutable = default([]); // per stream delay

        ingress.typename = default("PdcDelayer");
        egress.typename = default("");

        // Arrives at tt (ingress, from other UE or NWTT) --> delay --> bridging logic --> eth (egress)
        // We do not know the egress interface at the ingress delayer, so only use indInterfaceType
        ingress.indInterfaceTypes = ["tt"];
        ingress.delay = default(this.pdcDefault);
        ingress.mapping = default(dup(this.pdcMapping));
        ingress.clockModule = default("^.^.^.detComClock");
}

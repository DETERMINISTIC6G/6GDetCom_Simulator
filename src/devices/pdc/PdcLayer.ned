// This file is part of Deliverable D4.4 DetCom Simulator Framework Release 2
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.devices.pdc;

import inet.protocolelement.processing.IProcessingDelayLayer;
import inet.protocolelement.processing.ProcessingDelayLayer;


module PdcLayer extends ProcessingDelayLayer like IProcessingDelayLayer
{
    parameters:
        @display("i=block/layer");
        volatile double defaultPdc @mutable @unit(s) = default(0s); // const default delay
        volatile double defaultJitter @mutable @unit(s) = default(0s); // const default delay
        object mapping @mutable = default([]); // per stream delay

        ingress.typename = default("PdcDelayer");
        egress.typename = default("");

        // Arrives at tt (ingress, from other UE or NWTT) --> delay --> bridging logic --> eth (egress)
        // We do not know the egress interface at the ingress delayer, so only use indInterfaceType
        ingress.indInterfaceTypes = ["tt"];
        ingress.delay = default(this.delay);
        ingress.jitter = default(this.jitter);
        ingress.mapping = default(dup(this.mapping));
        ingress.clockModule = default("^.^.^.detComClock");
}

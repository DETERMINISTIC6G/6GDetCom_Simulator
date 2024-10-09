//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package d6g.delayer;

import inet.protocolelement.processing.IProcessingDelayLayer;
import inet.protocolelement.processing.ProcessingDelayLayer;

// This module is to be used as the delayLayer within a TsnTranslator.
//
// The delayDownlink parameter is used to specify the delay in the downlink direction (from the network to the UE).
// This means, we delay every frame at the earliest possible point in the TsnTranslator
// (directly after arriving from another TsnTranslator), which is at the ingress direction of the parent bridging layer.
// We know that all frames arriving from another TsnTranslator (i.e. a tt interface) are downlink frames.
// Therefore, we set the ingress interface type to "tt" and the delay to delayDownlink.
//
// The delayUplink parameter is used to specify the delay in the uplink direction (from the UE to the network).
// This means, we delay every frame at the latest possible point in the TsnTranslator
// (directly before leaving to another TsnTranslator), which is at the egress direction of the parent bridging layer.
// We know that all frames coming from the UE (i.e. an eth interface) and leaving to another TsnTranslator
// (i.e. a tt interface) are uplink frames.
// Therefore, we set the ingress interface type to "eth" and the egress interface type to "tt"
// and the delay to delayUplink.
//
// TODO: ADD DESCRIPTION OF ALL queueingDelayer PARAMETERS
module TTDelayLayer extends ProcessingDelayLayer like IProcessingDelayLayer
{
    parameters:
        bool queueingDelayer = default(false); // Use QueueingTTDelayer instead of TTDelayer

        volatile double delayUplink @mutable @unit(s) = default(0s); // Uplink Delay
        volatile double delayDownlink @mutable @unit(s) = default(0s); // Downlink Delay

        // Ingress for downlink
        // Arrives at tt (ingress, from other UE or NWTT) --> delay --> bridging logic --> eth (egress)
        ingress.typename = default(queueingDelayer ? "QueueingTTDelayer" : "TTDelayer");
         // We do not know the egress interface at the ingress delayer, so only use indInterfaceType
        ingress.indInterfaceTypes = ["tt"];
        ingress.delay = default(this.delayDownlink);

        // Egress for uplink
        // Arrives at eth (ingress) --> bridging logic --> delay --> tt (egress, to other UE or NWTT)
        egress.typename = default(queueingDelayer ? "QueueingTTDelayer" : "TTDelayer");
        egress.indInterfaceTypes = ["eth"];
         // Normally there should only be one eth interface, so if ind is eth, egress should always be tt
         // However, we don't want to mess up things if someone adds another eth interface, so specify reqInterfaceType regardless
        egress.reqInterfaceTypes = ["tt"];
        egress.delay = default(this.delayUplink);
}

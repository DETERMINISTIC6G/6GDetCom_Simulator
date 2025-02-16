// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.devices.tsntranslator;

import d6g.timestamping.DetComTimeTagging;
import inet.clock.contract.IClock;
import inet.linklayer.contract.IEthernetInterface;
import inet.node.tsn.TsnSwitch;


// This module extends the TsnSwitch module and is to be used within the DetCom node.
// It provides all functionality a full TsnSwitch provides.
//
// Additionally it provides the possibility to specify an uplink and downlink delay.
// It has multiple TTInterface modules, which are connected to the other dstts and the nwtt within the same DetCom node.
module TsnTranslator extends TsnSwitch
{
    parameters:
        int numTtInterfaces = default(0); // Number of other TT interfaces connected to this TSN Translator.
        bool isDstt = default(false); // Specifies whether the TSN Translator is a DS-TT (otherwise it is a NW-TT).
        bool hasPdc = default(false);

        clock.typename = "IdealClock";
        
        eth[*].typename = default("TTEthInterface");

        volatile double delayUplink @mutable @unit(s) = default(0s); // Uplink Delay
        volatile double delayDownlink @mutable @unit(s) = default(0s); // Downlink Delay

        // Chooses the MAC layer type based on whether frame preemption is enabled.
        // "EthernetPreemptingMacLayer" is used for preemptive frames, otherwise "EthernetMacLayer" is used.
        tt[*].macLayer.typename = default(hasFramePreemption ? "EthernetPreemptingMacLayer" : "EthernetMacLayer");

        // Determines the type of queue to be used for egress traffic shaping.
        // If egress traffic shaping is enabled, "Ieee8021qTimeAwareShaper" is used.
        // Otherwise, a standard "PacketQueue" is used.
        tt[*].macLayer.queue.typename = default(hasEgressTrafficShaping ? "Ieee8021qTimeAwareShaper" : "PacketQueue");

        // Chooses the appropriate PHY layer type.
        // "EthernetStreamThroughPhyLayer" is used if cut-through switching is enabled.
        // "EthernetPreemptingPhyLayer" is used if frame preemption is enabled but cut-through is not.
        // "EthernetPhyLayer" is the default PHY layer type.
        tt[*].phyLayer.typename = default("EthernetStreamingPhyLayer");

        bridging.typename = default("TTBridgingLayer");
        bridging.pdc.typename = default(hasPdc ? "PdcLayer" : "");

        bridging.delayLayer.typename = default("TTDelayLayer");
        bridging.delayLayer.delayUplink = default(this.delayUplink);
        bridging.delayLayer.delayDownlink = default(this.delayDownlink);
        eth[*].enableTimeTagging = default(true);
        tt[*].enableTimeTagging = default(false);

        eth[*].timeTagger.clockModule = default(exists(detComClock) ? absPath(".detComClock") : "");

    gates:
        inout ttg[numTtInterfaces] @labels(EtherFrame-conn);
    submodules:
        tt[sizeof(ttg)]: <default("TTInterface")> like IEthernetInterface {
            @display("p=325,900,row,150;q=txQueue");
        }
        timeTagging: DetComTimeTagging {
            @display("p=1100,525");
        }
        detComClock: <default("IdealClock")> like IClock {
            @display("p=200,500;is=s");
        }
    connections:
        for i=0..sizeof(ttg)-1 {
            li.out++ --> tt[i].upperLayerIn;
            li.in++ <-- tt[i].upperLayerOut;
            tt[i].phys <--> { @display("m=s"); } <--> ttg[i];
        }

        timeTagging.lowerLayerOut --> li.in++;
        li.out++ --> timeTagging.lowerLayerIn;

        timeTagging.upperLayerOut --> bl.in++;
        bl.out++ --> timeTagging.upperLayerIn;
}

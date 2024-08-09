// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.devices.tsntranslator;

import inet.linklayer.ethernet.modular.LayeredEthernetInterface;
import inet.node.tsn.TsnSwitch;
import inet.linklayer.contract.IEthernetInterface;


import inet.linklayer.ethernet.contract.IEthernetMacLayer;
import inet.networklayer.common.NetworkInterface;
import inet.physicallayer.wired.ethernet.IEthernetPhyLayer;
import inet.protocolelement.measurement.IMeasurementLayer;
import inet.protocolelement.processing.IProcessingDelayLayer;
import inet.protocolelement.trafficconditioner.ITrafficConditionerLayer;

// This module extends the LayeredEthernetInterface module.
// However, as its purpose is to connect multiple TsnTranslator modules within a DetCom node, no additional delay and
// no ifg should be used there. Thus, the interFrameGapInserter is set to 0s and the bitrate is set to inf Mbps.
// This module only works in combination when connecting it with another TTInterface using the TTChannel module.
//
// See the DetCom to see how this module is implemented.
module TTInterface  extends NetworkInterface like IEthernetInterface {
    parameters:
        double bitrate @unit(bps);
    	phyLayer.interFrameGapInserter.duration = default(0s); // duration of the inter frame gap inserter 
    	bitrate = default(inf Mbps); // bitrate of UE communication link
    	
        string protocol = default("ethernetmac");
        bool broadcast = default(true);
        bool multicast = default(true);
        string address @mutable = default("auto");

        int mtu @unit(B) = default(1500B);
        string interfaceTableModule;
        *.bitrate = default(this.bitrate);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input cutthroughIn @loose;
        output cutthroughOut @loose;
        inout phys @labels(EthernetSignal);
    submodules:
        processingDelayLayer: <default("")> like IProcessingDelayLayer {
            parameters:
                @display("p=400,100");
        }
        trafficConditionerLayer: <default("")> like ITrafficConditionerLayer {
            parameters:
                @display("p=400,200");
        }
        measurementLayer: <default("")> like IMeasurementLayer {
            parameters:
                @display("p=400,300");
        }
        macLayer: <default("EthernetMacLayer")> like IEthernetMacLayer {
            parameters:
                @display("p=400,400");
        }
        timeChunkLayer: TimeChunkLayer {
            parameters:
                @display("p=400,500");
        }
        
        phyLayer: <default("EthernetPhyLayer")> like IEthernetPhyLayer {
            parameters:
                transmitter.clockModule = default("");
                @display("p=400,600");
        }


    connections allowunconnected:
        upperLayerIn --> { @display("m=n"); } --> processingDelayLayer.upperLayerIn;
        processingDelayLayer.lowerLayerOut --> trafficConditionerLayer.upperLayerIn;
        trafficConditionerLayer.lowerLayerOut --> measurementLayer.upperLayerIn;
        measurementLayer.lowerLayerOut --> macLayer.upperLayerIn;
        macLayer.lowerLayerOut --> timeChunkLayer.upperLayerIn;
        timeChunkLayer.lowerLayerOut-->phyLayer.upperLayerIn;
        phyLayer.lowerLayerOut --> { @display("m=s"); } --> phys$o;

        phys$i --> { @display("m=s"); } --> phyLayer.lowerLayerIn;
        phyLayer.upperLayerOut --> timeChunkLayer.lowerLayerIn;
        timeChunkLayer.upperLayerOut --> macLayer.lowerLayerIn;
        macLayer.upperLayerOut --> measurementLayer.lowerLayerIn;
        measurementLayer.upperLayerOut --> trafficConditionerLayer.lowerLayerIn;
        trafficConditionerLayer.upperLayerOut --> processingDelayLayer.lowerLayerIn;
        processingDelayLayer.upperLayerOut --> { @display("m=n"); } --> upperLayerOut;
    	
    	
}




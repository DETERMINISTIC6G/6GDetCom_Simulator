// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


package d6g.devices.tsntranslator;

import d6g.timestamping.TimeChunkEnforcer;
import inet.protocolelement.contract.IProtocolLayer;
import inet.protocolelement.processing.IProcessingDelayLayer;


//
// This module is a copy from the BridgingLayer module of the INET Framework
// However, it additionally includes the delayLayer, which is used to specify the uplink and downlink delays for its
// parent TsnTranslator module
//
module TTBridgingLayer like IProtocolLayer
{
    parameters:
        string macTableModule = default("");
        string interfaceTableModule; // relative module path of the interface table
        *.macTableModule = default(absPath(this.macTableModule));
        *.interfaceTableModule = default(this.interfaceTableModule);
        @display("i=block/layer");
        directionReverser.reverser.typename = "TTPacketDirectionReverser";
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        processingDelayLayer: <default("")> like IProcessingDelayLayer {
            @display("p=300,150");
        }
        directionReverser: <default("DirectionReverserLayer")> like IProtocolLayer {
            @display("p=300,300");
        }

        streamIdentifier: <default("")> like IProtocolLayer {
            @display("p=300,450");
        }
        streamRelay: <default("")> like IProtocolLayer {
            @display("p=300,600");
        }
        streamFilter: <default("")> like IProtocolLayer {
            @display("p=300,750");
        }

        pdcCorrection: <default("PDCLayer")> like IProtocolLayer { // new layer
            //@display("i=block/control;b=50,50,255;p=574,360");
            @display("p=300,900");
        }

        streamCoder: <default("")> like IProtocolLayer {
            @display("p=300,1050");
        }
        interfaceRelay: <default("InterfaceRelayLayer")> like IProtocolLayer {
            @display("p=300,1200");
        }
        vlanPolicy: <default("")> like IProtocolLayer {
            @display("p=300,1350");
        }
        timestampingLayer: <default("DetComTimestampingLayer")> like IProtocolLayer {
            @display("p=300,1500");
        }
        delayLayer: <default("")> like IProcessingDelayLayer {
            @display("p=300,1650");
        }
    connections:
        upperLayerIn --> processingDelayLayer.upperLayerIn;
        processingDelayLayer.lowerLayerOut --> directionReverser.upperLayerIn;
        directionReverser.lowerLayerOut --> streamIdentifier.upperLayerIn;
        streamIdentifier.lowerLayerOut --> streamRelay.upperLayerIn;
        streamRelay.lowerLayerOut --> streamFilter.upperLayerIn;
        streamFilter.lowerLayerOut --> pdcCorrection.upperLayerIn;
        pdcCorrection.lowerLayerOut --> streamCoder.upperLayerIn;
        streamCoder.lowerLayerOut --> interfaceRelay.upperLayerIn;
        interfaceRelay.lowerLayerOut --> vlanPolicy.upperLayerIn;
        vlanPolicy.lowerLayerOut --> timestampingLayer.upperLayerIn;
        timestampingLayer.lowerLayerOut --> delayLayer.upperLayerIn;
        delayLayer.lowerLayerOut --> lowerLayerOut;

        lowerLayerIn --> delayLayer.lowerLayerIn;
        delayLayer.upperLayerOut --> timestampingLayer.lowerLayerIn;
        timestampingLayer.upperLayerOut --> vlanPolicy.lowerLayerIn;
        vlanPolicy.upperLayerOut --> interfaceRelay.lowerLayerIn;
        interfaceRelay.upperLayerOut --> streamCoder.lowerLayerIn;
        streamCoder.upperLayerOut --> pdcCorrection.lowerLayerIn;
        pdcCorrection.upperLayerOut --> streamFilter.lowerLayerIn;
        streamFilter.upperLayerOut --> streamRelay.lowerLayerIn;
        streamRelay.upperLayerOut --> streamIdentifier.lowerLayerIn;
        streamIdentifier.upperLayerOut --> directionReverser.lowerLayerIn;
        directionReverser.upperLayerOut --> processingDelayLayer.lowerLayerIn;
        processingDelayLayer.upperLayerOut --> upperLayerOut;
}

// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later


package d6g.devices.tsntranslator;

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
        pdcCorrection: <default("")> like IProcessingDelayLayer { // new layer
			@display("i=block/control;b=50,50,255");
            @display("p=300,376");
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
        streamCoder: <default("")> like IProtocolLayer {
            @display("p=300,900");
        }
        interfaceRelay: <default("InterfaceRelayLayer")> like IProtocolLayer {
            @display("p=300,1050");
        }
        vlanPolicy: <default("")> like IProtocolLayer {
            @display("p=300,1200");
        }
        delayLayer: <default("")> like IProcessingDelayLayer {
            @display("p=300,1350");
        }
    connections:
        upperLayerIn --> processingDelayLayer.upperLayerIn;
        processingDelayLayer.lowerLayerOut --> directionReverser.upperLayerIn;

        directionReverser.lowerLayerOut --> pdcCorrection.upperLayerIn; // new
        pdcCorrection.lowerLayerOut --> streamIdentifier.upperLayerIn;
        streamIdentifier.lowerLayerOut --> streamRelay.upperLayerIn;

        streamRelay.lowerLayerOut --> streamFilter.upperLayerIn;
        streamFilter.lowerLayerOut --> streamCoder.upperLayerIn;
        streamCoder.lowerLayerOut --> interfaceRelay.upperLayerIn;
        interfaceRelay.lowerLayerOut --> vlanPolicy.upperLayerIn;
        vlanPolicy.lowerLayerOut --> delayLayer.upperLayerIn;
        delayLayer.lowerLayerOut --> lowerLayerOut;

        lowerLayerIn --> delayLayer.lowerLayerIn;
        delayLayer.upperLayerOut --> vlanPolicy.lowerLayerIn;
        vlanPolicy.upperLayerOut --> interfaceRelay.lowerLayerIn;
        interfaceRelay.upperLayerOut --> streamCoder.lowerLayerIn;
        streamCoder.upperLayerOut --> streamFilter.lowerLayerIn;
        streamFilter.upperLayerOut --> streamRelay.lowerLayerIn;

        streamRelay.upperLayerOut --> streamIdentifier.lowerLayerIn; // new
        streamIdentifier.upperLayerOut --> pdcCorrection.lowerLayerIn;
        pdcCorrection.upperLayerOut --> directionReverser.lowerLayerIn;

        directionReverser.upperLayerOut --> processingDelayLayer.lowerLayerIn;
        processingDelayLayer.upperLayerOut --> upperLayerOut;
}

// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.devices.tsntranslator;


module TimeChunkLayer
{
    parameters:
        @display("i=block/layer");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
    submodules:
        timeChunkInserter: TimeChunkInserter {
            @display("p=200,100");
        }
        timeChunkChecker: TimeChunkInserter {
            @display("p=500,100");
        }
    connections:
        upperLayerIn --> { @display("m=n"); } --> timeChunkInserter.in;
        timeChunkInserter.out --> { @display("m=n"); } --> lowerLayerOut;

        lowerLayerIn --> { @display("m=n"); } --> timeChunkChecker.in;
        timeChunkChecker.out --> { @display("m=n"); } --> upperLayerOut;


}

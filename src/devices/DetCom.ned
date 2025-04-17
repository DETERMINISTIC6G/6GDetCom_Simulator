// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.devices;



import d6g.devices.tsntranslator.TTChannel;
import d6g.devices.tsntranslator.TsnTranslator;


// The DetCom node is used as a 6G TSN bridge.
// It consists of multiple TsnTranslator modules, one network TsnTranslator (nwtt) and multiple device side
// TsnTranslators (dstt), specifically one for every UE.
// The behavior of the DetCom node is defined by the TsnTranslator modules, especially using the uplink and downlink
// delay parameters.
//
// Every dstt module is connected to the network TsnTranslator (nwtt) and to the other dstt modules using a TTChannel
// module.
// The TTChannel is a perfect channel with no delay, jitter or loss, as the delay should be defined in the TsnTranslator
// modules.
module DetCom
{
    parameters:
        @display("i=device/antennatower");
        int numUEs = default(0); // Number of connected UEs (specifies number of dstt modules).
        xml delayConfig = default(xml("<delays></delays>")); // XML delay configuration, alternative way to specify the uplink and downlink delays for dstts.
        @class(DetCom);
    gates:
        inout dsttg[numUEs];
        inout nwttg;
    submodules:
        dstt[sizeof(dsttg)]: TsnTranslator {
            isDstt = default(true);
            @display("p=100,100,col,150;");
        }
        nwtt: TsnTranslator {
            @display("p=250,100;");
        }
    connections:
        nwttg <--> nwtt.ethg++;
        for i=0..sizeof(dsttg)-1 {
            dsttg[i] <--> dstt[i].ethg++;
            dstt[i].ttg++ <--> TTChannel <--> nwtt.ttg++;
        }

        for i=0..sizeof(dsttg)-1, for j=i+1..sizeof(dsttg)-1 {
            dstt[i].ttg++ <--> TTChannel <--> dstt[j].ttg++;
        }
}

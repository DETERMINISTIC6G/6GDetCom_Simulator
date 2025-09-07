package d6g.simulations.summerschool.task1;

import d6g.simulations.summerschool.BabblingTalker;
import d6g.simulations.summerschool.BurstyTalker;
import inet.networks.base.TsnNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;


network NetworkTask1 extends TsnNetworkBase
{
    parameters:
        *.eth[*].bitrate = default(100Mbps);
        @display("bgb=919,863");
    submodules:
        periodicTalker: TsnDevice {
            @display("p=400,300");
        }
        burstyTalker: BurstyTalker {
            @display("p=400,400");
        }
        babblingTalker: BabblingTalker {
            @display("p=400,500");
        }
        bridge: TsnSwitch {
            @display("p=600,400");
        }
        listener: TsnDevice {
            @display("p=800,400");
        }

    connections:
        periodicTalker.ethg++ <--> EthernetLink <--> bridge.ethg++;
        burstyTalker.ethg++ <--> EthernetLink <--> bridge.ethg++;
        babblingTalker.ethg++ <--> EthernetLink <--> bridge.ethg++;
}

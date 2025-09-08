package d6g.simulations.summerschool.task2;

import d6g.distribution.histogram.HistogramContainer;
import inet.networks.base.TsnNetworkBase;
import inet.node.ethernet.EthernetLink;
import inet.node.tsn.TsnDevice;
import inet.node.tsn.TsnSwitch;


network NetworkTask2 extends TsnNetworkBase
{
    parameters:
        *.eth[*].bitrate = default(100Mbps);
        detCom.*.eth[*].bitrate = default(100Mbps);
    submodules:
        histogramContainer: HistogramContainer {
            @display("p=100,800");
        }
        talker1: TsnDevice {
            @display("p=400,300");
        }
        talker2: TsnDevice {
            @display("p=400,500");
        }
        detCom: TsnSwitch {
            @display("p=550,400");
        }
        bridge: TsnSwitch {
            @display("p=700,400");
        }
        listener: TsnDevice {
            @display("p=850,400");
        }
    connections:
        talker1.ethg++ <--> EthernetLink <--> detCom.ethg++;
        talker2.ethg++ <--> EthernetLink <--> detCom.ethg++;
        detCom.ethg++ <--> EthernetLink <--> bridge.ethg++;
        bridge.ethg++ <--> EthernetLink <--> listener.ethg++;
}

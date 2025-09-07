package d6g.simulations.summerschool.solution2;

import d6g.devices.DetCom;
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
        detCom: DetCom {
            @display("p=550,400");
        }
        bridge: TsnSwitch {
            @display("p=700,400");
        }
        listener: TsnDevice {
            @display("p=850,400");
        }
    connections:
        talker1.ethg++ <--> EthernetLink <--> detCom.dsttg++;
        talker2.ethg++ <--> EthernetLink <--> detCom.dsttg++;
        detCom.nwttg <--> EthernetLink <--> bridge.ethg++;
        bridge.ethg++ <--> EthernetLink <--> listener.ethg++;
}

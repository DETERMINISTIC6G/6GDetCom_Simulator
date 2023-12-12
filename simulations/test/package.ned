// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.simulations.test;

import d6g.distribution.delayreplayer.DelayReplayerContainer;
import d6g.distribution.histogram.HistogramContainer;
import inet.networks.base.TsnNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;


@license(LGPL);

//
// An extension of the TsnNetworkBase from the INET framework.
// The network consists of a client, a switch, and a server.
// A Histogram submodule is also included for statistical analysis, with data sourced from example-hist.xml.
//
network TsnTestNetwork extends TsnNetworkBase
{
    parameters:
        *.eth[*].bitrate = default(100Mbps); // Default bitrate for all Ethernet interfaces is set to 100Mbps.
    submodules:
        // Histogram for capturing statistical data.
        histogramContainer: HistogramContainer {
            @display("p=100,400;is=s");
        }

        delayreplayerContainer: DelayReplayerContainer{
            @display("p=100,500;is=s");
        }

        // Client node in the network, likely generating TSN traffic.
        client: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=350,200");
        }

        // Switch node for routing and handling TSN traffic.
        switch: <default("TsnSwitch")> like IEthernetNetworkNode {
            @display("p=550,200");
        }

        // Server node in the network, typically a recipient of TSN traffic.
        server: <default("TsnDevice")> like IEthernetNetworkNode {
            @display("p=750,200");
        }
    connections:
        client.ethg++ <--> EthernetLink <--> switch.ethg++;
        switch.ethg++ <--> EthernetLink <--> server.ethg++;
}

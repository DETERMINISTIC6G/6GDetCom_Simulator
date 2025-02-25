// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.networks;

import inet.networks.base.TsnNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.EthernetLink;

//
// This network extends the TsnNetworkBase but predefines some parameters.
// This might need some changes!
network DetComNetworkBase extends TsnNetworkBase 
{
    parameters:
        // Setting the bitrate of all Ethernet interfaces to a default value of 100Mbps.
        // If not explicitly defined elsewhere, the default value will be used.
        *.eth[*].bitrate = default(1Gbps);
        detCom.*.eth[*].bitrate = default(1Gbps);
        **.eth[*].phyLayer.typename = default("EthernetStreamingPhyLayer");
        
        
		**.hasEgressTrafficShaping = default(true); // Enabling egress traffic shaping for all nodes in the network.
		
		
		**.hasIncomingStreams = default(true); // Specifying that all nodes in the network have incoming streams.
		
		
		**.eth[*].macLayer.queue.numTrafficClasses = default(2); // Specifying the number of traffic classes at the MAC layer for all Ethernet interfaces.
}
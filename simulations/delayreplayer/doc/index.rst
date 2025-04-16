Delayreplayer Showcase
======================

To simulate real-world delay patterns in communication networks, we implemented a new component for the simulator.
This showcase demonstrates how to use the :ned:`DelayReplayer` and :ned:`DelayReplayerContainer` components to replay delay traces in a simulation scenario.

Showcase Design and Implementation
----------------------------------

The following figure shows the design of the showcase scenario.

.. image:: Delayreplayer_TsnTestNetwork.png

The network consists of two devices connected via switches and a wireless link, as defined in the TsnTestNetwork. The DelayReplayerContainer module is included to provide delay patterns from CSV files. 

The following code snippet shows the configuration of the DelayReplayerContainer and how it is used to apply delay patterns to the network communication:

.. literalinclude:: ../omnetpp.ini
    :language: ini
    :start-at: delayreplayerContainer
    :end-at: delayDownlink

The DelayReplayerContainer loads CSV files with delay traces and provides them as random number providers. The detCom module uses these providers to apply realistic delay patterns to uplink and downlink communication. 

Results
-------

The simulation results show the impact of applying real-world delay traces to network communication. The histograms below display the distribution of packet delays in both uplink and downlink directions.

+-----------+------------+
| |uplink|  | |downlink| |
+-----------+------------+

.. |uplink| image:: uplink.png
   :width: 100%

.. |downlink| image:: downlink.png
   :width: 100%
   
The uplink histogram shows a concentrated delay distribution centered around 0.65ms, while the downlink histogram displays a bimodal distribution with peaks at approximately 0.5ms and 0.7ms. These distributions reflect the characteristics of the delay traces provided by the DelayReplayerContainer, demonstrating how real-world network conditions can be simulated using recorded delay patterns.

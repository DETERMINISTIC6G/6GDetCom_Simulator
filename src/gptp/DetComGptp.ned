package d6g.gptp;

import inet.linklayer.ieee8021as.Gptp;


//
// This module implements the IEEE 802.1as protocol also known as gPTP. It
// measures link delays to neighboring gPTP network nodes periodically. The
// slave and master ports specify where the connected gPTP network nodes
// and their roles in the time synchronization domain. The time synchronization
// is done periodically and the clock module is set.
//
simple DetComGptp extends Gptp
{
    parameters:
        @class(DetComGptp);
        object detComInterfaceTypes = default(["tt"]); // Interfaces with 5G specific behavior
}

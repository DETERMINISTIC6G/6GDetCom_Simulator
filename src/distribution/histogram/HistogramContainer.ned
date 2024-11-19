// This file is part of Deliverable D4.1 DetCom Simulator Framework Release 1
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

package d6g.distribution.histogram;

//
// This module extends the Histograms module to support multiple Histograms from various XML files.
// It allows for the aggregation and management of multiple Histogram objects, each configured with its own XML file.
//
// <pre>
// histograms object:
// {"key1": "path/to/histogram1.xml",
//  "key2": "path/to/histogram2.xml",
// ...}
// </pre>
simple HistogramContainer
{
    parameters:
        volatile object histograms @mutable = default({});
        @class(HistogramContainer);
        @display("i=block/delay");
}

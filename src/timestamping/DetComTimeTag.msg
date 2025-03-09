// This file is part of Deliverable D4.4 [D44PLACEHOLDER]
// of the DETERMINISTIC6G project receiving funding from the
// European Union’s Horizon Europe research and innovation programme
// under Grant Agreement No. 101096504.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

import inet.common.TagBase;
import inet.clock.contract.ClockTime;

namespace d6g;

cplusplus {{
using namespace inet;
}}

class DetComIngressTimeTag extends inet::TagBase
{
    inet::clocktime_t receptionStarted;
    inet::clocktime_t receptionEnded;
}

class DetComResidenceTimeTag extends inet::TagBase
{
    inet::clocktime_t residenceTime;
}

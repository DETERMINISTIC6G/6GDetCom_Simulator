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

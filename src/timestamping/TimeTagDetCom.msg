import inet.common.TagBase;
import inet.clock.contract.ClockTime;

namespace d6g;

cplusplus {{
using namespace inet;
}}

class DetComIngressTime extends inet::TagBase
{
    inet::clocktime_t receptionStarted;
    inet::clocktime_t receptionEnded;
}

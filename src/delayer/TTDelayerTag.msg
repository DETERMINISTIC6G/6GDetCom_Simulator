import inet.common.TagBase;
import inet.clock.contract.ClockTime;

namespace d6g;

cplusplus {{
using namespace inet;
}}

class TTDelayerTag extends inet::TagBase
{
    inet::clocktime_t enqueueTime;
}

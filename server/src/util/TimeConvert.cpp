#include "TimeConvert.h"
void convertTimePointToTimestamp(const std::chrono::system_clock::time_point& tp,google::protobuf::Timestamp* timestamp) {
    if (!timestamp) return;
    auto duration = tp.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - seconds);
    timestamp->set_seconds(seconds.count());
    timestamp->set_nanos(static_cast<int32_t>(nanoseconds.count()));
}
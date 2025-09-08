#pragma once
#include <google/protobuf/util/time_util.h>
#include <chrono>
void convertTimePointToTimestamp(const std::chrono::system_clock::time_point& tp,google::protobuf::Timestamp* timestamp);
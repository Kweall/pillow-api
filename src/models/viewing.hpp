#pragma once

#include <string>
#include <chrono>
#include <optional>

namespace pillow {

enum class ViewingStatus {
    SCHEDULED,
    CONFIRMED,
    CANCELLED,
    COMPLETED
};

struct Viewing {
    std::string id;
    std::string property_id;
    std::string user_id;
    std::chrono::system_clock::time_point scheduled_time;
    ViewingStatus status;
    std::optional<std::string> notes;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
};

struct ViewingCreateRequest {
    std::string property_id;
    std::string scheduled_time; // ISO 8601 format
    std::optional<std::string> notes;
};

struct ViewingResponse {
    std::string id;
    std::string property_id;
    std::string user_id;
    std::string scheduled_time;
    std::string status;
    std::optional<std::string> notes;
    std::string created_at;
};

struct ViewingUpdateRequest {
    std::optional<std::chrono::system_clock::time_point> scheduled_time;
    std::optional<ViewingStatus> status;
    std::optional<std::string> notes;
};

} // namespace pillow
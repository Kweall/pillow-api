#pragma once

#include <string>
#include <vector>
#include <optional>
#include <chrono>

namespace pillow {

enum class PropertyType {
    APARTMENT,
    HOUSE,
    COMMERCIAL,
    LAND
};

enum class PropertyStatus {
    AVAILABLE,
    SOLD,
    RENTED,
    UNAVAILABLE
};

struct Property {
    std::string id;
    std::string title;
    std::string description;
    PropertyType type;
    PropertyStatus status;
    double price;
    std::string city;
    std::string address;
    double area;
    int rooms;
    std::string owner_id;
    std::vector<std::string> images;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
};

struct PropertyCreateRequest {
    std::string title;
    std::string description;
    PropertyType type;
    double price;
    std::string city;
    std::string address;
    double area;
    int rooms;
    std::vector<std::string> images;
};

struct PropertyUpdateRequest {
    std::optional<std::string> title;
    std::optional<std::string> description;
    std::optional<PropertyType> type;
    std::optional<PropertyStatus> status;
    std::optional<double> price;
    std::optional<std::string> city;
    std::optional<std::string> address;
    std::optional<double> area;
    std::optional<int> rooms;
    std::optional<std::vector<std::string>> images;
};

struct PropertyResponse {
    std::string id;
    std::string title;
    std::string description;
    std::string type;
    std::string status;
    double price;
    std::string city;
    std::string address;
    double area;
    int rooms;
    std::string owner_id;
    std::vector<std::string> images;
    std::string created_at;
};

struct PropertySearchRequest {
    std::optional<std::string> city;
    std::optional<double> min_price;
    std::optional<double> max_price;
    std::optional<PropertyType> type;
    std::optional<int> min_rooms;
    std::optional<int> max_rooms;
    std::optional<double> min_area;
    std::optional<double> max_area;
    int limit = 20;
    int offset = 0;
};

} // namespace pillow
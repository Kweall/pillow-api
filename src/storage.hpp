#pragma once

#include <unordered_map>
#include <string>

namespace pillow {

struct UserData {
    std::string id;
    std::string username;
    std::string password_hash;
    std::string email;
    std::string created_at;
};

struct Property {
    std::string id;
    std::string title;
    std::string description;
    double price;
    std::string city;
    int rooms;
    std::string owner_id;
    std::string created_at;
};

struct Viewing {
    std::string id;
    std::string property_id;
    std::string user_id;
    std::string scheduled_time;
    std::string status;
    std::string notes;
    std::string created_at;
};

// Глобальные хранилища
extern std::unordered_map<std::string, UserData> users;
extern std::unordered_map<std::string, std::string> tokens;
extern std::unordered_map<std::string, Property> properties;
extern std::unordered_map<std::string, Viewing> viewings;
extern int next_property_id;
extern int next_viewing_id;

} // namespace pillow
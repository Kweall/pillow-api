#pragma once

#include <unordered_map>
#include <string>

namespace pillow {

struct UserData {
    std::string username;
    std::string password_hash;
    std::string email;
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

// Глобальное хранилище (объявление)
extern std::unordered_map<std::string, UserData> users;
extern std::unordered_map<std::string, std::string> tokens; // username -> token
extern std::unordered_map<std::string, Property> properties;
extern int next_id;

} // namespace pillow
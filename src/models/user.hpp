#pragma once

#include <string>
#include <optional>
#include <chrono>

namespace pillow {

struct User {
    std::string id;
    std::string email;
    std::string username;
    std::string password_hash; // В реальном проекте хранить хеш
    std::string first_name;
    std::string last_name;
    std::string phone;
    bool is_admin;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point updated_at;
};

struct UserCreateRequest {
    std::string email;
    std::string username;
    std::string password;
    std::string first_name;
    std::string last_name;
    std::string phone;
};

struct UserResponse {
    std::string id;
    std::string email;
    std::string username;
    std::string first_name;
    std::string last_name;
    std::string phone;
    bool is_admin;
    std::string created_at;
};

struct LoginRequest {
    std::string username;
    std::string password;
};

struct LoginResponse {
    std::string access_token;
    std::string token_type;
    int expires_in;
    UserResponse user;
};

} // namespace pillow
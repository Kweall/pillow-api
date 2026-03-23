#include "services/user_service.hpp"
#include <regex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>

namespace pillow {

std::string UserService::GenerateId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()
    ).count();
    
    return "user_" + std::to_string(timestamp) + "_" + std::to_string(dis(gen));
}

std::string UserService::GetCurrentTimeISO() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

bool UserService::IsValidEmail(const std::string& email) {
    const std::regex pattern(R"((\w+)(\.\w+)*@(\w+)(\.\w{2,})+)");
    return std::regex_match(email, pattern);
}

UserData UserService::Register(const std::string& username, const std::string& password, const std::string& email) {
    if (username.length() < 3 || username.length() > 50) {
        throw std::runtime_error("Username must be between 3 and 50 characters");
    }
    
    if (password.length() < 6) {
        throw std::runtime_error("Password must be at least 6 characters");
    }
    
    if (!IsValidEmail(email)) {
        throw std::runtime_error("Invalid email format");
    }
    
    if (users.find(username) != users.end()) {
        throw std::runtime_error("User already exists");
    }
    
    UserData user;
    user.id = GenerateId();
    user.username = username;
    user.password_hash = password;
    user.email = email;
    user.created_at = GetCurrentTimeISO();
    
    users[username] = user;
    
    return user;
}

std::optional<UserData> UserService::Login(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    if (it != users.end() && it->second.password_hash == password) {
        return it->second;
    }
    return std::nullopt;
}

std::string UserService::GenerateToken(const std::string& username) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()
    ).count();
    
    std::string token = "jwt_" + username + "_" + std::to_string(timestamp);
    tokens[username] = token;
    return token;
}

bool UserService::ValidateToken(const std::string& token, std::string& username) {
    if (token.find("jwt_") != 0) {
        return false;
    }
    
    std::string token_content = token.substr(4);
    auto underscore_pos = token_content.rfind('_');
    if (underscore_pos == std::string::npos) {
        return false;
    }
    
    username = token_content.substr(0, underscore_pos);
    
    auto it = users.find(username);
    if (it == users.end()) {
        return false;
    }
    
    auto token_it = tokens.find(username);
    if (token_it == tokens.end()) {
        return false;
    }
    
    return token_it->second == token;
}

std::optional<UserData> UserService::GetUserById(const std::string& id) {
    for (const auto& [username, user] : users) {
        if (user.id == id) {
            return user;
        }
    }
    return std::nullopt;
}

std::optional<UserData> UserService::GetUserByUsername(const std::string& username) {
    auto it = users.find(username);
    if (it != users.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool UserService::UpdateUser(const std::string& id, const std::string& username, const std::string& email) {
    for (auto& [key, user] : users) {
        if (user.id == id) {
            user.username = username;
            user.email = email;
            // Обновляем ключ в мапе
            if (key != username) {
                users.erase(key);
                users[username] = user;
            }
            return true;
        }
    }
    return false;
}

bool UserService::DeleteUser(const std::string& id) {
    for (auto it = users.begin(); it != users.end(); ++it) {
        if (it->second.id == id) {
            users.erase(it);
            return true;
        }
    }
    return false;
}

bool UserService::UserExists(const std::string& username) {
    return users.find(username) != users.end();
}

bool UserService::IsAdmin(const std::string& user_id) {
    // Для простоты, считаем пользователя с id "user_1" админом
    return user_id == "user_1";
}

} // namespace pillow
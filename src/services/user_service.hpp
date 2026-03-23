#pragma once

#include "storage.hpp"
#include <string>
#include <vector>
#include <optional>

namespace pillow {

class UserService {
public:
    UserService() = default;
    
    // Регистрация
    UserData Register(const std::string& username, const std::string& password, const std::string& email);
    
    // Аутентификация
    std::optional<UserData> Login(const std::string& username, const std::string& password);
    std::string GenerateToken(const std::string& username);
    bool ValidateToken(const std::string& token, std::string& username);
    
    // CRUD
    std::optional<UserData> GetUserById(const std::string& id);
    std::optional<UserData> GetUserByUsername(const std::string& username);
    bool UpdateUser(const std::string& id, const std::string& username, const std::string& email);
    bool DeleteUser(const std::string& id);
    
    // Проверки
    bool UserExists(const std::string& username);
    bool IsAdmin(const std::string& user_id);
    
private:
    std::string GenerateId();
    std::string GetCurrentTimeISO();
    bool IsValidEmail(const std::string& email);
};

} // namespace pillow
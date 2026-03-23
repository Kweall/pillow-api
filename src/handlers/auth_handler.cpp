#include "handlers/auth_handler.hpp"
#include "storage.hpp"
#include <userver/formats/json.hpp>
#include <userver/formats/json/serialize.hpp>
#include <string>
#include <regex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace pillow {

// Функция для валидации email
bool IsValidEmail(const std::string& email) {
    const std::regex pattern(R"((\w+)(\.\w+)*@(\w+)(\.\w{2,})+)");
    return std::regex_match(email, pattern);
}

// Генерация токена
std::string GenerateToken(const std::string& username) {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()
    ).count();
    
    // Используем символ, который не может быть в username
    return "jwt_" + username + "_" + std::to_string(timestamp);
}

// Валидация токена
bool ValidateToken(const std::string& token, std::string& username) {
    if (token.find("jwt_") != 0) {
        return false;
    }
    
    std::string token_content = token.substr(4);
    // Ищем ПОСЛЕДНЕЕ подчеркивание, так как в username может быть подчеркивание
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

// Извлечение токена из заголовка
std::string ExtractTokenFromHeader(const userver::server::http::HttpRequest& request) {
    auto auth_header = request.GetHeader("Authorization");
    if (auth_header.empty() || auth_header.find("Bearer ") != 0) {
        return "";
    }
    return auth_header.substr(7);
}

AuthHandler::AuthHandler(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string AuthHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
    
    if (request.GetMethod() != userver::server::http::HttpMethod::kPost) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kMethodNotAllowed);
        userver::formats::json::ValueBuilder builder;
        builder["error"] = "Method not allowed";
        return userver::formats::json::ToString(builder.ExtractValue());
    }
    
    try {
        auto body = userver::formats::json::FromString(request.RequestBody());
        
        if (!body.HasMember("username") || !body.HasMember("password")) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Missing required fields";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        std::string username = body["username"].As<std::string>();
        std::string password = body["password"].As<std::string>();
        
        std::cout << "\n=== Login Attempt ===" << std::endl;
        std::cout << "Username: " << username << std::endl;
        std::cout << "Users map size: " << users.size() << std::endl;
        
        auto it = users.find(username);
        if (it != users.end() && it->second.password_hash == password) {
            std::string token = GenerateToken(username);
            tokens[username] = token;
            
            std::cout << "Login successful for: " << username << std::endl;
            std::cout << "Generated token: " << token << std::endl;
            std::cout << "Tokens map size: " << tokens.size() << std::endl;
            std::cout << "=====================\n" << std::endl;
            
            userver::formats::json::ValueBuilder builder;
            builder["access_token"] = token;
            builder["token_type"] = "Bearer";
            builder["expires_in"] = 3600;
            builder["user"]["id"] = username;
            builder["user"]["username"] = username;
            
            request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        std::cout << "Login failed for: " << username << std::endl;
        std::cout << "=====================\n" << std::endl;
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        userver::formats::json::ValueBuilder builder;
        builder["error"] = "Invalid credentials";
        return userver::formats::json::ToString(builder.ExtractValue());
        
    } catch (const std::exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder builder;
        builder["error"] = "Invalid JSON format";
        return userver::formats::json::ToString(builder.ExtractValue());
    }
}

RegisterHandler::RegisterHandler(const userver::components::ComponentConfig& config,
                                 const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string RegisterHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
    
    if (request.GetMethod() != userver::server::http::HttpMethod::kPost) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kMethodNotAllowed);
        userver::formats::json::ValueBuilder builder;
        builder["error"] = "Method not allowed";
        return userver::formats::json::ToString(builder.ExtractValue());
    }
    
    try {
        auto body = userver::formats::json::FromString(request.RequestBody());
        
        if (!body.HasMember("username") || !body.HasMember("password") || !body.HasMember("email")) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Missing required fields";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        std::string username = body["username"].As<std::string>();
        std::string password = body["password"].As<std::string>();
        std::string email = body["email"].As<std::string>();
        
        if (username.length() < 3 || username.length() > 50) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Username must be between 3 and 50 characters";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        if (password.length() < 6) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Password must be at least 6 characters";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        if (!IsValidEmail(email)) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Invalid email format";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        if (users.find(username) != users.end()) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "User already exists";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        UserData user;
        user.username = username;
        user.password_hash = password;
        user.email = email;
        users[username] = user;
        
        std::cout << "\n=== Registration ===" << std::endl;
        std::cout << "Registered user: " << username << std::endl;
        std::cout << "Total users: " << users.size() << std::endl;
        std::cout << "===================\n" << std::endl;
        
        userver::formats::json::ValueBuilder builder;
        builder["id"] = username;
        builder["username"] = username;
        builder["email"] = email;
        builder["message"] = "User registered successfully";
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
        return userver::formats::json::ToString(builder.ExtractValue());
        
    } catch (const std::exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder builder;
        builder["error"] = "Invalid JSON format";
        return userver::formats::json::ToString(builder.ExtractValue());
    }
}

} // namespace pillow
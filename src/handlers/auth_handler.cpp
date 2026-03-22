#include "handlers/auth_handler.hpp"
#include <userver/formats/json.hpp>
#include <userver/formats/json/serialize.hpp>
#include <unordered_map>
#include <string>
#include <regex>
#include <iomanip>

namespace pillow {

static std::unordered_map<std::string, std::string> users;
static std::unordered_map<std::string, std::string> passwords;

// Простая валидация email
bool IsValidEmail(const std::string& email) {
    const std::regex pattern(R"((\w+)(\.\w+)*@(\w+)(\.\w{2,})+)"); // Простой regex для email
    return std::regex_match(email, pattern);
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
        userver::formats::json::ValueBuilder allowed;
        allowed = userver::formats::json::Type::kArray;
        allowed.PushBack("POST");
        builder["allowed_methods"] = allowed.ExtractValue();
        return userver::formats::json::ToString(builder.ExtractValue());
    }
    
    try {
        auto body = userver::formats::json::FromString(request.RequestBody());
        
        if (!body.HasMember("username") || !body.HasMember("password")) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Missing required fields";
            userver::formats::json::ValueBuilder required;
            required = userver::formats::json::Type::kArray;
            required.PushBack("username");
            required.PushBack("password");
            builder["required"] = required.ExtractValue();
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        std::string username = body["username"].As<std::string>();
        std::string password = body["password"].As<std::string>();
        
        if (username.empty() || password.empty()) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Username and password cannot be empty";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        auto it = passwords.find(username);
        if (it != passwords.end() && it->second == password) {
            userver::formats::json::ValueBuilder builder;
            builder["access_token"] = "token_" + username;
            builder["token_type"] = "Bearer";
            builder["expires_in"] = 3600;
            builder["user"]["id"] = username;
            builder["user"]["username"] = username;
            
            request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        userver::formats::json::ValueBuilder builder;
        builder["error"] = "Invalid credentials";
        return userver::formats::json::ToString(builder.ExtractValue());
        
    } catch (const std::exception& e) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder builder;
        builder["error"] = "Invalid JSON format";
        builder["details"] = e.what();
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
        userver::formats::json::ValueBuilder allowed;
        allowed = userver::formats::json::Type::kArray;
        allowed.PushBack("POST");
        builder["allowed_methods"] = allowed.ExtractValue();
        return userver::formats::json::ToString(builder.ExtractValue());
    }
    
    try {
        auto body = userver::formats::json::FromString(request.RequestBody());
        
        // Проверка обязательных полей
        if (!body.HasMember("username") || !body.HasMember("password") || !body.HasMember("email")) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Missing required fields";
            userver::formats::json::ValueBuilder required;
            required = userver::formats::json::Type::kArray;
            required.PushBack("username");
            required.PushBack("password");
            required.PushBack("email");
            builder["required"] = required.ExtractValue();
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        std::string username = body["username"].As<std::string>();
        std::string password = body["password"].As<std::string>();
        std::string email = body["email"].As<std::string>();
        
        // Валидация
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
        
        // Проверка существования пользователя
        if (users.find(username) != users.end()) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "User already exists";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        // Регистрация
        users[username] = username;
        passwords[username] = password;
        
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
        builder["details"] = e.what();
        return userver::formats::json::ToString(builder.ExtractValue());
    }
}

} // namespace pillow
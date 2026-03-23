#include "handlers/auth_handler.hpp"
#include "services/user_service.hpp"
#include <userver/formats/json.hpp>
#include <userver/formats/json/serialize.hpp>
#include <iostream>

namespace pillow {

static UserService user_service;

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
        
        auto user = user_service.Login(username, password);
        if (user) {
            std::string token = user_service.GenerateToken(username);
            
            userver::formats::json::ValueBuilder builder;
            builder["access_token"] = token;
            builder["token_type"] = "Bearer";
            builder["expires_in"] = 3600;
            builder["user"]["id"] = user->id;
            builder["user"]["username"] = user->username;
            
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
        
        auto user = user_service.Register(username, password, email);
        
        userver::formats::json::ValueBuilder builder;
        builder["id"] = user.id;
        builder["username"] = user.username;
        builder["email"] = user.email;
        builder["message"] = "User registered successfully";
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
        return userver::formats::json::ToString(builder.ExtractValue());
        
    } catch (const std::exception& e) {
        std::string error_msg = e.what();
        
        // Тип ошибки для правильного статуса
        if (error_msg.find("User already exists") != std::string::npos) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kConflict);
        } else {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        }
        
        userver::formats::json::ValueBuilder builder;
        builder["error"] = error_msg;
        return userver::formats::json::ToString(builder.ExtractValue());
    }
}

} // namespace pillow
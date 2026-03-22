#include "handlers/auth_handler.hpp"
#include <userver/formats/json.hpp>
#include <unordered_map>
#include <string>

namespace pillow {

static std::unordered_map<std::string, std::string> users;
static std::unordered_map<std::string, std::string> passwords;

AuthHandler::AuthHandler(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string AuthHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
    
    if (request.GetMethod() != userver::server::http::HttpMethod::kPost) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kMethodNotAllowed);
        return R"({"error": "Method not allowed"})";
    }
    
    auto body = userver::formats::json::FromString(request.RequestBody());
    std::string username = body["username"].As<std::string>();
    std::string password = body["password"].As<std::string>();
    
    if (passwords.find(username) != passwords.end() && passwords[username] == password) {
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
    return R"({"error": "Invalid credentials"})";
}

RegisterHandler::RegisterHandler(const userver::components::ComponentConfig& config,
                                 const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string RegisterHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
    
    if (request.GetMethod() != userver::server::http::HttpMethod::kPost) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kMethodNotAllowed);
        return R"({"error": "Method not allowed"})";
    }
    
    auto body = userver::formats::json::FromString(request.RequestBody());
    std::string username = body["username"].As<std::string>();
    std::string password = body["password"].As<std::string>();
    std::string email = body["email"].As<std::string>();
    
    if (users.find(username) != users.end()) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        return R"({"error": "User already exists"})";
    }
    
    users[username] = username;
    passwords[username] = password;
    
    userver::formats::json::ValueBuilder builder;
    builder["id"] = username;
    builder["username"] = username;
    builder["email"] = email;
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
    return userver::formats::json::ToString(builder.ExtractValue());
}

} // namespace pillow
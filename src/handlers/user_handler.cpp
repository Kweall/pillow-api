#include "handlers/user_handler.hpp"
#include "utils/auth_utils.hpp"
#include "services/user_service.hpp"
#include "services/property_service.hpp"
#include <userver/formats/json.hpp>

namespace pillow {

static UserService user_service;
static PropertyService property_service;

UserHandler::UserHandler(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string UserHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
    
    auto method = request.GetMethod();
    std::string id = request.GetPathArg("id");
    
    if (method == userver::server::http::HttpMethod::kGet) {
        std::string token = ExtractTokenFromHeader(request);
        std::string username;
        
        if (!ValidateToken(token, username)) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Authentication required";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        // Если нет айди, возвращаем информацию о текущем пользователе
        if (id.empty()) {
            auto user = user_service.GetUserByUsername(username);
            if (!user) {
                request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
                userver::formats::json::ValueBuilder builder;
                builder["error"] = "User not found";
                return userver::formats::json::ToString(builder.ExtractValue());
            }
            
            userver::formats::json::ValueBuilder builder;
            builder["id"] = user->id;
            builder["username"] = user->username;
            builder["email"] = user->email;
            builder["created_at"] = user->created_at;
            
            request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        // Получение объектов пользователя
        if (request.GetRequestPath().find("/properties") != std::string::npos) {
            auto properties = property_service.GetUserProperties(id);
            
            userver::formats::json::ValueBuilder builder;
            builder = userver::formats::json::Type::kArray;
            
            for (const auto& prop : properties) {
                userver::formats::json::ValueBuilder item;
                item["id"] = prop.id;
                item["title"] = prop.title;
                item["description"] = prop.description;
                item["price"] = prop.price;
                item["city"] = prop.city;
                item["rooms"] = prop.rooms;
                item["created_at"] = prop.created_at;
                builder.PushBack(item.ExtractValue());
            }
            
            request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        // Получение информации о пользователе по ID (только для админа)
        request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
        userver::formats::json::ValueBuilder builder;
        builder["error"] = "Not implemented";
        return userver::formats::json::ToString(builder.ExtractValue());
    }
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kMethodNotAllowed);
    userver::formats::json::ValueBuilder builder;
    builder["error"] = "Method not allowed";
    return userver::formats::json::ToString(builder.ExtractValue());
}

} // namespace pillow
#include "handlers/property_handler.hpp"
#include "handlers/auth_handler.hpp"
#include "storage.hpp"
#include <userver/formats/json.hpp>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace pillow {

std::string GetCurrentTimeISO() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

PropertyListHandler::PropertyListHandler(const userver::components::ComponentConfig& config,
                                         const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string PropertyListHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
    
    auto method = request.GetMethod();
    
    if (method == userver::server::http::HttpMethod::kGet) {
        userver::formats::json::ValueBuilder builder;
        builder = userver::formats::json::Type::kArray;
        
        std::string city = request.GetArg("city");
        std::string min_price = request.GetArg("min_price");
        std::string max_price = request.GetArg("max_price");
        
        for (const auto& [id, prop] : properties) {
            if (!city.empty() && prop.city != city) continue;
            if (!min_price.empty() && prop.price < std::stod(min_price)) continue;
            if (!max_price.empty() && prop.price > std::stod(max_price)) continue;
            
            userver::formats::json::ValueBuilder item;
            item["id"] = prop.id;
            item["title"] = prop.title;
            item["description"] = prop.description;
            item["price"] = prop.price;
            item["city"] = prop.city;
            item["rooms"] = prop.rooms;
            item["owner_id"] = prop.owner_id;
            item["created_at"] = prop.created_at;
            builder.PushBack(item.ExtractValue());
        }
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
        return userver::formats::json::ToString(builder.ExtractValue());
        
    } else if (method == userver::server::http::HttpMethod::kPost) {
        std::string token = ExtractTokenFromHeader(request);
        std::string username;
        
        if (!ValidateToken(token, username)) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Authentication required";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        try {
            auto body = userver::formats::json::FromString(request.RequestBody());
            
            if (!body.HasMember("title") || !body.HasMember("price") || 
                !body.HasMember("city") || !body.HasMember("rooms")) {
                request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
                userver::formats::json::ValueBuilder builder;
                builder["error"] = "Missing required fields";
                userver::formats::json::ValueBuilder required;
                required = userver::formats::json::Type::kArray;
                required.PushBack("title");
                required.PushBack("price");
                required.PushBack("city");
                required.PushBack("rooms");
                builder["required"] = required.ExtractValue();
                return userver::formats::json::ToString(builder.ExtractValue());
            }
            
            Property prop;
            prop.id = std::to_string(next_id++);
            prop.title = body["title"].As<std::string>();
            prop.description = body.HasMember("description") ? body["description"].As<std::string>() : "";
            prop.price = body["price"].As<double>();
            prop.city = body["city"].As<std::string>();
            prop.rooms = body["rooms"].As<int>();
            prop.owner_id = username;
            prop.created_at = GetCurrentTimeISO();
            
            if (prop.title.empty()) {
                request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
                userver::formats::json::ValueBuilder builder;
                builder["error"] = "Title cannot be empty";
                return userver::formats::json::ToString(builder.ExtractValue());
            }

            if (prop.price <= 0) {
                request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
                userver::formats::json::ValueBuilder builder;
                builder["error"] = "Price must be greater than 0";
                return userver::formats::json::ToString(builder.ExtractValue());
            }

            if (prop.rooms < 1 || prop.rooms > 10) {
                request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
                userver::formats::json::ValueBuilder builder;
                builder["error"] = "Rooms must be between 1 and 10";
                return userver::formats::json::ToString(builder.ExtractValue());
            }

            properties[prop.id] = prop;
            
            userver::formats::json::ValueBuilder builder;
            builder["id"] = prop.id;
            builder["title"] = prop.title;
            builder["description"] = prop.description;
            builder["price"] = prop.price;
            builder["city"] = prop.city;
            builder["rooms"] = prop.rooms;
            builder["owner_id"] = prop.owner_id;
            builder["created_at"] = prop.created_at;
            builder["message"] = "Property created successfully";
            
            request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
            return userver::formats::json::ToString(builder.ExtractValue());
            
        } catch (const std::exception& e) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Invalid request";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
    }
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kMethodNotAllowed);
    userver::formats::json::ValueBuilder builder;
    builder["error"] = "Method not allowed";
    return userver::formats::json::ToString(builder.ExtractValue());
}

} // namespace pillow
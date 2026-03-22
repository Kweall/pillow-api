#include "handlers/property_handler.hpp"
#include "handlers/auth_handler.hpp"
#include <userver/formats/json.hpp>
#include <unordered_map>
#include <string>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace pillow {

// Структура и хранилище должны быть доступны из property_item_handler
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

std::unordered_map<std::string, Property> properties;
static int next_id = 1;

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
        // GET /api/properties - список
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
        // POST /api/properties - создание
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
            
            if (prop.title.empty() || prop.price <= 0 || prop.rooms <= 0 || prop.rooms > 10) {
                request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
                userver::formats::json::ValueBuilder builder;
                builder["error"] = "Invalid property data";
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
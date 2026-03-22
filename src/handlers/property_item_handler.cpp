#include "handlers/property_item_handler.hpp"
#include "handlers/auth_handler.hpp"
#include <userver/formats/json.hpp>
#include <unordered_map>
#include <string>

namespace pillow {

// Используем ту же структуру и хранилище что и в property_handler
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

// Внешняя ссылка на хранилище (определено в property_handler.cpp)
extern std::unordered_map<std::string, Property> properties;

PropertyItemHandler::PropertyItemHandler(const userver::components::ComponentConfig& config,
                                         const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string PropertyItemHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
    
    auto method = request.GetMethod();
    std::string id = request.GetPathArg("id");
    
    if (id.empty()) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
        userver::formats::json::ValueBuilder builder;
        builder["error"] = "Property ID required";
        return userver::formats::json::ToString(builder.ExtractValue());
    }
    
    if (method == userver::server::http::HttpMethod::kGet) {
        // GET /api/properties/{id}
        auto it = properties.find(id);
        if (it == properties.end()) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Property not found";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        userver::formats::json::ValueBuilder builder;
        builder["id"] = it->second.id;
        builder["title"] = it->second.title;
        builder["description"] = it->second.description;
        builder["price"] = it->second.price;
        builder["city"] = it->second.city;
        builder["rooms"] = it->second.rooms;
        builder["owner_id"] = it->second.owner_id;
        builder["created_at"] = it->second.created_at;
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
        return userver::formats::json::ToString(builder.ExtractValue());
        
    } else if (method == userver::server::http::HttpMethod::kPut) {
        // PUT /api/properties/{id}
        std::string token = ExtractTokenFromHeader(request);
        std::string username;
        
        if (!ValidateToken(token, username)) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Authentication required";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        auto it = properties.find(id);
        if (it == properties.end()) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Property not found";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        if (it->second.owner_id != username) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kForbidden);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "You don't have permission to update this property";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        try {
            auto body = userver::formats::json::FromString(request.RequestBody());
            Property& prop = it->second;
            
            if (body.HasMember("title")) {
                std::string title = body["title"].As<std::string>();
                if (!title.empty()) prop.title = title;
            }
            if (body.HasMember("description")) {
                prop.description = body["description"].As<std::string>();
            }
            if (body.HasMember("price")) {
                double price = body["price"].As<double>();
                if (price > 0) prop.price = price;
            }
            if (body.HasMember("city")) {
                std::string city = body["city"].As<std::string>();
                if (!city.empty()) prop.city = city;
            }
            if (body.HasMember("rooms")) {
                int rooms = body["rooms"].As<int>();
                if (rooms > 0 && rooms <= 10) prop.rooms = rooms;
            }
            
            userver::formats::json::ValueBuilder builder;
            builder["id"] = prop.id;
            builder["title"] = prop.title;
            builder["description"] = prop.description;
            builder["price"] = prop.price;
            builder["city"] = prop.city;
            builder["rooms"] = prop.rooms;
            builder["owner_id"] = prop.owner_id;
            builder["created_at"] = prop.created_at;
            builder["message"] = "Property updated successfully";
            
            request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ToString(builder.ExtractValue());
            
        } catch (const std::exception& e) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Invalid request";
            builder["details"] = e.what();
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
    } else if (method == userver::server::http::HttpMethod::kDelete) {
        // DELETE /api/properties/{id}
        std::string token = ExtractTokenFromHeader(request);
        std::string username;
        
        if (!ValidateToken(token, username)) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Authentication required";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        auto it = properties.find(id);
        if (it == properties.end()) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Property not found";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        if (it->second.owner_id != username) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kForbidden);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "You don't have permission to delete this property";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        properties.erase(it);
        
        userver::formats::json::ValueBuilder builder;
        builder["message"] = "Property deleted successfully";
        builder["id"] = id;
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
        return userver::formats::json::ToString(builder.ExtractValue());
    }
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kMethodNotAllowed);
    userver::formats::json::ValueBuilder builder;
    builder["error"] = "Method not allowed";
    return userver::formats::json::ToString(builder.ExtractValue());
}

} // namespace pillow
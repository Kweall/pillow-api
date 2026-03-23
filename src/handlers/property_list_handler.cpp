#include "handlers/property_handler.hpp"
#include "utils/auth_utils.hpp"
#include "services/property_service.hpp"
#include <userver/formats/json.hpp>

namespace pillow {

static PropertyService property_service;

PropertyListHandler::PropertyListHandler(const userver::components::ComponentConfig& config,
                                         const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string PropertyListHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
    
    auto method = request.GetMethod();
    
    if (method == userver::server::http::HttpMethod::kGet) {
        std::string city = request.GetArg("city");
        std::string min_price = request.GetArg("min_price");
        std::string max_price = request.GetArg("max_price");
        
        double min_p = min_price.empty() ? 0 : std::stod(min_price);
        double max_p = max_price.empty() ? 0 : std::stod(max_price);
        
        auto properties = property_service.SearchProperties(city, min_p, max_p);
        
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
                return userver::formats::json::ToString(builder.ExtractValue());
            }
            
            std::string title = body["title"].As<std::string>();
            std::string description = body.HasMember("description") ? body["description"].As<std::string>() : "";
            double price = body["price"].As<double>();
            std::string city = body["city"].As<std::string>();
            int rooms = body["rooms"].As<int>();
            
            auto prop = property_service.CreateProperty(title, description, price, city, rooms, username);
            
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
            builder["error"] = e.what();
            return userver::formats::json::ToString(builder.ExtractValue());
        }
    }
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kMethodNotAllowed);
    userver::formats::json::ValueBuilder builder;
    builder["error"] = "Method not allowed";
    return userver::formats::json::ToString(builder.ExtractValue());
}

} // namespace pillow
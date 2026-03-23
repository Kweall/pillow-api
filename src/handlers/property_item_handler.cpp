#include "handlers/property_item_handler.hpp"
#include "utils/auth_utils.hpp"
#include "services/property_service.hpp"
#include <userver/formats/json.hpp>

namespace pillow {

static PropertyService property_service;

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
        auto prop = property_service.GetPropertyById(id);
        if (!prop) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Property not found";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        userver::formats::json::ValueBuilder builder;
        builder["id"] = prop->id;
        builder["title"] = prop->title;
        builder["description"] = prop->description;
        builder["price"] = prop->price;
        builder["city"] = prop->city;
        builder["rooms"] = prop->rooms;
        builder["owner_id"] = prop->owner_id;
        builder["created_at"] = prop->created_at;
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
        return userver::formats::json::ToString(builder.ExtractValue());
        
    } else if (method == userver::server::http::HttpMethod::kPut) {
        std::string token = ExtractTokenFromHeader(request);
        std::string username;
        
        if (!ValidateToken(token, username)) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Authentication required";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        // Проверяем существование объекта
        auto prop = property_service.GetPropertyById(id);
        if (!prop) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Property not found";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        // Проверяем права доступа
        if (prop->owner_id != username) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kForbidden);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "You don't have permission to update this property";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        try {
            auto body = userver::formats::json::FromString(request.RequestBody());
            
            std::string title = body.HasMember("title") ? body["title"].As<std::string>() : "";
            std::string description = body.HasMember("description") ? body["description"].As<std::string>() : "";
            double price = body.HasMember("price") ? body["price"].As<double>() : 0;
            std::string city = body.HasMember("city") ? body["city"].As<std::string>() : "";
            int rooms = body.HasMember("rooms") ? body["rooms"].As<int>() : 0;
            
            // Вызываем метод обновления
            bool updated = property_service.UpdateProperty(id, title, description, price, city, rooms, username);
            
            if (!updated) {
                request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
                userver::formats::json::ValueBuilder builder;
                builder["error"] = "Property not found";
                return userver::formats::json::ToString(builder.ExtractValue());
            }
            
            auto updated_prop = property_service.GetPropertyById(id);
            
            userver::formats::json::ValueBuilder builder;
            builder["id"] = updated_prop->id;
            builder["title"] = updated_prop->title;
            builder["description"] = updated_prop->description;
            builder["price"] = updated_prop->price;
            builder["city"] = updated_prop->city;
            builder["rooms"] = updated_prop->rooms;
            builder["owner_id"] = updated_prop->owner_id;
            builder["created_at"] = updated_prop->created_at;
            builder["message"] = "Property updated successfully";
            
            request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ToString(builder.ExtractValue());
            
        } catch (const std::exception& e) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = e.what();
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
    } else if (method == userver::server::http::HttpMethod::kDelete) {
        std::string token = ExtractTokenFromHeader(request);
        std::string username;
        
        if (!ValidateToken(token, username)) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Authentication required";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        // Проверяем существование объекта
        auto prop = property_service.GetPropertyById(id);
        if (!prop) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Property not found";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        // Проверяем права доступа
        if (prop->owner_id != username) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kForbidden);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "You don't have permission to delete this property";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        // Вызываем метод удаления
        bool deleted = property_service.DeleteProperty(id, username);
        
        if (!deleted) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Property not found";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
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
#include "handlers/viewing_handler.hpp"
#include "utils/auth_utils.hpp"
#include "services/viewing_service.hpp"
#include "services/property_service.hpp"
#include <userver/formats/json.hpp>

namespace pillow {

static ViewingService viewing_service;
static PropertyService property_service;

ViewingHandler::ViewingHandler(const userver::components::ComponentConfig& config,
                               const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string ViewingHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
    
    auto method = request.GetMethod();
    std::string id = request.GetPathArg("id");
    
    std::string token = ExtractTokenFromHeader(request);
    std::string username;
    
    if (!ValidateToken(token, username)) {
        request.SetResponseStatus(userver::server::http::HttpStatus::kUnauthorized);
        userver::formats::json::ValueBuilder builder;
        builder["error"] = "Authentication required";
        return userver::formats::json::ToString(builder.ExtractValue());
    }
    
    if (method == userver::server::http::HttpMethod::kGet) {
        if (!id.empty()) {
            auto viewing = viewing_service.GetViewingById(id);
            if (!viewing) {
                request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
                userver::formats::json::ValueBuilder builder;
                builder["error"] = "Viewing not found";
                return userver::formats::json::ToString(builder.ExtractValue());
            }
            
            if (viewing->user_id != username) {
                request.SetResponseStatus(userver::server::http::HttpStatus::kForbidden);
                userver::formats::json::ValueBuilder builder;
                builder["error"] = "Access denied";
                return userver::formats::json::ToString(builder.ExtractValue());
            }
            
            userver::formats::json::ValueBuilder builder;
            builder["id"] = viewing->id;
            builder["property_id"] = viewing->property_id;
            builder["user_id"] = viewing->user_id;
            builder["scheduled_time"] = viewing->scheduled_time;
            builder["status"] = viewing->status;
            if (!viewing->notes.empty()) builder["notes"] = viewing->notes;
            builder["created_at"] = viewing->created_at;
            
            request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        auto viewings = viewing_service.GetUserViewings(username);
        
        userver::formats::json::ValueBuilder builder;
        builder = userver::formats::json::Type::kArray;
        
        for (const auto& viewing : viewings) {
            userver::formats::json::ValueBuilder item;
            item["id"] = viewing.id;
            item["property_id"] = viewing.property_id;
            item["user_id"] = viewing.user_id;
            item["scheduled_time"] = viewing.scheduled_time;
            item["status"] = viewing.status;
            if (!viewing.notes.empty()) item["notes"] = viewing.notes;
            item["created_at"] = viewing.created_at;
            builder.PushBack(item.ExtractValue());
        }
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
        return userver::formats::json::ToString(builder.ExtractValue());
        
    } else if (method == userver::server::http::HttpMethod::kPost) {
        try {
            auto body = userver::formats::json::FromString(request.RequestBody());
            
            if (!body.HasMember("property_id") || !body.HasMember("scheduled_time")) {
                request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
                userver::formats::json::ValueBuilder builder;
                builder["error"] = "Missing required fields";
                return userver::formats::json::ToString(builder.ExtractValue());
            }
            
            std::string property_id = body["property_id"].As<std::string>();
            std::string scheduled_time = body["scheduled_time"].As<std::string>();
            std::string notes = body.HasMember("notes") ? body["notes"].As<std::string>() : "";
            
            if (!property_service.PropertyExists(property_id)) {
                request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
                userver::formats::json::ValueBuilder builder;
                builder["error"] = "Property not found";
                return userver::formats::json::ToString(builder.ExtractValue());
            }
            
            auto viewing = viewing_service.CreateViewing(property_id, username, scheduled_time, notes);
            
            userver::formats::json::ValueBuilder builder;
            builder["id"] = viewing.id;
            builder["property_id"] = viewing.property_id;
            builder["user_id"] = viewing.user_id;
            builder["scheduled_time"] = viewing.scheduled_time;
            builder["status"] = viewing.status;
            if (!viewing.notes.empty()) builder["notes"] = viewing.notes;
            builder["created_at"] = viewing.created_at;
            builder["message"] = "Viewing scheduled successfully";
            
            request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
            return userver::formats::json::ToString(builder.ExtractValue());
            
        } catch (const std::exception& e) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = e.what();
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
    } else if (method == userver::server::http::HttpMethod::kPut) {
        if (id.empty()) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Viewing ID required";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        try {
            auto body = userver::formats::json::FromString(request.RequestBody());
            
            std::string status = body.HasMember("status") ? body["status"].As<std::string>() : "";
            std::string notes = body.HasMember("notes") ? body["notes"].As<std::string>() : "";
            
            bool updated = viewing_service.UpdateViewing(id, status, notes, username);
            
            if (!updated) {
                request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
                userver::formats::json::ValueBuilder builder;
                builder["error"] = "Viewing not found or access denied";
                return userver::formats::json::ToString(builder.ExtractValue());
            }
            
            userver::formats::json::ValueBuilder builder;
            builder["message"] = "Viewing updated successfully";
            
            request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
            return userver::formats::json::ToString(builder.ExtractValue());
            
        } catch (const std::exception& e) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = e.what();
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
    } else if (method == userver::server::http::HttpMethod::kDelete) {
        if (id.empty()) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kBadRequest);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Viewing ID required";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        bool deleted = viewing_service.DeleteViewing(id, username);
        
        if (!deleted) {
            request.SetResponseStatus(userver::server::http::HttpStatus::kNotFound);
            userver::formats::json::ValueBuilder builder;
            builder["error"] = "Viewing not found or access denied";
            return userver::formats::json::ToString(builder.ExtractValue());
        }
        
        userver::formats::json::ValueBuilder builder;
        builder["message"] = "Viewing cancelled successfully";
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
        return userver::formats::json::ToString(builder.ExtractValue());
    }
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kMethodNotAllowed);
    userver::formats::json::ValueBuilder builder;
    builder["error"] = "Method not allowed";
    return userver::formats::json::ToString(builder.ExtractValue());
}

} // namespace pillow
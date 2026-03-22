#include "handlers/viewing_handler.hpp"
#include <userver/formats/json.hpp>
#include <unordered_map>
#include <string>

namespace pillow {

struct Viewing {
    std::string id;
    std::string property_id;
    std::string user_id;
    std::string scheduled_time;
};

static std::unordered_map<std::string, Viewing> viewings;
static int next_viewing_id = 1;

ViewingHandler::ViewingHandler(const userver::components::ComponentConfig& config,
                               const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string ViewingHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
    
    auto method = request.GetMethod();
    
    if (method == userver::server::http::HttpMethod::kGet) {
        userver::formats::json::ValueBuilder builder;
        builder = userver::formats::json::Type::kArray;
        
        for (const auto& [id, viewing] : viewings) {
            userver::formats::json::ValueBuilder item;
            item["id"] = viewing.id;
            item["property_id"] = viewing.property_id;
            item["user_id"] = viewing.user_id;
            item["scheduled_time"] = viewing.scheduled_time;
            builder.PushBack(item.ExtractValue());
        }
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
        return userver::formats::json::ToString(builder.ExtractValue());
        
    } else if (method == userver::server::http::HttpMethod::kPost) {
        auto body = userver::formats::json::FromString(request.RequestBody());
        
        Viewing viewing;
        viewing.id = std::to_string(next_viewing_id++);
        viewing.property_id = body["property_id"].As<std::string>();
        viewing.user_id = "user_1";
        viewing.scheduled_time = body["scheduled_time"].As<std::string>();
        
        viewings[viewing.id] = viewing;
        
        userver::formats::json::ValueBuilder builder;
        builder["id"] = viewing.id;
        builder["property_id"] = viewing.property_id;
        builder["user_id"] = viewing.user_id;
        builder["scheduled_time"] = viewing.scheduled_time;
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
        return userver::formats::json::ToString(builder.ExtractValue());
    }
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kMethodNotAllowed);
    return R"({"error": "Method not allowed"})";
}

} // namespace pillow
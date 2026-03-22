#include "handlers/property_handler.hpp"
#include <userver/formats/json.hpp>
#include <unordered_map>
#include <string>

namespace pillow {

struct Property {
    std::string id;
    std::string title;
    std::string description;
    double price;
    std::string city;
    int rooms;
};

static std::unordered_map<std::string, Property> properties;
static int next_id = 1;

PropertyHandler::PropertyHandler(const userver::components::ComponentConfig& config,
                                 const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string PropertyHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
    
    auto method = request.GetMethod();
    
    if (method == userver::server::http::HttpMethod::kGet) {
        userver::formats::json::ValueBuilder builder;
        builder = userver::formats::json::Type::kArray;
        
        for (const auto& [id, prop] : properties) {
            userver::formats::json::ValueBuilder item;
            item["id"] = prop.id;
            item["title"] = prop.title;
            item["description"] = prop.description;
            item["price"] = prop.price;
            item["city"] = prop.city;
            item["rooms"] = prop.rooms;
            builder.PushBack(item.ExtractValue());
        }
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
        return userver::formats::json::ToString(builder.ExtractValue());
        
    } else if (method == userver::server::http::HttpMethod::kPost) {
        auto body = userver::formats::json::FromString(request.RequestBody());
        
        Property prop;
        prop.id = std::to_string(next_id++);
        prop.title = body["title"].As<std::string>();
        prop.description = body["description"].As<std::string>();
        prop.price = body["price"].As<double>();
        prop.city = body["city"].As<std::string>();
        prop.rooms = body["rooms"].As<int>();
        
        properties[prop.id] = prop;
        
        userver::formats::json::ValueBuilder builder;
        builder["id"] = prop.id;
        builder["title"] = prop.title;
        builder["description"] = prop.description;
        builder["price"] = prop.price;
        builder["city"] = prop.city;
        builder["rooms"] = prop.rooms;
        
        request.SetResponseStatus(userver::server::http::HttpStatus::kCreated);
        return userver::formats::json::ToString(builder.ExtractValue());
    }
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kMethodNotAllowed);
    return R"({"error": "Method not allowed"})";
}

} // namespace pillow
#include "handlers/user_handler.hpp"
#include <userver/formats/json.hpp>

namespace pillow {

UserHandler::UserHandler(const userver::components::ComponentConfig& config,
                         const userver::components::ComponentContext& context)
    : HttpHandlerBase(config, context) {}

std::string UserHandler::HandleRequestThrow(
    const userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context) const {
    
    userver::formats::json::ValueBuilder builder;
    builder["message"] = "User endpoint";
    builder["status"] = "ok";
    
    request.SetResponseStatus(userver::server::http::HttpStatus::kOk);
    return userver::formats::json::ToString(builder.ExtractValue());
}

} // namespace pillow
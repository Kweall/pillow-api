#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/server/middlewares/builtin.hpp>
#include "services/user_service.hpp"

namespace pillow {

class AuthMiddleware final : public userver::server::middlewares::HttpMiddlewareBase {
public:
    static constexpr std::string_view kName = "auth-middleware";

    AuthMiddleware(const userver::components::ComponentConfig& config,
                   const userver::components::ComponentContext& context);
    
    void HandleRequest(
        userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context,
        userver::server::middlewares::NextMiddlewareCallback next
    ) const override;

private:
    std::shared_ptr<UserService> user_service_;
    
    std::optional<std::string> ExtractToken(const userver::server::http::HttpRequest& request) const;
    bool ValidateToken(const std::string& token, std::string& user_id) const;
};

} // namespace pillow
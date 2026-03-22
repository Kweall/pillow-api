#pragma once

#include <userver/server/handlers/http_handler_base.hpp>
#include <userver/components/component.hpp>

namespace pillow {

class AuthHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    AuthHandler(const userver::components::ComponentConfig& config,
                const userver::components::ComponentContext& context);
    
    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context
    ) const override;
};

class RegisterHandler final : public userver::server::handlers::HttpHandlerBase {
public:
    RegisterHandler(const userver::components::ComponentConfig& config,
                    const userver::components::ComponentContext& context);
    
    std::string HandleRequestThrow(
        const userver::server::http::HttpRequest& request,
        userver::server::request::RequestContext& context
    ) const override;
};

} // namespace pillow
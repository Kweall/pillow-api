#include "middleware/auth_middleware.hpp"
#include <userver/server/handlers/exceptions.hpp>

namespace pillow {

AuthMiddleware::AuthMiddleware(const userver::components::ComponentConfig& config,
                               const userver::components::ComponentContext& context)
    : HttpMiddlewareBase(config, context) {
    user_service_ = context.FindComponent<UserService>();
}

void AuthMiddleware::HandleRequest(
    userver::server::http::HttpRequest& request,
    userver::server::request::RequestContext& context,
    userver::server::middlewares::NextMiddlewareCallback next) const {
    
    // Пропускаем публичные endpoints
    auto path = request.GetRequestPath();
    if (path == "/api/auth/login" || 
        path == "/api/auth/register" ||
        path == "/ping") {
        next(request, context);
        return;
    }
    
    // Проверяем наличие токена
    auto auth_header = request.GetHeader("Authorization");
    if (auth_header.empty() || auth_header.find("Bearer ") != 0) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::HandlerErrorCode::kUnauthorized,
            "Missing or invalid authorization header"
        );
    }
    
    auto token = auth_header.substr(7);
    std::string user_id;
    
    if (!ValidateToken(token, user_id)) {
        throw userver::server::handlers::ClientError(
            userver::server::handlers::HandlerErrorCode::kUnauthorized,
            "Invalid token"
        );
    }
    
    // Сохраняем user_id в контексте для дальнейшего использования
    context.SetData("user_id", user_id);
    
    next(request, context);
}

std::optional<std::string> AuthMiddleware::ExtractToken(const userver::server::http::HttpRequest& request) const {
    auto auth_header = request.GetHeader("Authorization");
    if (auth_header.empty() || auth_header.find("Bearer ") != 0) {
        return std::nullopt;
    }
    return auth_header.substr(7);
}

bool AuthMiddleware::ValidateToken(const std::string& token, std::string& user_id) const {
    // Простая валидация для учебного проекта
    if (token.find("jwt_") == 0) {
        auto parts = token.substr(4);
        auto underscore_pos = parts.find('_');
        if (underscore_pos != std::string::npos) {
            user_id = parts.substr(0, underscore_pos);
            // Проверяем, существует ли пользователь
            return user_service_->GetUserById(user_id).has_value();
        }
    }
    return false;
}

} // namespace pillow
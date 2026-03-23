#include "utils/auth_utils.hpp"
#include "services/user_service.hpp"

namespace pillow {

static UserService user_service;

std::string ExtractTokenFromHeader(const userver::server::http::HttpRequest& request) {
    auto auth_header = request.GetHeader("Authorization");
    if (auth_header.empty() || auth_header.find("Bearer ") != 0) {
        return "";
    }
    return auth_header.substr(7);
}

bool ValidateToken(const std::string& token, std::string& username) {
    return user_service.ValidateToken(token, username);
}

} // namespace pillow
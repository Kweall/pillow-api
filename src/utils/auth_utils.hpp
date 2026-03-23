#pragma once

#include <userver/server/http/http_request.hpp>
#include <string>

namespace pillow {

std::string ExtractTokenFromHeader(const userver::server::http::HttpRequest& request);
bool ValidateToken(const std::string& token, std::string& username);

} // namespace pillow
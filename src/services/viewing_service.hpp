#pragma once

#include "storage.hpp"
#include <string>
#include <vector>
#include <optional>

namespace pillow {

class ViewingService {
public:
    ViewingService() = default;
    
    // CRUD
    Viewing CreateViewing(const std::string& property_id, const std::string& user_id,
                          const std::string& scheduled_time, const std::string& notes);
    std::optional<Viewing> GetViewingById(const std::string& id);
    std::vector<Viewing> GetUserViewings(const std::string& user_id);
    std::vector<Viewing> GetPropertyViewings(const std::string& property_id);
    bool UpdateViewing(const std::string& id, const std::string& status, const std::string& notes,
                       const std::string& user_id);
    bool DeleteViewing(const std::string& id, const std::string& user_id);
    
private:
    std::string GenerateId();
    std::string GetCurrentTimeISO();
};

} // namespace pillow
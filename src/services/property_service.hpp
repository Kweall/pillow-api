#pragma once

#include "storage.hpp"
#include <string>
#include <vector>
#include <optional>

namespace pillow {

class PropertyService {
public:
    PropertyService() = default;
    
    // CRUD
    Property CreateProperty(const std::string& title, const std::string& description,
                            double price, const std::string& city, int rooms,
                            const std::string& owner_id);
    std::optional<Property> GetPropertyById(const std::string& id);
    std::vector<Property> GetAllProperties();
    std::vector<Property> SearchProperties(const std::string& city,
                                            double min_price, double max_price);
    std::vector<Property> GetUserProperties(const std::string& owner_id);
    bool UpdateProperty(const std::string& id, const std::string& title,
                        const std::string& description, double price,
                        const std::string& city, int rooms, const std::string& user_id);
    bool DeleteProperty(const std::string& id, const std::string& user_id);
    
    // Проверки
    bool PropertyExists(const std::string& id);
    bool IsOwner(const std::string& property_id, const std::string& user_id);
    
private:
    std::string GenerateId();
    std::string GetCurrentTimeISO();
};

} // namespace pillow
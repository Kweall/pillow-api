#include "services/property_service.hpp"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>

namespace pillow {

std::string PropertyService::GenerateId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()
    ).count();
    
    return "prop_" + std::to_string(timestamp) + "_" + std::to_string(dis(gen));
}

std::string PropertyService::GetCurrentTimeISO() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

Property PropertyService::CreateProperty(const std::string& title, const std::string& description,
                                          double price, const std::string& city, int rooms,
                                          const std::string& owner_id) {
    if (title.empty()) {
        throw std::runtime_error("Title cannot be empty");
    }
    if (price <= 0) {
        throw std::runtime_error("Price must be greater than 0");
    }
    if (rooms < 1 || rooms > 10) {
        throw std::runtime_error("Rooms must be between 1 and 10");
    }
    
    Property prop;
    prop.id = std::to_string(next_property_id++);
    prop.title = title;
    prop.description = description;
    prop.price = price;
    prop.city = city;
    prop.rooms = rooms;
    prop.owner_id = owner_id;
    prop.created_at = GetCurrentTimeISO();
    
    properties[prop.id] = prop;
    return prop;
}

std::optional<Property> PropertyService::GetPropertyById(const std::string& id) {
    auto it = properties.find(id);
    if (it != properties.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<Property> PropertyService::GetAllProperties() {
    std::vector<Property> result;
    for (const auto& [id, prop] : properties) {
        result.push_back(prop);
    }
    return result;
}

std::vector<Property> PropertyService::SearchProperties(const std::string& city,
                                                         double min_price, double max_price) {
    std::vector<Property> result;
    for (const auto& [id, prop] : properties) {
        if (!city.empty() && prop.city != city) continue;
        if (min_price > 0 && prop.price < min_price) continue;
        if (max_price > 0 && prop.price > max_price) continue;
        result.push_back(prop);
    }
    return result;
}

std::vector<Property> PropertyService::GetUserProperties(const std::string& owner_id) {
    std::vector<Property> result;
    for (const auto& [id, prop] : properties) {
        if (prop.owner_id == owner_id) {
            result.push_back(prop);
        }
    }
    return result;
}

bool PropertyService::UpdateProperty(const std::string& id, const std::string& title,
                                      const std::string& description, double price,
                                      const std::string& city, int rooms,
                                      const std::string& user_id) {
    auto it = properties.find(id);
    if (it == properties.end()) {
        return false;
    }
    
    if (it->second.owner_id != user_id) {
        return false;
    }
    
    Property& prop = it->second;
    if (!title.empty()) prop.title = title;
    if (!description.empty()) prop.description = description;
    if (price > 0) prop.price = price;
    if (!city.empty()) prop.city = city;
    if (rooms > 0 && rooms <= 10) prop.rooms = rooms;
    
    return true;
}

bool PropertyService::DeleteProperty(const std::string& id, const std::string& user_id) {
    auto it = properties.find(id);
    if (it == properties.end()) {
        return false;
    }
    
    if (it->second.owner_id != user_id) {
        return false;
    }
    
    properties.erase(it);
    return true;
}

bool PropertyService::PropertyExists(const std::string& id) {
    return properties.find(id) != properties.end();
}

bool PropertyService::IsOwner(const std::string& property_id, const std::string& user_id) {
    auto it = properties.find(property_id);
    if (it != properties.end()) {
        return it->second.owner_id == user_id;
    }
    return false;
}

} // namespace pillow
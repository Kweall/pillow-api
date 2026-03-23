#include "services/viewing_service.hpp"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <random>

namespace pillow {

std::string ViewingService::GenerateId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(100000, 999999);
    
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
        now.time_since_epoch()
    ).count();
    
    return "view_" + std::to_string(timestamp) + "_" + std::to_string(dis(gen));
}

std::string ViewingService::GetCurrentTimeISO() {
    auto now = std::chrono::system_clock::now();
    std::time_t time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

Viewing ViewingService::CreateViewing(const std::string& property_id, const std::string& user_id,
                                       const std::string& scheduled_time, const std::string& notes) {
    Viewing viewing;
    viewing.id = std::to_string(next_viewing_id++);
    viewing.property_id = property_id;
    viewing.user_id = user_id;
    viewing.scheduled_time = scheduled_time;
    viewing.status = "scheduled";
    viewing.notes = notes;
    viewing.created_at = GetCurrentTimeISO();
    
    viewings[viewing.id] = viewing;
    return viewing;
}

std::optional<Viewing> ViewingService::GetViewingById(const std::string& id) {
    auto it = viewings.find(id);
    if (it != viewings.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::vector<Viewing> ViewingService::GetUserViewings(const std::string& user_id) {
    std::vector<Viewing> result;
    for (const auto& [id, viewing] : viewings) {
        if (viewing.user_id == user_id) {
            result.push_back(viewing);
        }
    }
    return result;
}

std::vector<Viewing> ViewingService::GetPropertyViewings(const std::string& property_id) {
    std::vector<Viewing> result;
    for (const auto& [id, viewing] : viewings) {
        if (viewing.property_id == property_id) {
            result.push_back(viewing);
        }
    }
    return result;
}

bool ViewingService::UpdateViewing(const std::string& id, const std::string& status,
                                    const std::string& notes, const std::string& user_id) {
    auto it = viewings.find(id);
    if (it == viewings.end()) {
        return false;
    }
    
    if (it->second.user_id != user_id) {
        return false;
    }
    
    Viewing& viewing = it->second;
    if (!status.empty()) viewing.status = status;
    if (!notes.empty()) viewing.notes = notes;
    
    return true;
}

bool ViewingService::DeleteViewing(const std::string& id, const std::string& user_id) {
    auto it = viewings.find(id);
    if (it == viewings.end()) {
        return false;
    }
    
    if (it->second.user_id != user_id) {
        return false;
    }
    
    viewings.erase(it);
    return true;
}

} // namespace pillow
#include "storage.hpp"

namespace pillow {

// Глобальное хранилище (определение)
std::unordered_map<std::string, UserData> users;
std::unordered_map<std::string, std::string> tokens;
std::unordered_map<std::string, Property> properties;
int next_id = 1;

} // namespace pillow
#include "storage.hpp"

namespace pillow {

// Глобальные хранилища
std::unordered_map<std::string, UserData> users;
std::unordered_map<std::string, std::string> tokens;
std::unordered_map<std::string, Property> properties;
std::unordered_map<std::string, Viewing> viewings;
int next_property_id = 1;
int next_viewing_id = 1;

} // namespace pillow
#include <userver/server/handlers/ping.hpp>
#include <userver/utils/daemon_run.hpp>
#include <userver/components/minimal_server_component_list.hpp>
#include <userver/components/component.hpp>

#include "handlers/auth_handler.hpp"
#include "handlers/property_handler.hpp"
#include "handlers/property_item_handler.hpp"
#include "handlers/viewing_handler.hpp"
#include "handlers/user_handler.hpp"

int main(int argc, char* argv[]) {
    auto component_list = userver::components::MinimalServerComponentList();
    
    component_list.Append<userver::server::handlers::Ping>();
    component_list.Append<pillow::AuthHandler>("auth-handler");
    component_list.Append<pillow::RegisterHandler>("register-handler");
    component_list.Append<pillow::PropertyListHandler>("property-list-handler");
    component_list.Append<pillow::PropertyItemHandler>("property-item-handler");
    component_list.Append<pillow::ViewingHandler>("viewing-handler");
    component_list.Append<pillow::UserHandler>("user-handler");
    
    return userver::utils::DaemonMain(argc, argv, component_list);
}
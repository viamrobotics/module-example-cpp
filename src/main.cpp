#include <viam/sdk/components/sensor/sensor.hpp>
#include <viam/sdk/module/service.hpp>

#include "wifi.hpp"

using namespace viam::sdk;

class WifiSensor : public Sensor {
   public:
    WifiSensor(Dependencies deps, ResourceConfig cfg) : Sensor(cfg.name()) {}
    AttributeMap do_command(const AttributeMap& command) { return {}; }
    std::vector<GeometryConfig> get_geometries(const AttributeMap& extra) { return std::vector<GeometryConfig>(); }
    AttributeMap get_readings(const AttributeMap& extra) { return read_wireless(); }
};

int main(int argc, char **argv) {
    if (argc < 2) {
        throw std::invalid_argument("Pass socket path as first argument");
    }
    std::string socket_addr = argv[1];

    set_logger_severity_from_args(argc, argv);

    SignalManager signals;

    std::shared_ptr<ModelRegistration> mr = std::make_shared<ModelRegistration>(
        ResourceType("WifiSensor"), Sensor::static_api(), Model("viam", "wifi", "cpp"),
        [](Dependencies deps, ResourceConfig cfg) { return std::make_shared<WifiSensor>(deps, cfg); }
    );

    Registry::register_model(mr);

    // The `ModuleService_` must outlive the Server, so the declaration order here matters.
    auto my_mod = std::make_shared<ModuleService_>(socket_addr);
    auto server = std::make_shared<Server>();

    my_mod->add_model_from_registry(server, mr->api(), mr->model());
    my_mod->start(server);
    std::cout << "Module serving model " << mr->model().to_string() << ", listening on "
              << socket_addr << std::endl;

    server->start();
    int sig = 0;
    auto result = signals.wait(&sig);
    server->shutdown();
    return 0;
}

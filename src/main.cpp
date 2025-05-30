#include "core/logger.hpp"
#include "core/config_manager.hpp"
#include "core/app.hpp"
#include <iostream>
#include <vector>
#include <string>


signed main(int argc, char* argv[]) {
    Logger::init();  // По умолчанию: vela.log
    Logger::info("Starting Vela!");
    cfg::init(); // По умолчанию: config/main.yaml
    
    std::vector<std::string> args(argv, argv + argc);
    unsigned int argp = 0;
    for (const auto &arg : args) {
        if (arg == "-h" || arg == "--help") {
            std::cout << "======== THIS IS HELP MESSAGE ========\n";
            std::cout << "-h|--help          show this meaasage\n";
            std::cout << "--debug            run into debug mode\n";
            std::cout << "--logpath [path]   change .log file\n";
            std::cout << "--cfgpath [path]   change main conig file\n";
            return 0;
        }
        if (arg == "--debug") {
            Logger::set_level(LogLevel::DEBUG);
            Logger::debug("DEBUG mod enabled");
        }
        if (arg == "--logpath") {
            if (argp + 1 == args.size()) {
                std::cerr << "Ошибка при задании флагов!" << std::endl;
                return 1;
            }
            Logger::set_log_file(args[argp + 1]);
            Logger::info("Log file changet to: " + args[argp + 1]);
        }
        if (arg == "--cfgpath") {
            if (argp + 1 == args.size()) {
                std::cerr << "Ошибка при задании флагов!" << std::endl;
                return 1;
            }
            cfg::init(args[argp + 1]);
            Logger::info("Main config file changed to: " + args[argp + 1]);
        }
        argp++;
    }

    Application app;
    try {
        return app.run();
    } catch (const std::exception& e) {
        Logger::error(e.what());
        return 1;
    }
}

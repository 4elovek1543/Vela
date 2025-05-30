#include "app.hpp"
#include "logger.hpp"
#include "config_manager.hpp"
#include <string>



// Module::Module(const std::string &_name, std::pair<int, int> _pos, const std::string &path) : modulename(_name), pos(_pos) {
//     auto config = loag_config_file(path);
//     try {
//         name = getstring(config, modulename + ".name");
//         icon = getstring(config, modulename + ".icon");
//         tooltip = getstring(config, modulename + ".tooltip");
//         action = getstring(config, modulename + ".action");
//         style_classes = std::vector<std::string>();
//         if (config[modulename]["style_classes"].IsSequence()) {
//             for (const auto &cls : config[modulename]["style_classes"]) {
//                 style_classes.push_back(cls.as<std::string>());
//             }
//         }
//         Logger::debug("Loaded module with path: " + path);
//     } catch (const YAML::Exception &e) {
//         Logger::error("Error while load module: " + std::string(e.what()));
//     }
// }


// ====================================================================

std::string NAME = "org.vela.launcher";


Application::Application() : app(Gtk::Application::create(NAME)) {
    constants = cfg::load_constants();
}

// void load_module(const std::string &path, std::vector<Module> &res) {
//     auto mod = cfg::get(path);
//     res = std::vector<Module>();
//     try {
//         if (mod.IsMap()) {
//             for (const auto &config : mod) {
//                 // std::pair<int, int> pos(getint(config.second, "row"), getint(config.second, "column"));
//                 std::pair<int, int> pos(config.second["row"].as<int>(), config.second["column"].as<int>());
//                 // auto cpath = getstring(config.second, "path");
//                 // cpath = cfg::getstring("app.modules") + cpath;
//                 auto cpath = cfg::getstring("app.modules") + getstring(config.second, "path");
//                 res.push_back(Module(config.first.as<std::string>(), pos, cpath));
//             }
//         }
//         Logger::debug("Loaded module: " + path);
//     } catch (const YAML::Exception &e) {
//         Logger::error("Error while loading module: " + path + ", error: " + std::string(e.what()));
//     } catch (...) {
//         Logger::error("Error while loading module: " + path);
//     }
// }

void Application::load_modules() {
    // load_module("modules.main", modules_main);
}

void Application::load_styles() {
    css_provider = Gtk::CssProvider::create();

    try {
        css_provider->load_from_path("../config/style.css");
        auto display = Gdk::Display::get_default();
        Gtk::StyleContext::add_provider_for_display(
            display,
            css_provider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );
        Logger::debug("App: CSS styles set");
    } catch (const Glib::Error &e) {
        Logger::error("Failed to load CSS: " + std::string(e.what()));
    }
}


// void Application::setup() {
//     app->signal_startup().connect([&]{
//         main_window = Gtk::make_managed<Gtk::ApplicationWindow>();
//         main_window->set_title("Vela launcher");
//         main_window->set_default_size(800, 600);
//         main_window->add_css_class("main");
//         app->add_window(*main_window);
//         main_window->set_visible();
//     });
//     Logger::debug("App: main window set");
// }

void Application::setup() {
    main_window->add_module("Treminal", "TEST COMMAND1");
    main_window->add_module("Test2", "TEST COMMAND2");
    main_window->add_module("Test3", "TEST COMMAND3");
    main_window->arrange_modules();
}

int Application::run() {
    load_modules();
    load_styles();
    app->signal_startup().connect([this] {
        main_window = Gtk::make_managed<MainWindow>(app);
        setup();
        main_window->add_css_class("main");
        main_window->present();
    });
    return app->run();
}
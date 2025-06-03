#include "app.hpp"
#include "logger.hpp"
#include "config_manager.hpp"
#include <string>




Application::Application() : app(Gtk::Application::create("org.vela.launcher", Gio::Application::Flags::NON_UNIQUE)) {}

void load_module(const std::string &path, std::vector<moduleinfo> &res) {
    auto mod = cfg::get(path);
    res = std::vector<moduleinfo>();
    try {
        if (mod.IsMap()) {
            for (const auto &config : mod) {
                std::pair<int, int> pos(config.second["row"].as<int>(), config.second["column"].as<int>());
                auto cpath = cfg::fixpath(getstring(config.second, "path"), "modules");
                res.push_back(moduleinfo(config.first.as<std::string>(), pos, cpath));
            }
        }
        Logger::debug("Loaded module: " + path);
    } catch (const YAML::Exception &e) {
        Logger::error("Error while loading module: " + path + ", error: " + std::string(e.what()));
    } catch (...) {
        Logger::error("Error while loading module: " + path);
    }
}

void Application::load_modules() {
    load_module("modules.main", modules_main);
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


void Application::setup() {
    for (const moduleinfo &mod : modules_main) {
        main_window->add_module(mod);
    }
}

int Application::run(bool set_float) {
    load_modules();
    load_styles();
    app->signal_startup().connect([this, set_float] {
        main_window = Gtk::make_managed<MainWindow>(app);
        if (set_float) {
            main_window->set_name("Vela");
        }
        
        setup();
        main_window->add_css_class("main");
        main_window->present();
    });
    return app->run();
}
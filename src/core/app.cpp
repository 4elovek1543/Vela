#include "app.hpp"
#include "logger.hpp"
#include "notifier.hpp"
#include "config_manager.hpp"
#include <string>




Application::Application() : app(Gtk::Application::create("org.vela.launcher", Gio::Application::Flags::NON_UNIQUE)) {}


std::pair<bool, std::string> check_desposition(std::vector<moduleinfo> &vec, std::pair<int, int> mxsz) {
    std::vector<moduleinfo> res = vec;
    vec.clear();
    std::set<std::pair<int, int>> cgrid;
    for (auto mod : res) {
        if (cgrid.count(mod.pos)) return {false, "Error: duplicate element positions"};
        if (mod.pos.first >= mxsz.first || mod.pos.second >= mxsz.second) return {false, "Error: position out of range"};
        cgrid.insert(mod.pos);
        vec.push_back(mod);
    }
    return {true, "Correct"};
}


void load_module(const std::string &path, std::vector<moduleinfo> &res, std::pair<int, int> mxsz) {
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
        auto [ver, msg] = check_desposition(res, mxsz);
        if (!ver) { 
            Logger::error(msg);
            Notifier::notify(msg, "error");
            Notifier::notify("Fix config and reload app", "warning");
        }

        Logger::debug("Loaded module: " + path);
    } catch (const YAML::Exception &e) {
        Logger::error("Error while loading module: " + path + ", error: " + std::string(e.what()));
    } catch (...) {
        Logger::error("Error while loading module: " + path);
    }
}

void Application::load_modules() {
    mxsz_main = {cfg::getint("window.rows", 4), cfg::getint("window.columns", 5)};
    load_module("modules.main", modules_main, mxsz_main);
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
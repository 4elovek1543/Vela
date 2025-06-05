#include "app.hpp"
#include "logger.hpp"
#include "notifier.hpp"
#include "config_manager.hpp"
#include <string>




Application::Application() : app(Gtk::Application::create("org.vela.launcher", Gio::Application::Flags::NON_UNIQUE)) {}

// // TODO not here
// std::pair<bool, std::string> check_desposition(std::vector<moduleinfo> &vec, std::pair<int, int> mxsz) {
//     std::vector<moduleinfo> res = vec;
//     vec.clear();
//     std::set<std::pair<int, int>> cgrid;
//     for (auto mod : res) {
//         if (cgrid.count(mod.pos)) return {false, "Error: duplicate element positions"};
//         if (mod.pos.first >= mxsz.first || mod.pos.second >= mxsz.second) return {false, "Error: position out of range"};
//         cgrid.insert(mod.pos);
//         vec.push_back(mod);
//     }
//     return {true, "Correct"};
// }

// // TODO not here
// void load_module(const std::string &path, std::vector<moduleinfo> &res, std::pair<int, int> mxsz) {
//     auto mod = cfg::get(path);
//     res = std::vector<moduleinfo>();
//     try {
//         if (mod.IsMap()) {
//             for (const auto &config : mod) {
//                 std::pair<int, int> pos(config.second["row"].as<int>(), config.second["column"].as<int>());
//                 auto cpath = cfg::fixpath(getstring(config.second, "path"), "modules");
//                 res.push_back(moduleinfo(config.first.as<std::string>(), pos, cpath));
//             }
//         }
//         auto [ver, msg] = check_desposition(res, mxsz);
//         if (!ver) { 
//             Logger::error(msg);
//             Notifier::notify(msg, "error");
//             Notifier::notify("Fix config and reload app", "warning");
//         }

//         Logger::debug("Loaded module: " + path);
//     } catch (const YAML::Exception &e) {
//         Logger::error("Error while loading module: " + path + ", error: " + std::string(e.what()));
//     } catch (...) {
//         Logger::error("Error while loading module: " + path);
//     }
// }

// void Application::load_modules() {
//     mxsz_main = {cfg::getint("window.rows", 4), cfg::getint("window.columns", 5)};
//     load_module("modules.main", modules_main, mxsz_main);
// }

void Application::load_styles() {
    css_provider = Gtk::CssProvider::create();

    try {
        css_provider->load_from_path(cfg::fixpath("config/style.css"));
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
    main_window = Gtk::make_managed<MainWindow>(app);
    main_window->add_css_class("base");

    std::pair<int, int> sz, gridsz;
    sz = std::pair<int, int>(cfg::getint("window.width", 800), cfg::getint("window.height", 600));
    gridsz = std::pair<int, int>(cfg::getint("window.rows", 4), cfg::getint("window.columns", 5));
    centre_window = Gtk::make_managed<ElWindow>("main", sz, gridsz);
    centre_window->setup();
    centre_window->set_vexpand(true);
    centre_window->set_hexpand(true);
    main_window->add_window(centre_window, {1, 1});
    if (cfg::getbool("bars.left")) {
        sz = std::pair<int, int>(cfg::getint("bar_left.width", 50), cfg::getint("window.height", 600));
        gridsz = std::pair<int, int>(cfg::getint("bar_left.rows", 4), 1);
        bar_left = Gtk::make_managed<ElWindow>("left", sz, gridsz);        
        bar_left->setup();
        main_window->add_window(bar_left, {1, 0});
    }
    if (cfg::getbool("bars.right")) {
        sz = std::pair<int, int>(cfg::getint("bar_right.width", 50), cfg::getint("window.height", 600));
        gridsz = std::pair<int, int>(cfg::getint("bar_right.rows", 4), 1);
        bar_right = Gtk::make_managed<ElWindow>("right", sz, gridsz);        
        bar_right->setup();
        main_window->add_window(bar_right, {1, 2});
    }
    if (cfg::getbool("bars.top")) {
        sz = std::pair<int, int>(cfg::getint("window.width", 800), cfg::getint("bar_top.height", 50));
        gridsz = std::pair<int, int>(1, cfg::getint("bar_top.columns", 5));
        bar_top = Gtk::make_managed<ElWindow>("top", sz, gridsz);        
        bar_top->setup();
        main_window->add_window(bar_top, {0, 1});
    }
    if (cfg::getbool("bars.bottom")) {
        sz = std::pair<int, int>(cfg::getint("window.width", 800), cfg::getint("bar_bottom.height", 50));
        gridsz = std::pair<int, int>(1, cfg::getint("bar_bottom.columns", 5));
        bar_bottom = Gtk::make_managed<ElWindow>("bottom", sz, gridsz);        
        bar_bottom->setup();
        main_window->add_window(bar_bottom, {2, 1});
    }
}

int Application::run(bool set_float) {
    load_styles();
    app->signal_startup().connect([this, set_float] {
        setup();
        if (set_float) {
            main_window->set_name("Vela");
        }
        main_window->present();
    });
    return app->run();
}
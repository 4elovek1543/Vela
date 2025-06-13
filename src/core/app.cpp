#include "app.hpp"
#include "tools.hpp"
#include "logger.hpp"
#include "notifier.hpp"
#include "config_manager.hpp"
#include <string>




Application::Application() : app(Gtk::Application::create("org.vela.launcher", Gio::Application::Flags::NON_UNIQUE)) {}


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

    std::pair<int, int> sz, gridsz; int iconsize;
    sz = std::pair<int, int>(cfg::getint("window.width", 800), cfg::getint("window.height", 600));
    gridsz = std::pair<int, int>(cfg::getint("window.rows", 4), cfg::getint("window.columns", 5));
    iconsize = cfg::getint("window.size", 72);
    centre_window = Gtk::make_managed<ElWindow>("main", sz, gridsz, iconsize);
    centre_window->setup();
    centre_window->set_vexpand(true);
    centre_window->set_hexpand(true);
    main_window->add_window(centre_window, {1, 1});
    if (cfg::getbool("bars.left")) {
        sz = std::pair<int, int>(cfg::getint("bar_left.width", 50), cfg::getint("window.height", 600));
        gridsz = std::pair<int, int>(cfg::getint("bar_left.rows", 4), 1);
        iconsize = cfg::getint("bar_left.size", 72);
        bar_left = Gtk::make_managed<ElWindow>("left", sz, gridsz, iconsize);        
        bar_left->setup();
        main_window->add_window(bar_left, {1, 0});
    }
    if (cfg::getbool("bars.right")) {
        sz = std::pair<int, int>(cfg::getint("bar_right.width", 50), cfg::getint("window.height", 600));
        gridsz = std::pair<int, int>(cfg::getint("bar_right.rows", 4), 1);
        iconsize = cfg::getint("bar_right.size", 72);
        bar_right = Gtk::make_managed<ElWindow>("right", sz, gridsz, iconsize);        
        bar_right->setup();
        main_window->add_window(bar_right, {1, 2});
    }
    if (cfg::getbool("bars.top")) {
        sz = std::pair<int, int>(cfg::getint("window.width", 800), cfg::getint("bar_top.height", 50));
        gridsz = std::pair<int, int>(1, cfg::getint("bar_top.columns", 5));
        iconsize = cfg::getint("bar_top.size", 72);
        bar_top = Gtk::make_managed<ElWindow>("top", sz, gridsz, iconsize);        
        bar_top->setup();
        main_window->add_window(bar_top, {0, 1});
    }
    if (cfg::getbool("bars.bottom")) {
        sz = std::pair<int, int>(cfg::getint("window.width", 800), cfg::getint("bar_bottom.height", 50));
        gridsz = std::pair<int, int>(1, cfg::getint("bar_bottom.columns", 5));
        iconsize = cfg::getint("bar_bottom.size", 72);
        bar_bottom = Gtk::make_managed<ElWindow>("bottom", sz, gridsz, iconsize);        
        bar_bottom->setup();
        main_window->add_window(bar_bottom, {2, 1});
    }
}


void Application::fixposition() {
    int mode = cfg::getint("window.position", 0);
    int x, y;
    if (mode != 1) {
        auto posl = cfg::getseq("window.centr_pos");
        x = posl[0].as<int>();
        y = posl[1].as<int>();
    } else {
        auto [cx, cy] = get_cursor_pos();
        auto [mx, my] = get_monitor_size();
        auto [w, h] = get_window_size();
        auto [wx, wy] = get_window_pos();
        x = std::clamp(cx - mx/2, -wx, mx - w - wx);
        y = std::clamp(cy - my/2, -wy, my - h - wy);
    }
    std::cout << "type: " << mode << ", pos: " << x << " " << y << " aboba\n";
    if (mode == 2) {
        auto [mx, my] = get_monitor_size();
        std::cout << mx/2 + x << ", " << my/2 + y << "\n";
        std::string cmd = "hyprctl dispatch movecursor " + std::to_string(mx/2 + x) + " " + std::to_string(my/2 + y);
        system(cmd.c_str());
    }
    // std::string cmd = "hyprctl dispatch movewindowpixel '" + std::to_string(x) + " " + std::to_string(y) + "',address:";
    // Logger::debug("cmd: " + cmd);
    // std::cerr << cmd << "\n";
    auto check_address = [x, y]() -> bool {
        std::string address = get_window_address();
        if (address.empty()) {
            return true;
        }

        // Notifier::notify("address: " + address, "info");
        Logger::debug("window address: " + address);
        // std::string curcmd = cmd + address;
        std::string cmd;
        cmd = "hyprctl dispatch movewindowpixel " + std::to_string(x) + " 0,address:" + address;
        system(cmd.c_str());
        cmd = "hyprctl dispatch movewindowpixel '0 " + std::to_string(y) + "',address:" + address;
        system(cmd.c_str());
        return false;
    };

    Glib::signal_timeout().connect(check_address, 1);
}


int Application::run(bool set_float) {
    load_styles();
    app->signal_startup().connect([this, set_float] {
        setup();
        
        main_window->present();
        if (set_float) {
            main_window->set_name("Vela");
        }

        fixposition();
    });
    app->signal_shutdown().connect([]() {
        if (Logger::get_level() != LogLevel::DEBUG) {
            clear_tmp();
        } else {
            Logger::debug("Running into debug mode, tmp files are not deleted");
        }
    });
    return app->run();
}
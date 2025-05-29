#include "app.hpp"
#include "logger.hpp"
#include <string>


std::string NAME = "org.vela.launcher";


Application::Application() : app(Gtk::Application::create(NAME)) {}

// 

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
    app->signal_startup().connect([&]{
        main_window = Gtk::make_managed<Gtk::ApplicationWindow>();
        main_window->set_title("Vela launcher");
        main_window->set_default_size(800, 600);
        main_window->add_css_class("main");
        app->add_window(*main_window);
        main_window->set_visible();
    });
    Logger::debug("App: main window set");
}

int Application::run() {
    load_styles();
    setup();
    return app->run();
}
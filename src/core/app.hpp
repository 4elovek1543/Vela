#pragma once

#include <gtkmm.h>
#include <memory>


class Application {
public:
    Application();
    int run();
private:
    Glib::RefPtr<Gtk::Application> app;
    Glib::RefPtr<Gtk::CssProvider> css_provider;
    Gtk::ApplicationWindow *main_window = 0;

    void load_styles();
    void setup();
};



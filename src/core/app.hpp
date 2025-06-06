#pragma once

#include "../ui/windows.hpp"
#include <yaml-cpp/yaml.h>
#include <memory>
#include <vector>



class Application {
public:
    Application();
    int run(bool set_float=true);
private:
    // std::vector<moduleinfo> modules_main;
    // std::pair<int, int> mxsz_main;

    Glib::RefPtr<Gtk::Application> app;
    Glib::RefPtr<Gtk::CssProvider> css_provider;
    MainWindow *main_window = nullptr;
    ElWindow *centre_window = nullptr;
    ElWindow *bar_left = nullptr;
    ElWindow *bar_right = nullptr;
    ElWindow *bar_top = nullptr;
    ElWindow *bar_bottom = nullptr;

    // void load_modules();
    void load_styles();
    void setup();
};

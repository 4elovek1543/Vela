#pragma once

#include "../ui/windows.hpp"
#include <gtkmm.h>
#include <yaml-cpp/yaml.h>
#include <memory>
#include <vector>


// class Module {
// public:
//     std::string modulename;
//     std::string name;
//     std::string icon;
//     std::string tooltip;
//     std::string action;
//     std::pair<int, int> pos;
//     std::vector<std::string> style_classes;

//     Module(const std::string &_name, std::pair<int, int> _pos, const std::string &path);
// };


class Application {
public:
    Application();
    int run();
private:
    std::map<std::string, YAML::Node> constants;

    // std::vector<Module> modules_main;

    Glib::RefPtr<Gtk::Application> app;
    Glib::RefPtr<Gtk::CssProvider> css_provider;
    // Gtk::ApplicationWindow *main_window = 0;
    MainWindow *main_window = nullptr;

    void load_modules();
    void load_styles();
    void setup();
};

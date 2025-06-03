#pragma once

#include <gtkmm.h>
#include <yaml-cpp/yaml.h>
#include <string>


struct moduleinfo;

class MainWindow : public Gtk::ApplicationWindow {
public:
    MainWindow(const Glib::RefPtr<Gtk::Application> &app);

    void add_module(const moduleinfo &mod);
    void arrange_modules(int columns = 3);
private:
    Gtk::Grid _grid;
};


class Module : public Gtk::Button {
public:
    Module(const moduleinfo &mod);
    Module(const std::string &_name, std::pair<int, int> _pos, int icon_size, const std::string &path);

    void reload_styles();
    void execute() const;
private:
    std::string modulename;
    std::string name;
    std::string icon;
    std::string tooltip;
    std::string action;
    std::pair<int, int> pos;
    std::vector<std::string> style_classes;
};


struct moduleinfo {
    const std::string name;
    std::pair<int, int> pos;
    int icon_size = 72;
    const std::string path;

    moduleinfo(const std::string &_name, std::pair<int, int> _pos, const std::string &_path) : name(_name), pos(_pos), path(_path) {}
};

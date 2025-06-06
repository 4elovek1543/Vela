#pragma once

// #include <gtkmm.h>
#include <gtkmm-4.0/gtkmm.h>
#include <yaml-cpp/yaml.h>
#include <string>


struct moduleinfo;
class ElWindow;

class MainWindow : public Gtk::ApplicationWindow {
public:
    MainWindow(const Glib::RefPtr<Gtk::Application> &app);

    // void add_module(const moduleinfo &mod);
    // void arrange_modules(int columns = 3);
    void add_window(ElWindow *win, const std::pair<int, int> pos);
private:
    Gtk::Grid _grid;
};


class ElWindow : public Gtk::Overlay {
public:
    ElWindow(const std::string &name, const std::pair<int, int> sz, const std::pair<int, int> gridsz, int iconsize=72);

    void load_modules();
    void setup();

    void add_module(const moduleinfo &mod);
    void arrange_modules(int columns = 3);
private:
    std::string name;
    std::pair<int, int> sz;
    std::pair<int, int> gridsz;
    int iconsize;

    Gtk::Grid *_grid;
    Gtk::DrawingArea *focus_frame;
    Gdk::RGBA frame_color1, frame_color2;

    std::vector<moduleinfo>  modules;
};

class Module : public Gtk::Button {
public:
    Module(const moduleinfo &mod);
    Module(const std::string &_name, std::pair<int, int> _pos, int icon_size, const std::string &path);
    
    std::pair<int, int> getpos() const;

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
    std::string name;
    std::pair<int, int> pos;
    int icon_size = 72;
    std::string path;

    moduleinfo(const std::string &_name, std::pair<int, int> _pos, int _icon_size, const std::string &_path) 
        : name(_name), pos(_pos), icon_size(_icon_size), path(_path) {}
};

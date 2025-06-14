#pragma once

// #include <gtkmm.h>
#include <gtkmm-4.0/gtkmm.h>
#include <yaml-cpp/yaml.h>
#include <string>


struct moduleinfo;
class ElWindow;
class FocusFrame;

class MainWindow : public Gtk::ApplicationWindow {
public:
    MainWindow(const Glib::RefPtr<Gtk::Application> &app);

    void add_window(ElWindow *win, const std::pair<int, int> pos, bool init=false);
private:
    Gtk::Grid _grid;

    ElWindow *init_focus_window;
};


class ElWindow : public Gtk::Overlay {
public:
    ElWindow(const std::string &name, const std::pair<int, int> sz, const std::pair<int, int> gridsz, int iconsize=72);

    void load_modules();
    void setup();

    void add_module(const moduleinfo &mod);
    void arrange_modules(int columns = 3);

    FocusFrame *focus_frame;
private:
    std::string name;
    std::pair<int, int> sz;
    std::pair<int, int> gridsz;
    int iconsize;

    Gtk::Grid *_grid;

    std::vector<moduleinfo>  modules;
};


class FocusFrame : public Gtk::DrawingArea {
public:
    FocusFrame(const std::string &col1_name, const std::string &col2_name, double farme_radius=10, double frame_ainm_duration=100);

    void setup();
    const Glib::RefPtr<Gtk::EventControllerMotion>&  get_hover_controller() const;
    void start_frame_animation(bool fade_in);
private:
    std::string col1_name, col2_name;
    Gdk::RGBA color1, color2;
    double alpha = 0.0;
    double radius;
    double fade_duration;
    sigc::connection frame_animation;

    Glib::RefPtr<Gtk::EventControllerMotion> hover_controller;

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
    bool need_sudo;
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

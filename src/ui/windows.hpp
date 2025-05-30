#pragma once

#include <gtkmm.h>
#include <yaml-cpp/yaml.h>
#include <string>


class MainWindow : public Gtk::ApplicationWindow {
public:
    MainWindow(const Glib::RefPtr<Gtk::Application> &app);

    void add_module(const std::string &name, const std::string &command);
    void arrange_modules(int columns = 3);
private:
    Gtk::Grid _grid;

    void setup();
};



class Module : public Gtk::Button {
public:
    Module(const std::string &name, const std::string &command);

    void execute() const;
    const std::string& get_name() const;
    const std::string& get_command() const;
private:
    std::string _name;
    std::string _command;
};
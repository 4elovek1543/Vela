#include "windows.hpp"
#include "../core/logger.hpp"



MainWindow::MainWindow(const Glib::RefPtr<Gtk::Application> &app) : Gtk::ApplicationWindow(app) {
    set_title("Vela Launcher");
    set_default_size(800, 600);
    setup();
}

void MainWindow::setup() {
    _grid.set_margin(10);
    _grid.set_row_spacing(10);
    _grid.set_column_spacing(10);
    set_child(_grid);
}


void MainWindow::add_module(const std::string &name, const std::string &command) {
    auto module = Gtk::make_managed<Module>(name, command);
    module->add_css_class("element");
    int sz = _grid.get_children().size();
    int row = sz / 3;
    int col = sz % 3;
    _grid.attach(*module, col, row, 1, 1);
}

void MainWindow::arrange_modules(int columns) {
    auto children = _grid.get_children();
    // for (auto ch : children) _grid.remove(*ch);

    for (size_t i = 0; i < children.size(); i++) {
        _grid.attach(*dynamic_cast<Gtk::Widget*>(children[i]), i % columns, i / columns);
    }
}


// =============================================================================================


Module::Module(const std::string &name, const std::string &command) 
        : Gtk::Button(name), _name(name), _command(command) {
    // set_margin(5);
    // set_vexpand(true);
    // set_hexpand(true);

    signal_clicked().connect([this]() {
        execute();
    });
}

void Module::execute() const {
    Logger::info("EXECUTE from [" + _name + "]: " + _command);
}

const std::string& Module::get_name() const {
    return _name;
}

const std::string& Module::get_command() const {
    return _command;
}

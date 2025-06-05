#include "windows.hpp"
#include "../core/logger.hpp"
#include "../core/notifier.hpp"
#include "../core/config_manager.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sigc++/trackable.h> 



MainWindow::MainWindow(const Glib::RefPtr<Gtk::Application> &app) : Gtk::ApplicationWindow(app) {
    set_title("Vela Launcher");
    int width = cfg::getint("window.width", 800);
    int height = cfg::getint("window.height", 600);
    set_default_size(width, height);

    _grid.set_vexpand(true);
    _grid.set_hexpand(true);
    set_child(_grid);

    auto controller = Gtk::EventControllerKey::create();
    controller->signal_key_pressed().connect([this](guint keyval, guint, Gdk::ModifierType) {
        if (keyval == GDK_KEY_Escape) {
            close();
            return true;
        } 
        return false;
    }, false);

    add_controller(controller);
}


void MainWindow::add_window(ElWindow *win, const std::pair<int, int> pos) {
    auto [row, col] = pos;
    _grid.attach(*win, col, row, 1, 1);
}


// =============================================================================================


ElWindow::ElWindow(const std::string &_name, const std::pair<int, int> _sz, const std::pair<int, int> _gridsz) 
        : name(_name), sz(_sz), gridsz(_gridsz) {
    
    auto [width, height] = sz;
    set_size_request(width, height);

    auto [rows, columns] = gridsz;
    set_margin(10);
    set_row_spacing(10);
    set_column_spacing(10);
    set_row_homogeneous(true);
    set_column_homogeneous(true);
    auto fictiveel = Gtk::make_managed<Gtk::Box>();
    attach(*fictiveel, 0, 0, columns, rows);

    load_modules();
}


std::pair<bool, std::string> check_desposition(std::vector<moduleinfo> &vec, std::pair<int, int> mxsz) {
    std::vector<moduleinfo> res = vec;
    vec.clear();
    std::set<std::pair<int, int>> cgrid;
    for (auto mod : res) {
        if (cgrid.count(mod.pos)) return {false, "Error: duplicate element positions"};
        if (mod.pos.first >= mxsz.first || mod.pos.second >= mxsz.second) return {false, "Error: position out of range"};
        cgrid.insert(mod.pos);
        vec.push_back(mod);
    }
    return {true, "Correct"};
}


void load_module(const std::string &path, std::vector<moduleinfo> &res, std::pair<int, int> mxsz) {
    auto mod = cfg::get(path);
    res = std::vector<moduleinfo>();
    try {
        if (mod.IsMap()) {
            for (const auto &config : mod) {
                std::pair<int, int> pos(config.second["row"].as<int>(), config.second["column"].as<int>());
                auto cpath = cfg::fixpath(getstring(config.second, "path"), "modules");
                res.push_back(moduleinfo(config.first.as<std::string>(), pos, cpath));
            }
        }
        auto [ver, msg] = check_desposition(res, mxsz);
        if (!ver) { 
            Logger::error(msg);
            Notifier::notify(msg, "error");
            Notifier::notify("Fix config and reload app", "warning");
        }

        Logger::debug("Loaded module: " + path);
    } catch (const YAML::Exception &e) {
        Logger::error("Error while loading module: " + path + ", error: " + std::string(e.what()));
    } catch (...) {
        Logger::error("Error while loading module: " + path);
    }
}


void ElWindow::load_modules() {
    load_module("modules." + name, modules, gridsz);
}


void ElWindow::setup() {
    for (const auto &mod : modules) {
        add_module(mod);
    }
    add_css_class("window");
    add_css_class(name);
}


void ElWindow::add_module(const moduleinfo &mod) {
    auto module = Gtk::make_managed<Module>(mod);
    module->reload_styles();

    auto [row, col] = module->getpos();
    attach(*module, col, row, 1, 1);
}

// not shure this is needful
void ElWindow::arrange_modules(int columns) {
    auto children = get_children();
    // for (auto ch : children) _grid.remove(*ch);

    for (size_t i = 0; i < children.size(); i++) {
        attach(*dynamic_cast<Gtk::Widget*>(children[i]), i % columns, i / columns);
    }
}

// =============================================================================================


Module::Module(const moduleinfo &mod) : Module(mod.name, mod.pos, mod.icon_size, mod.path) {}


Module::Module(const std::string &_name, std::pair<int, int> _pos, int icon_size, const std::string &path) 
        : Gtk::Button(), modulename(_name), pos(_pos) {
    auto config = load_config_file(path);
    try {
        name = getstring(config, modulename + ".name");
        icon = getstring(config, modulename + ".icon");
        tooltip = getstring(config, modulename + ".tooltip");
        
        action = getstring(config, modulename + ".action");
        action = cfg::fill_from_constants(action);
        action = fill_from_scripts(action);

        auto cstyles = getseq(config, modulename + ".style_classes");
        for (const auto &cls : cstyles) {
            style_classes.push_back(cls.as<std::string>());
        }
        Logger::debug("Loaded module with path: " + path);
    } catch (const YAML::Exception &e) {
        Logger::error("Error while load module: " + std::string(e.what()));
    }
    set_hexpand(false);
    set_vexpand(false);
    
    set_halign(Gtk::Align::CENTER);
    set_valign(Gtk::Align::CENTER);


    reload_styles();

    auto box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL, 5);

    auto icon_image = Gtk::make_managed<Gtk::Image>();
    try {
        auto pixbuf = Gdk::Pixbuf::create_from_file(cfg::fixpath(icon, "static"), icon_size, icon_size, true);
        icon_image->set(Gdk::Texture::create_for_pixbuf(pixbuf));
    } catch(const Glib::Error& e) {
        Logger::error("Error while set icon: " + path + ", error: " + std::string(e.what()));
        icon_image->set_from_icon_name("image-missing");
    }
    icon_image->set_size_request(icon_size, icon_size);
    icon_image->add_css_class("element-icon");
    box->append(*icon_image);

    auto lbl = Gtk::make_managed<Gtk::Label>(name);
    box->append(*lbl);    

    set_child(*box);

    set_tooltip_text(tooltip);

    signal_clicked().connect(
        sigc::track_obj([this]() {
            execute();
        }, *this)
    );
}


std::pair<int, int> Module::getpos() const {
    return pos;
}


void Module::reload_styles() {
    add_css_class("element");
    for (const auto &cls : style_classes) {
        add_css_class(cls);
    }
}

void Module::execute() const {
    Logger::info("EXECUTE from [" + modulename + "]: " + action);

    pid_t pid = fork();
    if (pid == 0) {
        setsid();

        execl("/bin/sh", "sh", "-c", action.c_str(), NULL);

        Logger::error("Failed to execute: " + action);
        _exit(EXIT_FAILURE);
    } else if (pid < 0) {
        Logger::error("Failed to fork, execution failed!");
    }
}

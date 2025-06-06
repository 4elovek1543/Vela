#include "windows.hpp"
#include "../core/tools.hpp"
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


ElWindow::ElWindow(const std::string &_name, const std::pair<int, int> _sz, const std::pair<int, int> _gridsz, int _iconsize) 
        : name(_name), sz(_sz), gridsz(_gridsz), iconsize(_iconsize) {
    
    auto [width, height] = sz;
    set_size_request(width, height);

    auto [rows, columns] = gridsz;
    _grid = Gtk::make_managed<Gtk::Grid>();
    _grid->set_margin(5);
    _grid->set_row_spacing(10);
    _grid->set_column_spacing(10);
    _grid->set_row_homogeneous(true);
    _grid->set_column_homogeneous(true);
    auto fictiveel = Gtk::make_managed<Gtk::Box>();
    _grid->attach(*fictiveel, 0, 0, columns, rows);


    focus_frame = Gtk::make_managed<Gtk::DrawingArea>();
    focus_frame->set_visible(false);
    focus_frame->set_can_target(false);
    add_overlay(*focus_frame);
    

    auto hover_controller = Gtk::EventControllerMotion::create();
    hover_controller->signal_enter().connect([this](double, double) {
        focus_frame->set_visible(true);
    });
    hover_controller->signal_leave().connect([this]() {
        focus_frame->set_visible(false);
    });
    add_controller(hover_controller);

    
    focus_frame->set_draw_func([this](const Cairo::RefPtr<Cairo::Context>& cr, int w, int h) {
        cr->set_line_width(2.0);
        cr->arc(12, 12, 10, M_PI, 1.5 * M_PI); // Левый верхний
        cr->arc(w - 12, 12, 10, 1.5 * M_PI, 2 * M_PI); // Правый верхний
        cr->arc(w - 12, h - 12, 10, 0, 0.5 * M_PI); // Правый нижний
        cr->arc(12, h - 12, 10, 0.5 * M_PI, M_PI); // Левый нижний
        cr->close_path();

        auto gradient = Cairo::LinearGradient::create(0, 0, w, h);
        gradient->add_color_stop_rgba(0, frame_color1.get_red(), frame_color1.get_green(), frame_color1.get_blue(), frame_color1.get_alpha());
        gradient->add_color_stop_rgba(1, frame_color2.get_red(), frame_color2.get_green(), frame_color2.get_blue(), frame_color2.get_alpha());
        cr->set_source(gradient);
        cr->stroke();
    });


    set_child(*_grid);

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


void load_module(const std::string &path, std::vector<moduleinfo> &res, std::pair<int, int> mxsz, int iconszie) {
    auto mod = cfg::get(path);
    res = std::vector<moduleinfo>();
    try {
        if (mod.IsMap()) {
            for (const auto &config : mod) {
                std::pair<int, int> pos(config.second["row"].as<int>(), config.second["column"].as<int>());
                auto cpath = cfg::fixpath(getstring(config.second, "path"), "modules");
                res.push_back(moduleinfo(config.first.as<std::string>(), pos, iconszie, cpath));
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
    load_module("modules." + name, modules, gridsz, iconsize);
}


float getframe(const std::string &val) {
    float res = 0.0;
    for (auto x : val) {
        if (x == ' ') continue;
        res *= 10;
        res += (x - '0');
    }
    return res / 256.0;
}


bool load_color(const std::string &name, Gdk::RGBA &res) {
    auto ini = cfg::getstring("app." + name, "");
    if (ini == "") return false;
    auto val = split(ini, '(')[1];
    val.pop_back();
    auto clr = split(val, ',');
    if (clr.size() < 3 || clr.size() > 4) return false;
    try {
        float r = getframe(clr[0]);
        float g = getframe(clr[1]);
        float b = getframe(clr[2]);
        float a = 1.0;
        if (clr.size() == 4) {
            int i = 0;
            if (clr[3][0] == ' ') i++;
            if (clr[3][i] != '1') {
                float xx = 0.0;
                i += 2; 
                int sz = clr[3].size() - i;
                for (; i < (int)clr[3].size(); i++) xx*=10, xx += (clr[3][i] - '0');
                for (int j = 0; j < sz; j++) xx /= 10.0;
                a = xx;
            }
        }
        res.set_rgba(r, g, b, a);
        return true;
    } catch (...) {
        Logger::warning("Error at loading color with name " + name + ", use default");
        return false;
    }
}


void ElWindow::setup() {
    for (const auto &mod : modules) {
        add_module(mod);
    }
    add_css_class("window");
    add_css_class(name);
    
    auto style_context = get_style_context();
    // if (style_context->lookup_color("--color-focus-border-1", frame_color1)) {} 
    if (load_color("--color-focus-border-1", frame_color1)) {} 
    else {
        Logger::warning("Color --color-focus-border-1 not found");
        frame_color1.set_rgba(0.16, 0.80, 0.78, 1.0);
    }
    // if (style_context->lookup_color("--color-focus-border-2", frame_color2)) {} 
    if (load_color("--color-focus-border-2", frame_color2)) {} 
    else {
        Logger::warning("Color --color-focus-border-2 not found");
        frame_color2.set_rgba(0.22, 0.83, 0.42, 1.0);
    }

    Logger::debug(frame_color1.to_string());
    Logger::debug(frame_color2.to_string());
}


void ElWindow::add_module(const moduleinfo &mod) {
    auto module = Gtk::make_managed<Module>(mod);
    module->reload_styles();

    auto [row, col] = module->getpos();
    _grid->attach(*module, col, row, 1, 1);
}

// not shure this is needful
void ElWindow::arrange_modules(int columns) {
    auto children = get_children();
    // for (auto ch : children) _grid.remove(*ch);

    for (size_t i = 0; i < children.size(); i++) {
        _grid->attach(*dynamic_cast<Gtk::Widget*>(children[i]), i % columns, i / columns);
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

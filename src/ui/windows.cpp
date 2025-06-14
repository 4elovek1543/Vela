#include "windows.hpp"
#include "../core/tools.hpp"
#include "../core/logger.hpp"
#include "../core/notifier.hpp"
#include "../core/config_manager.hpp"
#include <unistd.h>
#include <sys/wait.h>
#include <sigc++/trackable.h> 
#include <chrono>
#include <thread> 



MainWindow::MainWindow(const Glib::RefPtr<Gtk::Application> &app) : Gtk::ApplicationWindow(app) {
    set_title("Vela Launcher");
    int width = cfg::getint("window.width", 800);
    int height = cfg::getint("window.height", 600);
    set_default_size(width, height);

    // set_decorated(false);
    // set_opacity(0.5);

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

    auto focus_controller = Gtk::EventControllerFocus::create();
    focus_controller->signal_leave().connect([this]() {

    });
}


void MainWindow::add_window(ElWindow *win, const std::pair<int, int> pos, bool init) {
    auto [row, col] = pos;
    _grid.attach(*win, col, row, 1, 1);
    if (init) {
        init_focus_window = win;
        win->focus_frame->set_visible(true);
        win->focus_frame->start_frame_animation(true);

        auto ctr = Gtk::EventControllerFocus::create();
        ctr->signal_leave().connect([this]() {
            this->init_focus_window->focus_frame->start_frame_animation(false);
        });
        add_controller(ctr);
    }
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


    double frame_radius = cfg::getfloat("app.--focus-border-radius", 10);
    double frame_fade_duration = cfg::getfloat("app.--focus-border-fade-duration", 100);
    focus_frame = Gtk::make_managed<FocusFrame>("--color-focus-border-1", "--color-focus-border-2", frame_radius, frame_fade_duration);

    add_controller(focus_frame->get_hover_controller());

    add_overlay(*focus_frame);


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


void ElWindow::setup() {
    for (const auto &mod : modules) {
        add_module(mod);
    }
    add_css_class("window");
    add_css_class(name);

    focus_frame->setup();
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


FocusFrame::FocusFrame(const std::string &_col1_name, const std::string &_col2_name, double frame_radius, double frame_ainm_duration)
        : col1_name(_col1_name), col2_name(_col2_name), radius(frame_radius), fade_duration(frame_ainm_duration) {
    set_visible(false);
    set_can_target(false);
    
    hover_controller = Gtk::EventControllerMotion::create();
    hover_controller->signal_enter().connect([this](double, double) {
        set_visible(true);
        start_frame_animation(true);
    });
    hover_controller->signal_leave().connect([this]() {
        start_frame_animation(false);
    });


    set_draw_func([this](const Cairo::RefPtr<Cairo::Context>& cr, int w, int h) {
        cr->set_line_width(2.0);
        cr->arc(radius, radius, radius-2, M_PI, 1.5 * M_PI); // Левый верхний
        cr->arc(w - radius, radius, radius-2, 1.5 * M_PI, 2 * M_PI); // Правый верхний
        cr->arc(w - radius, h - radius, radius-2, 0, 0.5 * M_PI); // Правый нижний
        cr->arc(radius, h - radius, radius-2, 0.5 * M_PI, M_PI); // Левый нижний
        cr->close_path();

        auto gradient = Cairo::LinearGradient::create(0, 0, w, h);
        gradient->add_color_stop_rgba(0, color1.get_red(), color1.get_green(), color1.get_blue(), color1.get_alpha()*alpha);
        gradient->add_color_stop_rgba(1, color2.get_red(), color2.get_green(), color2.get_blue(), color2.get_alpha()*alpha);
        cr->set_source(gradient);
        cr->stroke();
    });
}


void FocusFrame::start_frame_animation(bool fade_in) {
    const double step = 0.05;
    const double interval = fade_duration * step;

    if (frame_animation) {
        frame_animation.disconnect();
    }

    frame_animation = Glib::signal_timeout().connect([this, step, fade_in]() {
        alpha += (fade_in) ? step : -step;
        alpha = std::clamp(alpha, 0.0, 1.0);

        queue_draw();

        if ((fade_in && alpha >= 1.0) || (!fade_in && alpha <= 0.0)) {
            if (!fade_in) set_visible(false);
            return false;
        }
        return true;
    }, interval);
}


const Glib::RefPtr<Gtk::EventControllerMotion>&  FocusFrame::get_hover_controller() const {
    return hover_controller;
}


float getfraction(const std::string &val) {
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
        float r = getfraction(clr[0]);
        float g = getfraction(clr[1]);
        float b = getfraction(clr[2]);
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


void FocusFrame::setup() {
    auto css_provider = Gtk::CssProvider::create();
    css_provider->load_from_data(".fix-border:hover { border-radius: " + std::to_string(radius) + "px; }");
    Gtk::StyleContext::add_provider_for_display(
        Gdk::Display::get_default(),
        css_provider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    get_parent()->add_css_class("fix-border");


    // auto style_context = get_style_context();
    // if (style_context->lookup_color(col1_name, frame_color1)) {} 
    if (load_color(col1_name, color1)) {} 
    else {
        Logger::warning("Color " + col1_name + " not found");
        color1.set_rgba(0.16, 0.80, 0.78, 1.0);
    }
    // if (style_context->lookup_color(col2_name, frame_color2)) {} 
    if (load_color(col2_name, color2)) {} 
    else {
        Logger::warning("Color " + col2_name + " not found");
        color2.set_rgba(0.22, 0.83, 0.42, 1.0);
    }

    Logger::debug(color1.to_string());
    Logger::debug(color2.to_string());
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

        need_sudo = getbool(config, modulename + ".need_sudo", false);

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

    if (need_sudo) {
        Logger::info("Need to run under sudo");
        int res = execute_as_root(action);
        if (res < 0) {
            Logger::error("Execution under root failed, code: " + std::to_string(res));
        } 
    } else {
        // Glib::spawn_command_line_async(action);
        pid_t pid = fork(); 
        if (pid == 0) {
            setsid();

            execl("/bin/sh", "sh", "-c", (action + add_logging(action)).c_str(), NULL);

            Logger::error("Failed to execute: " + action);
            _exit(EXIT_FAILURE);
        } else if (pid < 0) {
            Logger::error("Failed to fork, execution failed!");    
        }
    }

    Notifier::notify("address: " + get_window_address(), "info");
}

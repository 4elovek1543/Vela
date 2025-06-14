#include "tools.hpp"
#include <unistd.h>
#include <limits.h> 
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#include <gtkmm-4.0/gtkmm.h>


namespace fs = std::filesystem;


std::vector<std::string> split(const std::string &s, const char sep) {
    std::vector<std::string> res(1);
    for (auto x : s) {
        if (x == sep) {
            res.push_back(std::string(""));
        } else {
            res.back() += x;
        }
    }
    return res;
} 


std::string joinpath(const std::vector<std::string> &val) {
    std::string res = "";
    for (const auto &s : val) {
        int cnt = 0;
        if (res.size() > 0 && res[res.size()-1] == '/') cnt++;
        if (s.size() > 0 && s[0] == '/') cnt++;
        if (cnt == 0 && res.size() > 0) res += "/";
        if (cnt == 2) res.pop_back();
        res += s;
    }
    return res;
} 


int to_int(const std::string &val, int defval=0) {
    if (val.empty()) return defval;
    int res = 0;
    bool under0 = (val[0] == '-');
    for (size_t i = under0; i < val.size(); i++) {
        if (val[i] < '0' || val[i] > '9') return defval;
        res *= 10;
        res += (val[i] - '0');
    }
    if (under0) res *= -1;
    return res;
}


float to_float(const std::string &val, float defval=0.0) {
    if (val.empty()) return defval;
    float res = 0.0;
    size_t i = 0;
    for (; i < val.size() && val[i] != '.'; i++) {
        if (val[i] < '0' or val[i] > '9') return defval;
        res *= 10;
        res += (val[i] - '0');
    }
    i++;
    float e = 0.0;
    float d = 1.0;
    for (; i < val.size(); i++) {
        if (val[i] < '0' or val[i] > '9') return defval;
        e *= 10;
        e += (val[i] - '0');
        d *= 10;
    }
    res += e / d;
    return res;
}


std::string getExecutablePath() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        return std::string(result, count);
    }
    return "";
}


std::string getProjectPath() {
    auto cpath = getExecutablePath();
    if (cpath.size() >= 10) {
        for (int i = 0; i < 10; i++) {
            cpath.pop_back();
        }
    }
    return cpath;
}


bool streq(const std::string &s1, int p1, int e1, const std::string &s2, int p2, int e2) {
    if (e1 - p1 != e2 - p2) return false;
    for (int i = 0; i < e1 - p1; i++) if (s1[p1 + i] != s2[p2 + i]) return false;
    return true; 
} 


const std::string tmpdir = "/tmp/vela";


std::string get_tmp_scriptpath(const std::string &name, std::string ext) {
    if (access(tmpdir.c_str(), F_OK)) {
        mkdir(tmpdir.c_str(), 0700);
    }
    if (ext.size() > 0 && ext[0] != '.') ext = "." + ext;
    return joinpath({tmpdir, "/vela_" + name + "_" + std::to_string(getpid()) + ext});
}


std::string make_env_escaped(const std::string &str) {
    auto splcur = split(str, '=');
    auto key = splcur[0];
    std::string val = "";
    for (size_t i = 1; i < splcur.size(); i++) val += splcur[i];
    std::string rval;
    rval.reserve(val.size() + 2);
    rval += "'";
    for (char c : val) {
        if (c == '\'') {
            rval += "'\\''";
        } else {
            rval.push_back(c);
        }
    }
    rval += "'";
    return key + "=" + rval;
}


std::string add_logging(std::string command, const std::string &logpath) {
    auto get_stsmp = [command]() {
        std::ostringstream oss;
        oss << "\033[36m[" << getpid() << "] [action: " << command << "]\033[0m ";
        return oss.str();
    };
    return " 2>&1 | awk '{ print \"" + get_stsmp() + "\" $0 }' | tee -a " + logpath;
}


int execute_as_root(const std::string command, const std::string &logpath) {
    std::string script_path = get_tmp_scriptpath("child_wrapper");

    std::ofstream script(script_path);
    if (!script) {
        std::cerr << "Failed to create script file\n";
        return -1;
    }

    extern char **environ;

    script << "#!/bin/sh\n\n";
    for (char **env = environ; *env != nullptr; ++env) {
        script << "export " << make_env_escaped(*env) << "\n";
    }

    script << "\n\n";
    script << "export SUDO_ASKPASS=\"" << joinpath({getProjectPath(), "src/core/askpassword.sh"}) << "\"\n";
    script << "exec sudo -E -A " << command << add_logging(command, logpath) << "\n";
    // script << "exec sudo -E -A " << command << "\n";
    script.close();

    if (chmod(script_path.c_str(), 0700) == -1) {
        std::cerr << "cmod failed\n";
        return -1;
    }

    std::string action = script_path;
    std::cout << "Executing action: " << action << "\n";

    Glib::spawn_command_line_async(action);
    return 0;
}


void clear_tmp() {
    auto pid = std::to_string(getpid());

    try {
        for (const auto &entry : fs::directory_iterator(tmpdir)) {
            if (entry.is_regular_file()) {
                auto fname = entry.path().filename().string();
                if (fname.find(pid) != std::string::npos && fname.find("vela") != std::string::npos) {
                    fs::remove(entry.path());
                }
            }
        }
    } catch(const fs::filesystem_error &e) {
        std::cerr << "Error while clear tmp files: " << e.what() << std::endl;
    }
}


std::string get_from_cmd(const std::string &cmd) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";

    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe)) {
        result += buffer;
    }
    pclose(pipe);

    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}


std::string get_window_address() {
    std::string cmd = "hyprctl clients -j | jq -r '.[] | select(.pid == " + std::to_string(getpid()) + ") | .address'";
    return get_from_cmd(cmd);
}


std::pair<int, int> get_window_size() {
    std::string cmd = "hyprctl clients -j | jq -r '.[] | select(.pid == " + std::to_string(getpid()) + ") | .size'";
    auto s = split(get_from_cmd(cmd), ',');
    std::string sw, sh;
    for (auto x : s[0]) if ('0' <= x && x <= '9') sw += x;
    for (auto x : s[1]) if ('0' <= x && x <= '9') sh += x;
    return {to_int(sw, 0), to_int(sh, 0)};
}


std::pair<int, int> get_window_pos() {
    std::string cmd = "hyprctl clients -j | jq -r '.[] | select(.pid == " + std::to_string(getpid()) + ") | .at'";
    auto s = split(get_from_cmd(cmd), ',');
    std::string sw, sh;
    for (auto x : s[0]) if ('0' <= x && x <= '9') sw += x;
    for (auto x : s[1]) if ('0' <= x && x <= '9') sh += x;
    return {to_int(sw, 0), to_int(sh, 0)};
}


std::pair<int, int> get_monitor_size() {
    std::string cmd;
    cmd = "hyprctl monitors -j | jq -r '.[] | select(.focused == true) | .width'";
    auto sw = get_from_cmd(cmd);
    int w = to_int(sw, 0);
    cmd = "hyprctl monitors -j | jq -r '.[] | select(.focused == true) | .height'";
    auto sh = get_from_cmd(cmd);
    int h = to_int(sh, 0);
    cmd = "hyprctl monitors -j | jq -r '.[] | select(.focused == true) | .scale'";
    float sc = to_float(get_from_cmd(cmd), 1.0);
    return {float(w) / sc, float(h) / sc};
}


std::pair<int, int> get_monitor_pos() {
    std::string cmd;
    cmd = "hyprctl monitors -j | jq -r '.[] | select(.focused == true) | .x'";
    auto x = to_int(get_from_cmd(cmd), 0);
    cmd = "hyprctl monitors -j | jq -r '.[] | select(.focused == true) | .y'";
    auto y = to_int(get_from_cmd(cmd), 0);
    return {x, y};
}


std::pair<int, int> get_cursor_pos() {
    std::string cmd = "hyprctl cursorpos";
    auto s = split(get_from_cmd(cmd), ' ');
    s[0].pop_back();
    return {to_int(s[0], 0), to_int(s[1], 0)};
}
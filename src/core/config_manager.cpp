#include "config_manager.hpp"
#include "logger.hpp"
#include <vector>
#include <filesystem>
#include <unistd.h>
#include <limits.h> 


std::vector<std::string> split(const std::string &s, const char sep='.') {
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


std::string getExecutablePath() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        return std::string(result, count);
    }
    return "";
}


YAML::Node cfg::config;
std::string cfg::cfgpath;
std::map<std::string, YAML::Node> cfg::constants;
std::string cfg::PROJECT_PATH;


void cfg::init(const std::string &path_to_cfg) {
    PROJECT_PATH = getExecutablePath();
    if (PROJECT_PATH.size() > 0) for (int i = 0; i < 10; i++) PROJECT_PATH.pop_back();
    Logger::debug("Project path: " + PROJECT_PATH);
    try {
        cfgpath = cfg::fixpath(path_to_cfg);
        config = YAML::LoadFile(cfgpath);
        Logger::debug("Main config loaded");

        constants = cfg::load_constants();
    } catch (const YAML::Exception &e) {
        Logger::error("Couldn't load main config: " + std::string(e.what()));
    }
}

YAML::Node cfg::get(const std::string &path, YAML::Node defval) {
    return getnode(config, path, defval);
}

YAML::Node getnode(YAML::Node config, const std::string &path, YAML::Node defval) {
    auto cpath = split(path);
    if (path.size() == 0) {
        Logger::error("Error while get from config with path: " + path + ", error: empty path");
        return defval;
    }
    auto res = YAML::Clone(config);
    for (auto node : cpath) {
        if (res[node].IsDefined()) {
            res = res[node];
        } else {
            Logger::error("Error while get from config with path: " + path + ", error: wrong path");
            return defval;
        }
    }
    return res;
}

int cfg::getint(const std::string &path, int defval) {
    return ::getint(config, path, defval);
}

int getint(YAML::Node config, const std::string &path, int defval) {
    auto res = getnode(config, path);
    try {
        return res.as<int>();
    } catch (const YAML::BadConversion& e) {
        Logger::error("Error in getint: " + std::string(e.what()));
        return defval;
    }
}

float cfg::getfloat(const std::string &path, float defval) {
    return ::getfloat(config, path, defval);
}

float getfloat(YAML::Node config, const std::string &path, float defval) {
    auto res = getnode(config, path);
    try {
        return res.as<float>();
    } catch (const YAML::BadConversion& e) {
        Logger::error("Error in getfloat: " + std::string(e.what()));
        return defval;
    }
}

bool cfg::getbool(const std::string &path, bool defval) {
    return ::getbool(config, path, defval);
}

bool getbool(YAML::Node config, const std::string &path, bool defval) {
    auto res = getnode(config, path);
    try {
        return res.as<bool>();
    } catch (const YAML::BadConversion& e) {
        Logger::error("Error in getbool: " + std::string(e.what()));
        return defval;
    }
}

std::string cfg::getstring(const std::string &path, std::string defval) {
    return ::getstring(config, path, defval);
}

std::string getstring(YAML::Node config, const std::string &path, std::string defval) {
    auto res = getnode(config, path);
    try {
        return res.as<std::string>();
    } catch (const YAML::BadConversion& e) {
        Logger::error("Error in getstring: " + std::string(e.what()));
        return defval;
    }
}

std::vector<YAML::Node> getseq(YAML::Node config, const std::string &path, std::vector<YAML::Node> defval) {
    auto node = getnode(config, path);
    std::vector<YAML::Node> res;
    if (node.IsSequence()) {
        try {
            for (const auto &el : node) res.push_back(el);
            return res;
        } catch (const YAML::Exception& e) {
            Logger::error("Error in getseq: path:" + path + ", error: " + std::string(e.what()));
            return defval;
        }
    } else {
        Logger::error("Error in getseq: path:" + path + ", error: destination is not sequence");
        return defval;
    }
}


std::map<std::string, YAML::Node> cfg::load_constants() {
    std::map<std::string, YAML::Node> res;
    for (const auto &c : config["app"]["constants"]) {
        res[c.first.as<std::string>()] = c.second;
    }
    Logger::debug("Loaded constants (" + std::to_string(res.size()) + ")");
    return res;
}

std::string cfg::fill_from_constants(const std::string &val) {
    return ::fill_from_constants(val, constants);
}

YAML::Node cfg::get_const(const std::string &key, YAML::Node defval) {
    return (constants.count(key)) ? constants.at(key) : defval;
}

std::string cfg::fixpath(const std::string &path, const std::string mode) {
    Logger::debug("fixpath: " + path + " " + mode);
    std::string res;
    if (path.size() == 0) return res;
    if (path[0] == '/') return path;
    std::string addstr = "";
    if (mode == "modules") addstr = getstring("app.modules");
    if (mode == "scripts") addstr = getstring("app.scripts");
    if (mode == "static") addstr = getstring("app.static");

    Logger::debug("-> " + joinpath({cfg::PROJECT_PATH, addstr, path}));
    return joinpath({cfg::PROJECT_PATH, addstr, path}); 
}

YAML::Node load_config_file(const std::string &path, YAML::Node defval) {
    try {
        return YAML::LoadFile(path);
    } catch (const YAML::Exception &e) {
        Logger::error("Couldn't load config: path: " + path + ", error: " + std::string(e.what()));
        return defval;
    }
}

bool streq(const std::string &s1, int p1, int e1, const std::string &s2, int p2, int e2) {
    if (e1 - p1 != e2 - p2) return false;
    for (int i = 0; i < e1 - p1; i++) if (s1[p1 + i] != s2[p2 + i]) return false;
    return true; 
} 


std::string fill_from_constants(const std::string &val, const std::map<std::string, YAML::Node> &constants) {
    std::string res;
    for (size_t i = 0; i < val.size(); i++) {
        if (val[i] == '$' && !streq(val, i, i + 8, "$script:", 0, 8)) {
            int j = i + 1; 
            std::string cconst;
            while (j < (int)val.size() && val[j] != ' ') cconst += val[j], j++;
            if (constants.count(cconst)) res += constants.at(cconst).as<std::string>();
            else res += "$" + cconst;
            i = j - 1;
        } else {
            res += val[i];
        }
    }
    return res;
}

namespace fs = std::filesystem;

std::string fill_from_scripts(const std::string &val) {
    std::string res;
    for (size_t i = 0; i < val.size(); i++) {
        if (val[i] == '$' && streq(val, i, i + 8, "$script:", 0, 8)) {
            int j = i + 8; 
            std::string cconst;
            while (j < (int)val.size() && val[j] != ' ') cconst += val[j], j++;

            std::string fixcconst = cfg::fixpath(cconst, "scripts");
            Logger::debug(fixcconst + std::to_string(fs::exists(fixcconst) && fs::is_regular_file(fixcconst)));
            if (fs::exists(fixcconst) && fs::is_regular_file(fixcconst)) res += "bash " + fixcconst;
            else res += "$script:" + cconst;

            i = j - 1;
        } else {
            res += val[i];
        }
    }
    Logger::debug(val + " -> " + res);
    return res;
}

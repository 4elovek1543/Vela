#include "config_manager.hpp"
#include "logger.hpp"
#include <vector>


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


YAML::Node cfg::config;
std::string cfg::cfgpath;


void cfg::init(const std::string &path_to_cfg) {
    try {
        cfgpath = path_to_cfg;
        config = YAML::LoadFile(path_to_cfg);
        Logger::debug("Main config loaded");
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

std::map<std::string, YAML::Node> cfg::load_constants() {
    std::map<std::string, YAML::Node> res;
    for (const auto &c : config["app"]["constants"]) {
        res[c.first.as<std::string>()] = c.second;
    }
    Logger::debug("Loaded constants (" + std::to_string(res.size()) + ")");
    return res;
}

YAML::Node loag_config_file(const std::string &path, YAML::Node defval) {
    try {
        return YAML::LoadFile(path);
    } catch (const YAML::Exception &e) {
        Logger::error("Couldn't load config: path: " + path + ", error: " + std::string(e.what()));
        return defval;
    }
}

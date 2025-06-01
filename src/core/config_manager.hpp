#pragma once

#include <string>
#include <map>
#include <yaml-cpp/yaml.h>


std::string PFOJECT_PATH = "../";



class cfg {
public:
    static void init(const std::string &path_to_cfg="../config/main.yaml");

    static YAML::Node get(const std::string &path, YAML::Node defval=YAML::Node());
    static int getint(const std::string &path, int defval=0);
    static float getfloat(const std::string &path, float defval=0.0);
    static bool getbool(const std::string &path, bool defval=false);
    static std::string getstring(const std::string &path, std::string defval="");

    static std::map<std::string, YAML::Node> load_constants();
private:
    static YAML::Node config;
    static std::string cfgpath;
};


YAML::Node load_config_file(const std::string &path, YAML::Node defval=YAML::Node());

YAML::Node getnode(YAML::Node config, const std::string &path, YAML::Node defval=YAML::Node());
int getint(YAML::Node config, const std::string &path, int defval=0);
float getfloat(YAML::Node config, const std::string &path, float defval=0.0);
bool getbool(YAML::Node config, const std::string &path, bool defval=false);
std::string getstring(YAML::Node config, const std::string &path, std::string defval="");
std::vector<YAML::Node> getseq(YAML::Node config, const std::string &path, std::vector<YAML::Node> defval=std::vector<YAML::Node>());

std::string fill_from_constants(std::string &val, const std::map<std::string, YAML::Node> &constants);
#include "notifier.hpp"
#include "logger.hpp"
#include <cstdlib>



void Notifier::notify(const std::string &msg, const std::string &ctg, const int time) {
    std::string action = "hyprctl notify ";

    std::string icon = "-1 ";
    std::string color = "rgb(255,255,255)";
    // specify via ctg 
    if (ctg == "info") {
        icon = "1 ";
        color = "rgb(29,171,67)";
        Logger::info("NOTIFY: " + msg);
    } else if (ctg == "warning") {
        icon = "2 ";
        color = "rgb(191,161,40)";
        Logger::warning("NOTIFY: " + msg);
    } else if (ctg == "error") {
        icon = "3 ";
        color = "rgb(148,50,25)";
        Logger::error("NOTIFY: " + msg);
    }

    action += icon;
    action += std::to_string(time) + " ";
    action += "\"" + color + "\" ";
    action += "\"" + msg + "\" ";

    int res = std::system(action.c_str());
    if (res != 0) Logger::error("Error while notify, code: " + std::to_string(res));
}

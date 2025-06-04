#pragma once

#include "logger.hpp"
#include <string>


class Notifier {
public:
    static void notify(const std::string &msg, const std::string &ctg, const int time=10000);
private:
};
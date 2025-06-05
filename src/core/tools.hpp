#pragma once


#include <iostream>
#include <string>
#include <vector>


std::vector<std::string> split(const std::string &s, const char sep='.');

std::string joinpath(const std::vector<std::string> &val);

std::string getExecutablePath();

bool streq(const std::string &s1, int p1, int e1, const std::string &s2, int p2, int e2);

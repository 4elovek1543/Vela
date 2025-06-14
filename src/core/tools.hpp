#pragma once


#include <iostream>
#include <string>
#include <vector>


std::vector<std::string> split(const std::string &s, const char sep='.');

std::string joinpath(const std::vector<std::string> &val);

std::string getExecutablePath();

bool streq(const std::string &s1, int p1, int e1, const std::string &s2, int p2, int e2);

std::string get_tmp_scriptpath(const std::string &name="tmp_script", std::string ext=".sh");

std::string add_logging(std::string command, const std::string &logpath=get_tmp_scriptpath("child_log", ".log"));

int execute_as_root(const std::string command, const std::string &logpath=get_tmp_scriptpath("child_log", ".log"));

void clear_tmp();

std::string get_window_address();

std::pair<int, int> get_window_size();

std::pair<int, int> get_window_pos();

std::pair<int, int> get_monitor_size();

std::pair<int, int> get_monitor_pos();

std::pair<int, int> get_cursor_pos();

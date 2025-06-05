#include "tools.hpp"
#include <unistd.h>
#include <limits.h> 


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


std::string getExecutablePath() {
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    if (count != -1) {
        return std::string(result, count);
    }
    return "";
}


bool streq(const std::string &s1, int p1, int e1, const std::string &s2, int p2, int e2) {
    if (e1 - p1 != e2 - p2) return false;
    for (int i = 0; i < e1 - p1; i++) if (s1[p1 + i] != s2[p2 + i]) return false;
    return true; 
} 
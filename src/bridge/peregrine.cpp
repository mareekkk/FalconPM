#include <unordered_map>
#include <string>
#include <mutex>
#include "atcommand.hpp"

using AtCmdFunc = int(*)(map_session_data*, const char*, const char*);

static std::unordered_map<std::string, AtCmdFunc> runtime_atcmds;
static std::mutex runtime_mtx;

// [EDIT] Renamed to fpm_atcommand_register
extern "C" bool fpm_atcommand_register(const char* name, AtCmdFunc fn) {
    if (!name || !fn) return false;
    std::lock_guard<std::mutex> g(runtime_mtx);
    if (runtime_atcmds.count(name)) return false;  // prevent duplicates
    runtime_atcmds[name] = fn;
    return true;
}

// [EDIT] Renamed to fpm_atcommand_unregister
extern "C" bool fpm_atcommand_unregister(const char* name) {
    std::lock_guard<std::mutex> g(runtime_mtx);
    return runtime_atcmds.erase(name) > 0;
}

// Dispatcher stays the same
bool atcommand_runtime_dispatch(map_session_data* sd, const char* message) {
    if (!message || (message[0] != '@' && message[0] != '#')) return false;

    std::string cmd;
    const char* p = message + 1;
    while (*p && *p != ' ' && *p != '\t') cmd.push_back(*p++);
    while (*p == ' ' || *p == '\t') ++p; // skip whitespace

    std::lock_guard<std::mutex> g(runtime_mtx);
    auto it = runtime_atcmds.find(cmd);
    if (it != runtime_atcmds.end()) {
        const char* args = *p ? p : "";
        it->second(sd, cmd.c_str(), args);
        return true;
    }
    return false;
}

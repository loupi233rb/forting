#include "convert.h"
#include <chrono>
#include <filesystem>

namespace Forting {
    tm filetime_to_tm(const fs::path& path) {
        if(!fs::exists(path)) {
            throw runtime_error("File does not exist: " + path.string());
        }
        auto ftime = fs::last_write_time(path);
        auto st = chrono::clock_cast<chrono::system_clock>(ftime);
        time_t cftime = chrono::system_clock::to_time_t(st);
        return *localtime(&cftime);
    }
}
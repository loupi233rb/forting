#ifndef CONVERT_H
#define CONVERT_H

#include "type.h"
#include <ctime>
#include <filesystem>
#include <string>

namespace Forting {
    tm filetime_to_tm(const fs::path& path);
}
#endif
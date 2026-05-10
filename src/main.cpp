#include "type.h"

#include "fortingcore.h"
#include "SyntaxParser.h"
#include <cstdio>
#include <filesystem>
#include <getopt.h>

const static std::string HELP_INFO = R"(
@@@@@@@@@@@
by loupi233
@@@@@@@@@@@
)";

int main(int argc, char *argv[])
{
    using namespace Forting;

    bool recursive = false;
    bool force = false;
    fs::path root_path = fs::current_path();
    fs::path target_path = fs::current_path();
    fs::path delete_path = fs::current_path() / "deleted";

    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"r", no_argument, nullptr, 'r'},
        {"f", no_argument, nullptr, 'f'},
        {"m", no_argument, nullptr, 'm'},
        {"root_path",1, nullptr, 'r'},
        {"target_path", 1, nullptr, 't'},
        {"delete_path",1, nullptr, 'd'},
        {nullptr, 0, nullptr, 0}
    };

    while ((opt = getopt_long(argc, argv, "hrft:d:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                std::cout << HELP_INFO;
                return 0;
            case 'r':
                recursive = true;
                break;
            case 'f':
                force = true;
                break;
            case 't':
                target_path = optarg;
                break;
            case 'd':
                delete_path = optarg;
                break;
            default:
                std::cerr << "Unknown option: " << opt << "\n";
                return 1;
        }
    }

    File file;
    file.init();
    file.current_path = root_path;
    file.target_path = target_path;
    file.delete_path = delete_path;


    return 0;
    
}

#include "type.h"

#include "fortingcore.h"
#include "SyntaxParser.h"
#include <cstdio>
#include <filesystem>
#include <getopt.h>

#ifdef DEBUG
const static std::string HELP_INFO =
R"(@@@@@@@@@@@
by loupi233
@@@@@@@@@@@
DEBUG_MOD
)";
#else
const static std::string HELP_INFO =
R"(@@@@@@@@@@@
by loupi233
@@@@@@@@@@@
)";
#endif

int main(int argc, char *argv[])
{
    using namespace Forting;

    bool recursive = false;
    bool force = false;
    fs::path root_path = fs::current_path();
    fs::path target_path = fs::current_path();
    fs::path delete_path = fs::current_path() / "deleted";
    fs::path code_path = "../template.txt";

    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"code", 1, nullptr, 'c'},
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
            case 'c':
                code_path = optarg;
                break;
            default:
                std::cerr << "Unknown option: " << opt << "\n";
                return 1;
        }
    }

    root_path.lexically_normal();
    target_path.lexically_normal();
    delete_path.lexically_normal();

    File file;
    SyntaxParser parser;

    file.init();
    parser.loadFromFile(code_path.string());
    file.current_path = root_path;
    file.target_path = target_path;
    file.delete_path = delete_path;

    file.Walk(recursive);
    file.run(parser.run(file.getFileList()), force);

    return 0;
}

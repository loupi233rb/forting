#include "type.h"

#include "fortingcore.h"
#include "SyntaxParser.h"
#include <clocale>
#include <filesystem>
#include <getopt.h>
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef DEBUG
const static std::string HELP_INFO =
R"(@@@@@@@@@@@
by loupi233
@@@@@@@@@@@
DEBUG_MOD
)";
#else
const static std::string HELP_INFO =
R"(Usage: forting [options]... [path]
Parse and execute forting code to sort files in the specified path.

Options:
  -h, --help            Show this help message and exit
  -c, --code <file>    Specify the forting code file
  -r, --recursive       Recursively process subdirectories
  -f, --force           Force move files even if target exists
  -p, --path <path> Specify root directory to sort (default: current directory)
  -t, --target_path <path> Specify target directory for sorted files (default: current directory)
  -d, --delete_path <path> Specify directory to move deleted files with its original name(default: current directory/deleted)
  -m, --move            Move files instead of copying (default: copy)
)";
#endif

int main(int argc, char *argv[])
{
    using namespace Forting;

    std::setlocale(LC_ALL, ".UTF-8");
    #ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    #endif

    bool is_recursive = false;
    bool is_force = false;
    bool is_move = false;
    fs::path root_path = fs::current_path();
    fs::path target_path = fs::current_path();
    fs::path delete_path = fs::current_path() / "deleted";
    fs::path code_path = "../template.txt";

    int opt;
    int option_index = 0;
    static struct option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"code", 1, nullptr, 'c'},
        {"recursive", no_argument, nullptr, 'r'},
        {"force", no_argument, nullptr, 'f'},
        {"move", no_argument, nullptr, 'm'},
        {"path",1, nullptr, 'p'},
        {"target_path", 1, nullptr, 't'},
        {"delete_path",1, nullptr, 'd'},
        {"h", no_argument, nullptr, 'h'},
        {"c", 1, nullptr, 'c'},
        {"r", no_argument, nullptr, 'r'},
        {"f", no_argument, nullptr, 'f'},
        {"m", no_argument, nullptr, 'm'},
        {"p",1, nullptr, 'p'},
        {"t", 1, nullptr, 't'},
        {"d",1, nullptr, 'd'},
        {nullptr, 0, nullptr, 0}
    };

    while ((opt = getopt_long(argc, argv, "hc:rfmp:t:d:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                std::cout << HELP_INFO;
                return 0;
            case 'r':
                is_recursive = true;
                break;
            case 'f':
                is_force = true;
                break;
            case 'm':
                is_move = true;
                break;
            case 'p':
                root_path = optarg;
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

    file.Walk(is_recursive);
    file.run(parser.run(file.getFileList()), is_force, is_move);

    return 0;
}

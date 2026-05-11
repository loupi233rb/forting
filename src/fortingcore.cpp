#include "fortingcore.h"
#include "convert.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <string>

namespace Forting
{

#ifdef DEBUG
void remove_file(const fs::path& p) {
    cout << "R: " << fs::absolute(p) << "\n";
}
void copy_file(const fs::path& from, const fs::path& to) {
    cout << "C: " << fs::absolute(from) << " -> " << fs::absolute(to) << "\n";
}
void move_file(const fs::path& from, const fs::path& to) {
    cout << "M: " << fs::absolute(from) << " -> " << fs::absolute(to) << "\n";
}
void create_dirs(const fs::path& p) {
    cout << "MKDIR: " << fs::absolute(p) << "\n";
}
#else
void remove_file(const fs::path& p) {
        std::error_code ec;
        fs::remove(p, ec);
        if (ec) {
            std::cerr << "Error removing " << p << ": " << ec.message() << "\n";
        }
}
void copy_file(const fs::path& from, const fs::path& to) {
        std::error_code ec;
        fs::copy_file(from, to, fs::copy_options::overwrite_existing, ec);
        if (ec) {
            std::cerr << "Error copying " << from << " to " << to << ": " << ec.message() << "\n";
        }
}
void move_file(const fs::path& from, const fs::path& to) {
        std::error_code ec;
        fs::rename(from, to, ec);
        if (ec) {
            std::cerr << "Error moving " << from << " to " << to << ": " << ec.message() << "\n";
        }
}
void create_dirs(const fs::path& p) {
        std::error_code ec;
        fs::create_directories(p, ec);
        if (ec) {
            std::cerr << "Error creating directories " << p << ": " << ec.message() << "\n";
        }
}
#endif


void File::Walk(bool recursive) {
    this->FileList.clear();
    this->srcPaths.clear();
    if(!fs::exists(this->current_path) || !fs::is_directory(this->current_path)) {
        cerr << "Invalid current_path path: " << this->current_path << endl;
        return;
    }
    auto process_entry = [](const fs::directory_entry& entry, vector<FileEntry>& fileList, vector<fs::path>& srcPaths) {
        if(entry.is_directory()) return;
        FileEntry fe;
        fe.name = entry.path().stem().string();
        fe.size = entry.is_regular_file() ? entry.file_size() : 0;
        fe.lw_time = filetime_to_tm(entry.path());
        fe.suffix = entry.path().extension().string();
        fileList.push_back(std::move(fe));
        srcPaths.push_back(entry.path());
    };
    fs::recursive_directory_iterator d_it = fs::recursive_directory_iterator(this->current_path, fs::directory_options::skip_permission_denied);
    if(recursive) {
        for(auto &entry : fs::recursive_directory_iterator(this->current_path, fs::directory_options::skip_permission_denied)) {
            process_entry(entry, this->FileList, this->srcPaths);
        }
    }
    else {
        for(auto &entry : fs::directory_iterator(this->current_path, fs::directory_options::skip_permission_denied)) {
            process_entry(entry, this->FileList, this->srcPaths);
        }
    }
}


bool File::FileListWriteToTxt(){
    std::ofstream outfile("FortingFileListInfo.txt");
    if (!outfile.is_open()) {
        return false;
    }
    for(size_t i = 0; i < this->FileList.size(); ++i) {
        const auto& fe = this->FileList[i];
        outfile << fe.name;
        outfile << " " << fe.size;
        outfile << " " << this->srcPaths[i].string();
        outfile << "\n";
    }
    outfile.close();
    return true;
}

File::File() {
    this->current_path = fs::current_path();
}

void File::init() {
    this->FileList.clear();
    this->current_path = fs::current_path();
    #ifdef DEBUG
    this->Walk(true);
    this->FileListWriteToTxt();
    #else
    this->Walk(false);
    #endif
}

void File::run(const std::vector<action>& acts, bool is_force, bool is_move) {
    bool hasDelete = false;
    while(fs::exists(this->delete_path)) {
        this->delete_path.concat("_");
    }
    for(size_t i = 0; i < acts.size(); ++i) {
        const auto& act = acts[i];
        const auto& fe = this->FileList[i];
        string fileName = acts[i].renameValue.empty() ? srcPaths[i].filename().string():(acts[i].renameValue);
        fs::path target = this->target_path / act.paths;
        if(!fs::exists(target)) {
            Forting::create_dirs(target);
        }
        if(act.deleteFlag) {
            if(!hasDelete) {
                Forting::create_dirs(this->delete_path);
                hasDelete = true;
            }
            Forting::move_file(this->srcPaths[i], this->delete_path / fileName);
        }
        else {
            fs::path dest = target / fileName;
            if(fs::exists(dest)) {
                if(is_force) {
                    Forting::remove_file(dest);
                }
                else {
                    cerr << "Target file already exists: " << dest << ". Use --force to overwrite.\n";
                    continue;
                }
            }
            if(is_move) {
                Forting::move_file(this->srcPaths[i], dest);
            }
            else {
                Forting::copy_file(this->srcPaths[i], dest);
            }
        }
    }
}
}

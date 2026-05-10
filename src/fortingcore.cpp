#include "fortingcore.h"
#include "convert.h"

#include <fstream>
#include <filesystem>
#include <iostream>

namespace Forting
{
    void File::Walk(bool recursive) {
        if(!fs::exists(this->current_path) || !fs::is_directory(this->current_path)) {
            cerr << "Invalid current_path path: " << this->current_path << endl;
            return;
        }
        auto process_entry = [](const fs::directory_entry& entry, vector<FileEntry>& fileList) {
            if(entry.is_directory()) return;
            FileEntry fe;
            fe.name = entry.path().filename().string();
            fe.size = entry.is_regular_file() ? entry.file_size() : 0;
            fe.lw_time = filetime_to_tm(entry.path());
            fe.suffix = entry.path().extension().string();
            fileList.push_back(std::move(fe));
        };
        fs::recursive_directory_iterator d_it = fs::recursive_directory_iterator(this->current_path, fs::directory_options::skip_permission_denied);
        if(recursive) {
            for(auto &entry : fs::recursive_directory_iterator(this->current_path, fs::directory_options::skip_permission_denied)) {
                process_entry(entry, this->FileList);
            }
        }
        else {
            for(auto &entry : fs::directory_iterator(this->current_path, fs::directory_options::skip_permission_denied)) {
                process_entry(entry, this->FileList);
            }
        }
    }


    bool File::FileListWriteToTxt(){
        std::ofstream outfile("FortingFileListInfo.txt");
        if (!outfile.is_open()) {
            return false;
        }
        for(auto &fe : this->FileList) {
            outfile << fe.name;
            outfile << " " << fe.size << "\n";
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

    void File::run(const std::vector<action>& acts, bool force) {
        while(fs::exists(this->delete_path)) {
            this->delete_path.concat("_");
        }
        for(const auto& act: acts) {

        }
    }

}

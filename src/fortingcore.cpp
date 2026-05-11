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

    void File::run(const std::vector<action>& acts, bool force) {
        bool hasDelete = false;
        while(fs::exists(this->delete_path)) {
            this->delete_path.concat("_");
        }
        for(size_t i = 0; i < acts.size(); ++i) {
            const auto& act = acts[i];
            const auto& fe = this->FileList[i];
            bool rename = !act.renameValue.empty();
            fs::path target = this->target_path / act.paths;
            #ifndef DEBUG
            if(!fs::exists(target)) {
                fs::create_directories(target);
            }
            #endif
            if(act.deleteFlag) {
                if(!hasDelete) {
                    fs::create_directories(this->delete_path);
                    hasDelete = true;
                }
                if(force) {
                    fs::remove(this->srcPaths[i]);
                }
                else {
                    fs::rename(this->srcPaths[i], this->delete_path / (rename? act.renameValue : fe.name));
                }
            }
            else if(rename) {
                #ifdef DEBUG
                cout<<target / act.renameValue<<endl;
                #endif
                #ifndef DEBUG
                fs::rename(this->srcPaths[i], target / act.renameValue);
                #endif
            }
            else {
                #ifdef DEBUG
                cout<<target / srcPaths[i].filename().string()<<endl;
                #endif
                #ifndef DEBUG
                fs::rename(this->srcPaths[i], target / srcPaths[i].filename().string());
                #endif
            }
        }
    }

}

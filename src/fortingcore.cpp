#include "fortingcore.h"
#include "convert.h"

#include <fstream>
#include <filesystem>
#include <iostream>

namespace Forting
{
    void File::Walk(bool recursive) {
        if(!fs::exists(this->root) || !fs::is_directory(this->root)) {
            cerr << "Invalid root path: " << this->root << endl;
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
        fs::recursive_directory_iterator d_it = fs::recursive_directory_iterator(this->root, fs::directory_options::skip_permission_denied);
        if(recursive) {
            for(auto &entry : fs::recursive_directory_iterator(this->root, fs::directory_options::skip_permission_denied)) {
                process_entry(entry, this->FileList);
            }
        }
        else {
            for(auto &entry : fs::directory_iterator(this->root, fs::directory_options::skip_permission_denied)) {
                process_entry(entry, this->FileList);
            }
        }
    }


    bool File::FileListWriteToTxt(){
        std::ofstream outfile("FortingFileListInfo.txt");
        if (!outfile.is_open()) {
            return false;
        }
        for(auto &it: FileIndexList) {
            auto item = FileList[it];
            outfile << item.name;
            outfile << " " << item.size << "\n";
        }
        outfile.close();
        return true;
    }

    File::File() {
        this->root = fs::current_path();
    }

    void File::init() {
        this->FileList.clear();
        this->FileIndexList.clear();
        this->root = fs::current_path();
        #ifdef DEBUG
        this->Walk(true);
        #else
        this->Walk(false);
        #endif
        for(int i=0;i<this->FileList.size();i++) FileIndexList.push_back(i);
        this->FileListWriteToTxt();
    }

    // Sort::Sort(File& f) {
    //     this->file = &f;
    // }

    /*
    GTNptr Sort::buildGroupTree() {
        GTNptr rootptr = std::make_unique<GroupTreeNode>();
        rootptr->tag = "__root__";
        int len = this->file->fileListLen();
        for(int i=0;i<len;++i) {
            auto cur = rootptr.get();
            for(auto &t: Tag) {
                const QString& k = t.AllFileClassName[t.FileClassNameIndex[i]];
                if(!cur->children.contains(k)) {
                    auto child = std::make_unique<GroupTreeNode>();
                    child->tag = k;
                    cur->children[k] = std::move(child);
                }
                cur = cur->children[k].get();
                cur->indices.push_back(i);
            }
            cur->indices.push_back(i);
        }
        return rootptr;
    }
        */

}

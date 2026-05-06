#ifndef FORTINGCORE_H
#define FORTINGCORE_H

#include "type.h"
// #include "airuleparser.h"

#include <filesystem>
#include <sys/stat.h>

namespace Forting
{
    class File
    {
    private:
        fs::path root;  //working dir
        bool FileListWriteToTxt();
        vector<FileEntry> FileList;
        vi FileIndexList;

    public:
        File();
        void init();
        int fileListLen() { return FileList.size(); }
        void Walk(bool recursive = false);
        auto getFileList() const { return FileList; }
    
    };

    // using GTNptr = std::unique_ptr<GroupTreeNode>;

    // class Sort
    // {
    // private:
    //     File* file;
    //     RuleParser* parser;
    //     vector<FortingLayer> AllActions;

    //     Sort(File& f);
    //     bool bindParser(RuleParser* rp);
    //     void treeToTxt();
    //     void work();
    //     // GTNptr buildGroupTree();
    // };
}

#endif

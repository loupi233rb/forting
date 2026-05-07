#ifndef FORTINGCORE_H
#define FORTINGCORE_H

#include "type.h"
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

    public:
        File();
        void init();
        int fileListLen() { return FileList.size(); }
        void Walk(bool recursive = false);
        auto getFileList() const { return FileList; }
    };

}

#endif

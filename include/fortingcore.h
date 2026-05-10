#ifndef FORTINGCORE_H
#define FORTINGCORE_H

#include "type.h"
#include <filesystem>
#include <sys/stat.h>
#include <vector>

namespace Forting
{
    class File
    {
    private:
        bool FileListWriteToTxt();
        vector<FileEntry> FileList;

    public:
        File();
        fs::path current_path;
        fs::path target_path;
        fs::path delete_path;
        void init();
        int fileListLen() { return FileList.size(); }
        void Walk(bool recursive = false);
        void run(const std::vector<action>& acts, bool force=false);
        const auto getFileList() const { return FileList; }
    };

}

#endif

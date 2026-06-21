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
        vector<fs::path> srcPaths;

    public:
        File();
        fs::path current_path;
        fs::path target_path;
        fs::path delete_path;
        void init();
        int fileListLen() { return FileList.size(); }
        void Walk(bool recursive = false);
        void run(const std::vector<action>& acts, bool is_force=false, bool is_move=false);
        const auto getFileList() const { return FileList; }
        const auto getSrcPaths() const { return srcPaths; }
    };

    void remove_file(const fs::path& p);
    void copy_file(const fs::path& from, const fs::path& to);
    void move_file(const fs::path& from, const fs::path& to);
    void create_dirs(const fs::path& p);

}

#endif

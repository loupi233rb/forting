#include "fortingcore.h"

#include <algorithm>

#include <QDirIterator>
#include <QSaveFile>
#include <QTextStream>

namespace Forting
{
    void File::Walk() {
        QDirIterator it(this->root,
                        QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        while(it.hasNext()) {
            it.next();
            FileEntry fe;
            QFileInfo fi = it.fileInfo();
            if(fi.isDir()) continue;
            fe.name = fi.fileName();
            fe.suffix = fi.suffix().remove(".").toLower();
            fe.isFile = fi.isFile();
            fe.isDir = fi.isDir();
            fe.size = fi.size();
            fe.created = fi.birthTime();
            fe.modified = fi.lastModified();
            FileList.push_back(fe);
        }
    }

    bool File::FileListWriteToTxt(){
        QSaveFile outfile(QDir::current().absoluteFilePath("FortingFileListInfo.txt"));
        if (!outfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return false;
        }
        QTextStream out(&outfile);
        out.setEncoding(QStringConverter::Utf8);

        for(auto &it: FileIndexList) {
            auto item = FileList[it];
            out <<item.name;
            if(item.isFile) out<<" "<<item.size<<"\n";
            else out<<"\n";
        }
        return outfile.commit();
    }

    void File::sort_by(SortKey type, SortKey dir) {
        auto less = [&](int a, int b) {
            const FileEntry& A = this->FileList[a];
            const FileEntry& B = this->FileList[b];

            if(A.isFile != B.isFile) return A.isFile > B.isFile;
            switch (type) {
            case SortKey::suffix :
                return QString::compare(A.suffix,B.suffix, Qt::CaseInsensitive) < 0;
            case SortKey::size :
                return A.size < B.size;
            case SortKey::created :
                return A.created < B.created;
            case SortKey::last_modified :
                return A.modified < B.modified;
            default :
                return QString::compare(A.name,B.name, Qt::CaseInsensitive) < 0;
            }
        };
        std::sort(FileIndexList.begin(),FileIndexList.end()
                  ,[&](int a,int b)
                  {return (dir==SortKey::Asc)?less(a,b):less(b,a);});
    }

    File::File() {
        this->root = nullptr;
    }

    void File::init() {
        this->FileList.clear();
        this->FileIndexList.clear();
        this->root = QDir::currentPath();
        this->Walk();
        for(int i=0;i<this->FileList.size();i++) FileIndexList.push_back(i);
    #ifdef QT_DEBUG
        this->sort_by(SortKey::size, SortKey::Desc);
        this->FileListWriteToTxt();
    #endif
    }

    Sort::Sort(File& f) {
        this->file = &f;
    }
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

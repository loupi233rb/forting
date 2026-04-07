#ifndef FORTINGCORE_H
#define FORTINGCORE_H

#include "fortstruct.h"
#include "airuleparser.h"

#include <memory>
#include <QDir.h>
#include <QDateTime>
#include <QString>
#include <QVector>

namespace Forting
{
    class File : public QObject
    {
        Q_OBJECT

    private:
        QString root;
        void Walk();
        bool FileListWriteToTxt();
        QVector<FileEntry> FileList;
        vi FileIndexList;

        void init();
        void sort_by(SortKey type, SortKey dir = SortKey::Asc);
    public:
        explicit File(QObject *parent = nullptr);
        int fileListLen() { return FileList.size(); }
    };

    using GTNptr = std::unique_ptr<GroupTreeNode>;

    class Sort
    {
    private:
        File* file;
        RuleParser* parser;
        QVector<FortingLayer> AllActions;

        Sort(File& f);
        bool bindParser(RuleParser* rp);
        void treeToTxt();
        void work();
        GTNptr buildGroupTree();
    };
}

#endif // FORTINGCORE_H

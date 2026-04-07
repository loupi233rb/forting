#ifndef FORTSTRUCT_H
#define FORTSTRUCT_H

#include <vector>
#include <map>

#include <QString>
#include <QDateTime>

namespace Forting
{
enum class SortKey { name=Qt::UserRole+1, size, last_modified, created, suffix, Asc, Desc };
enum class decision {
    Delete=1, Tag=2, Rename=4
};

struct action {
    int decision = 0;     // bit: tag=4 rename=2 delete=1
    QString tagValue;     // tag(...) 结果
    QString renameValue;  // rename(...) 结果
    bool deleteFlag = false;
};

using Index = int;
using vi = std::vector<Index>;
using vvi = std::vector<std::vector<Index>>;

struct FileEntry {
    QString name;
    QString suffix;
    bool isDir;
    bool isFile;
    qint64 size;
    QDateTime created;
    QDateTime modified;
};  //文件项

struct FortingLayer {
    Index LayerIndex;
    QVector<action> actions;
};

struct GroupTreeNode {
    QString tag;
    std::map<QString,std::unique_ptr<GroupTreeNode>> children;
    vi indices;
};  //类别树节点

}

#endif // FORTSTRUCT_H

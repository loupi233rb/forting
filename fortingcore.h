#ifndef FORTINGCORE_H
#define FORTINGCORE_H

#include "fortstruct.h"
#include "airuleparser.h"

#include <memory>
#include <QDir>
#include <QDateTime>
#include <QString>
#include <QVector>
#include <QAbstractListModel>

namespace Forting
{
    class File : public QAbstractListModel
    {
        Q_OBJECT

    private:
        double m_value;
        QString root;  //working dir
        bool FileListWriteToTxt();
        QVector<FileEntry> FileList;
        vi FileIndexList;

    public:
        explicit File(QObject *parent = nullptr);
        void init();
        int fileListLen() { return FileList.size(); }
        /*
        double value() const {return m_value;}
        void setValue(double v)
        {
            if(qFuzzyCompare(m_value,v)) return;
            m_value = v;
            emit valueChanged();
        }
        */

        int rowCount(const QModelIndex &parent = QModelIndex()) const override;
        QVariant data(const QModelIndex &index, int role) const override;
        QHash<int, QByteArray> roleNames() const override;

        /*
        Q_INVOKABLE void add1() {setValue(m_value+1);}
        Q_INVOKABLE void sub1() {setValue(m_value-1);}
        Q_INVOKABLE void mul10() {setValue(m_value*10);}
        Q_INVOKABLE void div10() {setValue(m_value/10);}
        */
        // 每次改变都要在开始和末尾加上beginResetModel()和endResetModel()，以通知视图更新
        Q_INVOKABLE void Walk();
        Q_INVOKABLE void sort_by(SortKey type, SortKey dir = SortKey::Asc);
    signals:
        void valueChanged();
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

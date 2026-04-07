#ifndef FORTINGCORE_H
#define FORTINGCORE_H

#include "fortstruct.h"
#include "airuleparser.h"

#include <memory>
#include <QDir>
#include <QDateTime>
#include <QString>
#include <QVector>

namespace Forting
{
    class File : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged)

    private:
        double m_value;
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
        double value() const {return m_value;}
        void setValue(double v)
        {
            if(qFuzzyCompare(m_value,v)) return;
            m_value = v;
            emit valueChanged();
        }
        Q_INVOKABLE void add1() {setValue(m_value+1);}
        Q_INVOKABLE void sub1() {setValue(m_value-1);}
        Q_INVOKABLE void mul10() {setValue(m_value*10);}
        Q_INVOKABLE void div10() {setValue(m_value/10);}
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

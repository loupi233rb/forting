#ifndef TYPE_H
#define TYPE_H

#include <iostream>
#include <vector>
#include <ctime>
#include <string>
#include <filesystem>

namespace Forting
{
    using namespace std;
    namespace fs = filesystem;
    using Index = int;
    using vi = std::vector<Index>;
    using vvi = std::vector<std::vector<Index>>;

    enum class SortKey { name, size, last_modified, created, suffix, Asc, Desc };
    enum class decision { Delete=1, Tag=2, Rename=4 };

    struct action {
        int decision = 0;     // bit: tag=4 rename=2 delete=1
        string tagValue;     // tag(...) 结果
        string renameValue;  // rename(...) 结果
        bool deleteFlag = false;
    };

    struct FileEntry {
        string name;
        string suffix;
        int64_t size;
        tm lw_time;
    };

    using FortingLayer = vector<action>;  // 单层分类结果索引和FileList对应

    // struct GroupTreeNode {
    //     Index layerIndex;  //-1表示根节点
    //     string tag;
    //     vi indices;
    //     std::map<string,std::unique_ptr<GroupTreeNode>> children;
    // };  //类别树节点

    inline void printActions(const vector<action>& acts, ostream& os = cout) {
        os << "Actions:\n";
        for (size_t i = 0; i < acts.size(); ++i) {
            const auto& a = acts[i];
            os << "Unit[" << i << "] "
            << "decision=" << a.decision
            << " tag=\"" << a.tagValue << "\""
            << " rename=\"" << a.renameValue << "\""
            << " delete=" << (a.deleteFlag ? "true" : "false")
            << "\n";
        }
    }

}
#endif
# forting
按照编写的forting code对文件进行整理
### forting code介绍
示例如下：
```
white("folderName","whitelistFolder")
if conditions :             ------------
action1,action2,...,;                  |
elif conditions:                       |
......;                                |---- a unit
else conditons    : actions;           |
end                         ------------

when attibution:
=x : action;
!= y :
action;

end

action;
```
主体语法如上
一个单元代表一层文件夹分类，多个单元可实现嵌套分类
其中condition可以是使用 || && () !连接的任意布尔表达式
空格和空行不严格，但; end不可省略

代码开头可以指定黑白名单模式，但只能指定一种或者不指定，内部可有多个文件夹名字，只要文件相对于工作目录的路径中包含任一名字均会满足条件，包括文件名含有
```
root: d:\temp\
file: d:\temp\old\st.txt
relative: old\st.txt
black("st") -> exclude
white("old") -> include
```

action为
```
tag("tagValue")
rename("renameValue")
delete
nothing
```

attribution，也即属性有
```
name                                // 除去后缀的文件名，要使用""包裹
suffix                              // 后缀，无需""包裹
size                                // 大小，比较时可带单位
date                                // 格式为yyyy-mm-dd
time                                // 格式为hh-mm-ss
date.year date.month date.day       // 对应的阿拉伯数字
time.hour time.minute time.second   // 对应的阿拉伯数字
```
tag和rename的值可用字符串拼接，例如
```
rename(name+"-"size+"suffix")       // test.txt 16.3kb -> 
                                    // test-16.3kb.txt
```

### 使用
使用cmake进行构建
```
git clone https://github.com/loupi233rb/forting
cd forting
mkdir build && cd build
cmake ..
make
```
> 如需测试功能，可使用Debug模式编译，此时为只读的预览模式，预览操作不会创建任何文件夹以及对文件进行任何操作
> `cmake .. -DCMAKE_BUILD_TYPE=Debug`


`forting -h(--help)`查看使用说明



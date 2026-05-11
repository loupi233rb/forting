# forting
按照编写的forting code对文件进行整理
### forting code介绍
示例如下：
```
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

action为
```
tag("tagValue")
rename("renameValue")
delete
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
`forting -h(--help)`查看使用说明



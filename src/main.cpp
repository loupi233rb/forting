#include "type.h"

#include "fortingcore.h"
#include "SyntaxParser.h"
#include <cstdio>
#include <filesystem>



int main(int argc, char *argv[])
{
    using namespace Forting;

    // File file;
    // file.init();

    // SyntaxParser parser;
    // parser.loadFromFile("../template.txt");

    // printActions(parser.run(file.getFileList())[0]);

    // printActions(parser.run(file.getFileList())[1]);

    fs::path p ="e:/gewu/";
    string s = "name/";
    string s_ = "size/";
    p.concat(s);
    p.concat(s_);
    p.lexically_normal();
    p.make_preferred();
    cout<<p;
    

    return 0;
    
}

#ifndef VARCONTEXT_H
#define VARCONTEXT_H

#include <string>
#include <map>
#include <iostream>
using std::map;
using std::cout;
using std::endl;
using std::string;

//定义 变量上下文类，保存变量及变量的值
class VarContext
{
public:
    void setValue(std::string name, int value)
    {
        if (!defined(name)) symbolStat[name] = 0;
        symbolMap[name] = value;
    }
    int getValue(std::string name);
    bool defined(std::string name)
    {
        return (symbolMap.find(name) != symbolMap.end());
    }
    map<std::string, int> symbolMap;
    map<std::string, int> symbolStat;
};

#endif // VARCONTEXT_H

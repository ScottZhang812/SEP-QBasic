#include "varcontext.h"

int VarContext::getValue(std::string name)
{
    if (symbolMap.find(name) != symbolMap.end())
    {
        symbolStat[name]++;
        return symbolMap[name];
    }
    else throw int(4);
}

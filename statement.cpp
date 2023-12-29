#include "statement.h"

void IfStm::printTree()
{
    //return "IF "+sons[0]->toString()+" "+sons[1]->toString()+" "+sons[2]->toString()+" THEN "+sons[3]->toString();
    //emit printTreeLineSignal(0, std::to_string(lineNum) + " IF THEN");
    sons[0]->sendTree(4);
    sons[1]->sendTree(4);
    sons[2]->sendTree(4);
    sons[3]->sendTree(4);
}

void IfStm::doit(int& pc)
{
    int leftVal = sons[0]->calc(context), rightVal = sons[2]->calc(context);
    string op = sons[1]->toString();
    bool logicVal;
    if (op == "<") logicVal = (leftVal < rightVal);
    else if (op == "=") logicVal = (leftVal == rightVal);
    else if (op == ">") logicVal = (leftVal > rightVal);
    else throw int(3);
    if (logicVal) pc = stoi(sons[3]->toString());
}

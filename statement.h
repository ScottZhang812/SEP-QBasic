#ifndef STATEMENT_H
#define STATEMENT_H

#include <string>
#include <vector>
#include <expression.h>
#include <QObject>
#include <QDebug>
using std::string;
using std::vector;
class Expression;
enum statementType {INPUT, REM, END, LET, IF, GOTO, PRINT};
//namespace Ui { class MainWindow; }
//class MainWindow;

//定义 语句类，通过继承实现不同语句
class Statement : public QObject
{
    Q_OBJECT
public:
    Statement(QObject* parent = nullptr): QObject(parent) {}
    Statement(VarContext* cc, int ln, QObject* parent = nullptr): QObject(parent)
    {
        context = cc;
        lineNum = ln;
    }
    virtual statementType getType() = 0;
    virtual void printTree() = 0;
    virtual void doit(int& pc) = 0;
    //assigned by Statement()
    VarContext* context;
    //assigned by xxxStm()
    int sonNum = 0;
    int lineNum;
    vector<Expression*> sons;
    int stat = 0, Lstat = 0, Rstat = 0;
signals:
    void printTreeLineSignal(int bias, string str);
    void printAnsLineSignal(int ans);

};
class InputStm: public Statement
{
public:
    InputStm(VarContext* cc, int ln, Expression* exp1): Statement(cc, ln)
    {
        sonNum = 1;
        sons.push_back(exp1);
    }
    virtual statementType getType()
    {
        return INPUT;
    }
    virtual void printTree() //用于在syntax tree框中打印内容
    {
        //return "INPUT " + sons[0]->toString();
        //emit printTreeLineSignal(0, std::to_string(lineNum) + " INPUT");
        sons[0]->sendTree(4);
        //qDebug() <<"signal sent\n";
    }
    virtual void doit(int& pc)
    {
        //window->ui->cmdLineEdit->setText(" ? ");
        //window->isInputting = 1;
        //window->inputName = sons[0]->toString();
    }
};
class RemStm: public Statement
{
public:
    RemStm(VarContext* cc, int ln, Expression* p1): Statement(cc, ln)
    {
        sonNum = 1;
        sons.push_back(p1);
    }
    virtual statementType getType()
    {
        return REM;
    }
    virtual void printTree() //用于在syntax tree框中打印内容
    {
        //return "REM " + sons[0]->toString();
        //emit printTreeLineSignal(0, std::to_string(lineNum) + " REM");
        sons[0]->sendTree(4);
    }
    virtual void doit(int& pc) {}
};
class LetStm: public Statement
{
public:
    LetStm(VarContext* cc, int ln, Expression* leftIdenExp, Expression* rightValExp)
        : Statement(cc, ln)
    {
        sonNum = 2;
        sons.push_back(leftIdenExp);
        sons.push_back(rightValExp);
    }
    virtual statementType getType()
    {
        return LET;
    }
    virtual void printTree() //用于在syntax tree框中打印内容
    {
        //return "LET " + sons[0]->toString() + " = " + sons[1]->toString();
        //emit printTreeLineSignal(0, std::to_string(lineNum) + " LET = ");
        //sons[0]->sendTree(4);
        sons[1]->sendTree(4);
    }
    virtual void doit(int& pc) //执行语句功能
    {
        int rightVal = sons[1]->calc(context);
        context->setValue(sons[0]->toString(), rightVal);
    }
};
class IfStm: public Statement
{
public:
    IfStm(VarContext* cc, int ln, Expression* leftExp, Expression* opExp, Expression* rightExp, Expression* targetExp)
        : Statement(cc, ln)
    {
        sonNum = 4;
        sons.push_back(leftExp);
        sons.push_back(opExp);
        sons.push_back(rightExp);
        sons.push_back(targetExp);
    }
    virtual statementType getType()
    {
        return IF;
    }
    virtual void printTree(); //用于在syntax tree框中打印内容
    virtual void doit(int& pc); //执行语句功能
};
class GotoStm: public Statement
{
public:
    GotoStm(VarContext* cc, int ln, Expression* targetExp)
        : Statement(cc, ln)
    {
        sonNum = 1;
        sons.push_back(targetExp);
    }
    virtual statementType getType()
    {
        return GOTO;
    }
    virtual void printTree() //用于在syntax tree框中打印内容
    {
        //return "GOTO "+sons[0]->toString();
        //emit printTreeLineSignal(0, std::to_string(lineNum) + " GOTO");
        sons[0]->sendTree(4);
    }
    virtual void doit(int& pc) //执行语句
    {
        pc = stoi(sons[0]->toString());
    }
};
class PrintStm: public Statement
{
public:
    PrintStm(VarContext* cc, int ln, Expression* Exp1)
        : Statement(cc, ln)
    {
        sonNum = 1;
        sons.push_back(Exp1);
    }
    virtual statementType getType()
    {
        return PRINT;
    }
    virtual void printTree() //用于在syntax tree框中打印内容
    {
        //return "PRINT "+sons[0]->toString();
        //emit printTreeLineSignal(0, std::to_string(lineNum) + " PRINT");
        sons[0]->sendTree(4);
    }
    virtual void doit(int& pc)
    {
        //window->ui->textBrowser->append(QString::number(sons[0]->calc(context)));
    }
};
class EndStm: public Statement
{
public:
    EndStm(VarContext* cc, int ln): Statement(cc, ln)
    {
    }
    virtual statementType getType()
    {
        return END;
    }
    virtual void printTree()
    {
        //return "REM " + sons[0]->toString();
        //emit printTreeLineSignal(0, std::to_string(lineNum) + " END");
    }
    virtual void doit(int& pc) {}
};

#endif // STATEMENT_H

#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <varcontext.h>
#include <QObject>
#include <QDebug>
//#include <mainwindow.h>

//class MainWindow;
enum expressionType {CONSTANT, IDENTIFIER, COMPOUND, SPECIAL};

//定义 表达式类，通过继承实现不同表达式
class Expression : public QObject
{
    Q_OBJECT
public:
    Expression(QObject* parent = nullptr): QObject(parent)
    {
        //connect(this, &Expression::printTreeLineSignal, window, &MainWindow::printTreeLine);
    };
    virtual ~Expression() {};
    virtual expressionType getType() = 0;
    virtual void sendTree(int curBias) = 0; //用于在syntax tree框中打印内容
    virtual std::string toString() = 0;
    virtual int calc(VarContext* context) = 0;
    virtual char getOperator()
    {
        return '!';
    };
    virtual Expression* getLson()
    {
        return nullptr;
    };
    virtual Expression* getRson()
    {
        return nullptr;
    };
    virtual std::string getIdentifier()
    {
        return "error";
    };
    virtual int getConstant()
    {
        return -2147;
    };
    //virtual std::string getContent();
signals:
    void printTreeLineSignal(int bias, string str);
};
class ConstExp: public Expression
{
public:
    ConstExp(int x)
    {
        val = x;
    }
    virtual expressionType getType()
    {
        return CONSTANT;
    }
    virtual void sendTree(int curBias)
    {
        emit printTreeLineSignal(curBias, std::to_string(val));
    }
    virtual std::string toString()
    {
        return std::to_string(val);
    }
    virtual int calc(VarContext* context)
    {
        return val;
    }
    virtual int getConstant()
    {
        return val;
    }
private:
    int val;
};
class IdenExp: public Expression
{
public:
    IdenExp( std::string s)
    {
        name = s;
    }
    virtual expressionType getType()
    {
        return IDENTIFIER;
    }
    virtual void sendTree(int curBias)
    {
        emit printTreeLineSignal(curBias, name);
    }
    virtual std::string toString()
    {
        return name;
    }
    virtual int calc(VarContext* context)
    {
        if (!context->defined(name)) throw int(1);
        return context->getValue(name);
    }
private:
    std::string name;
};
class ComExp: public Expression
{
public:
    ComExp( char ch, Expression* ll, Expression* rr)
    {
        op = ch;
        lson = ll;
        rson = rr;
    }
    virtual expressionType getType()
    {
        return COMPOUND;
    }
    virtual void sendTree(int curBias);
    virtual std::string toString()
    {
        return lson->toString() + op + rson->toString();
    }
    virtual int calc(VarContext* context);
    virtual char getOperator()
    {
        return op;
    }
    virtual Expression* getLson()
    {
        return lson;
    }
    virtual Expression* getRson()
    {
        return rson;
    }
private:
    char op;
    Expression* lson, * rson;
};
//class SpecialExp: public Expression {
//public:
//    SpecialExp(std::string s) {content = s;}
//    virtual expressionType getType() {return SPECIAL;}
//    virtual std::string toString() {return content;}
//    virtual int calc(VarContext& context) {return 0;}
//    std::string content;
//};

#endif // EXPRESSION_H

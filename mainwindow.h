#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <map>
#include <iostream>
#include <statement.h>
#include <string>
#include <varcontext.h>
#include <sstream>
#include <QMessageBox>
#include <vector>
#include <stack>
#include <QObject>
#include <expression.h>
#include <QCoreApplication>
using std::map;
using std::cout;
using std::endl;
using std::string;
using std::stringstream;
using std::vector;
using std::stack;

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    map<int, string> inputStringMap;
    map<int, Statement*> parsedStmMap;
    Ui::MainWindow* ui;
    bool isInputting = 0;
    string inputName = "";
    VarContext context;
    //vector<string> inputStrings;
    stack<Expression*> operantStack;
    stack<char> operatorStack;
    stack<int> operantIndex;
    stack<int> operatorIndex;
    int overallIndex = 0;
    int pc = 0;

    void statementHandler(string inputStm);
    void codeFlush();
    void executeCmdLine(int cmdLineNumber, string cmdLineStr);
    void executeCmdLine(string stmLabel, Statement* stm);
    string strFormalize(string originStr);
    Expression* parser(string str);
    Expression* composeExp(char op);
    void printTreeLine(int bias, string str);
    void printAnsLine(int ans);
    void printTree(QColor color, string str);
    Expression* toLink(char tmp, int now);
    void statementHandler_exe();
    void statementHandler_printTree();
signals:
    void inputFinished();

private slots:
    void on_cmdLineEdit_editingFinished();

private:
};
#endif // MAINWINDOW_H

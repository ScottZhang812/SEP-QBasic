#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDebug>
#include <QFileDialog>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->CodeDisplay->setText("");
    ui->textBrowser->setText("");
    ui->treeDisplay->setText("");
    ui->cmdLineEdit->setText("");
    connect(ui->btnRunCode, &QPushButton::clicked, [ = ] ()
    {
        ui->cmdLineEdit->setText("RUN");
        on_cmdLineEdit_editingFinished();
    });
    connect(ui->btnClearCode, &QPushButton::clicked, [ = ] ()
    {
        ui->cmdLineEdit->setText("CLEAR");
        on_cmdLineEdit_editingFinished();
    });
    connect(ui->btnLoadCode, &QPushButton::clicked, [ = ] ()
    {
        ui->cmdLineEdit->setText("LOAD");
        on_cmdLineEdit_editingFinished();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
//删掉字符串首尾的冗余空格
string MainWindow::strFormalize(string originStr)
{
    int head = 0, tail = originStr.length() - 1;
    while (head <= tail && originStr[head] == ' ') head++;
    while (head <= tail && originStr[tail] == ' ') tail--;
    if (head > tail) return "";
    string ansStr(originStr, head, tail - head + 1);
    return ansStr;
}
//刷新代码框中的代码内容
void MainWindow::codeFlush()
{
    ui->CodeDisplay->setText("");
    for (map<int, string>::iterator it = inputStringMap.begin(); it != inputStringMap.end(); it++)
        ui->CodeDisplay->append(QString::fromStdString(it->second));
}
//执行某行语句
void MainWindow::executeCmdLine(int cmdLineNumber, string cmdLineStr)
{
    int mybin;
    stringstream ss;
    string whichStm = "";
    ss << cmdLineStr;
    ss >> mybin;
    ss >> whichStm;
    Statement* nowParsedStm =  parsedStmMap[cmdLineNumber];
    nowParsedStm->stat++;
    if (whichStm == "REM") {}
    else if (whichStm == "LET")
    {
        context.setValue(nowParsedStm->sons[0]->toString(), nowParsedStm->sons[1]->calc(&context));
    }
    else if (whichStm == "PRINT")
    {
        ui->textBrowser->append(QString::number(nowParsedStm->sons[0]->calc(&context)));
    }
    else if (whichStm == "INPUT")
    {
        ui->cmdLineEdit->setText(" ? ");
        isInputting = 1;
        inputName = nowParsedStm->sons[0]->toString();
        QEventLoop loop;
        connect(this, &MainWindow::inputFinished, &loop, &QEventLoop::quit);
        loop.exec();
    }
    else if (whichStm == "GOTO")
    {
        pc = nowParsedStm->sons[0]->calc(&context);
    }
    else if (whichStm == "IF")
    {
        int leftVal = nowParsedStm->sons[0]->calc(&context), rightVal = nowParsedStm->sons[2]->calc(&context);
        int cmp = leftVal - rightVal;
        string op = nowParsedStm->sons[1]->toString();
        if ((op == "<" && cmp < 0) || (op == "=" && !cmp) || (op == ">" && cmp > 0)) nowParsedStm->Lstat++, pc = nowParsedStm->sons[3]->calc(&context);
        else nowParsedStm->Rstat++;
    }
    else if (whichStm == "END")
    {
        pc = 1147483647;
    }
    else throw int(6);
}
//执行某行语句（重载）
void MainWindow::executeCmdLine(string stmLabel, Statement* stm)
{
    if (stmLabel == "LET")
    {
        context.setValue(stm->sons[0]->toString(), stm->sons[1]->calc(&context));
    }
    else if (stmLabel == "PRINT")
    {
        ui->textBrowser->append(QString::number(stm->sons[0]->calc(&context)));
    }
    else if (stmLabel == "INPUT")
    {
        ui->cmdLineEdit->setText(" ? ");
        isInputting = 1;
        inputName = stm->sons[0]->toString();
        QEventLoop loop;
        connect(this, &MainWindow::inputFinished, &loop, &QEventLoop::quit);
        loop.exec();
    }
    else throw int(6);
}
//Expression* MainWindow::composeExp(char op)
//{
//    Expression *RHS = operantStack.top();operantStack.pop();
//    Expression *LHS = operantStack.top();operantStack.pop();
//    Expression* Exp1 = new ComExp(op, LHS, RHS);
//    connect(Exp1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
//    return Exp1;
//}
//用于表达式parse过程，对于给定的运算符，从操作数栈中出栈两次，组成一个ComExp表达式
Expression* MainWindow::toLink(char tmp, int now)
{
    if (operantStack.empty()) throw int(7);
    Expression* operant1 = operantStack.top();
    operantStack.pop();
    int rIndex = operantIndex.top();
    operantIndex.pop();
    if (operantStack.empty()) throw int(7);
    Expression* operant2 = operantStack.top();
    operantStack.pop();
    int lIndex = operantIndex.top();
    operantIndex.pop();
    if (rIndex <= now || lIndex >= now) throw int(7);
    Expression* Exp1 = new ComExp(tmp, operant2, operant1);
    return Exp1;
}
//对表达式进行parse操作
Expression* MainWindow::parser(string str)
{
    while (!operantStack.empty()) operantStack.pop();
    while (!operatorStack.empty()) operatorStack.pop();
    while (!operantIndex.empty()) operantIndex.pop();
    while (!operatorIndex.empty()) operatorIndex.pop();
    overallIndex = 0;
    if (!str.length())
    {
        Expression* p1 = new IdenExp("");
        connect(p1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
        return p1;
    }
    unsigned int validPos = 0, len = str.length();
    bool case1 = 0;
    while (validPos != len && str[validPos] == ' ') validPos++;
    if (validPos != len && str[validPos] == '-') case1 = 1;
    while (validPos != len)
    {
        char ch = str[validPos];
        while (ch == ' ') ch = str[++validPos];
        int mode = 0;
        if (ch == 'M')
        {
            string strFromNow(str, validPos);
            stringstream ss;
            ss << strFromNow;
            string testStr;
            ss >> testStr;
            if (testStr == "MOD") mode = 1, ch = '%';
            else mode = 3;
        }
        else if (ch >= '0' && ch <= '9') mode = 2;
        else if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_') mode = 3;
        else if (ch == '+' || ch == '-' ) mode = 4;
        else if (ch == '(') mode = 5;
        else if (ch == ')') mode = 6;
        else if (ch == '/') mode = 7;
        else if (ch == '*')
        {
            if (validPos + 1 < len && str[validPos + 1] == '*')
            {
                mode = 8, ch = '^';
            }
            else mode = 7;
        }
        else throw(3);
        if (mode == 1)  // %
        {
            validPos += 3;
            while (!operatorStack.empty())
            {
                char tmp = operatorStack.top();
                if (!(tmp == '*' || tmp == '/' || tmp == '%' || tmp == '^')) break;
                operatorStack.pop();
                int now = operatorIndex.top();
                operatorIndex.pop();
                Expression* Exp1 = toLink(tmp, now);
                connect(Exp1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
                operantStack.push(Exp1);
                operantIndex.push(++overallIndex);
            }
            operatorStack.push(ch);
            operatorIndex.push(++overallIndex);
        }
        else if (mode == 2)  // 0-9
        {
            int tmp = 0;
            while (validPos < len)
            {
                if (str[validPos] >= '0' && str[validPos] <= '9') tmp = tmp * 10 + (str[validPos++] - '0');
                else if ((str[validPos] >= 'A' && str[validPos] <= 'Z') || (str[validPos] >= 'a' && str[validPos] <= 'z') || str[validPos] == '_')
                    throw std::invalid_argument("");
                else break;
            }
            Expression* Exp1 = new ConstExp(tmp);
            connect(Exp1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
            operantStack.push(Exp1);
            operantIndex.push(++overallIndex);
        }
        else if (mode == 3)  // character
        {
            string tmpStr = "";
            while (validPos < len && ( (str[validPos] >= '0' && str[validPos] <= '9') || (str[validPos] >= 'A' && str[validPos] <= 'Z') || (str[validPos] >= 'a' && str[validPos] <= 'z') || str[validPos] == '_' )) tmpStr.push_back(str[validPos++]);
            Expression* Exp1 = new IdenExp(tmpStr);
            if (tmpStr == "REM" || tmpStr == "LET" || tmpStr == "PRINT" || tmpStr == "INPUT" || tmpStr == "GOTO" ||
                    tmpStr == "IF" || tmpStr == "END" || tmpStr == "RUN" || tmpStr == "LOAD" || tmpStr == "LIST" ||
                    tmpStr == "CLEAR" || tmpStr == "HELP" || tmpStr == "QUIT") throw int(3);
            connect(Exp1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
            operantStack.push(Exp1);
            operantIndex.push(++overallIndex);
        }
        else if (mode == 4)  // +-
        {
            validPos += 1;
            if (ch == '-' && (case1 || (!operatorStack.empty() && operatorStack.top() == '(')))
            {
                //处理出现负数的特殊情况
                while (validPos != len && str[validPos] == ' ') validPos++;
                if (validPos < len && str[validPos] >= '0' && str[validPos] <= '9')
                {
                    int tmp = 0;
                    while (validPos < len)
                    {
                        if (str[validPos] >= '0' && str[validPos] <= '9') tmp = tmp * 10 + (str[validPos++] - '0');
                        else if ((str[validPos] >= 'A' && str[validPos] <= 'Z') || (str[validPos] >= 'a' && str[validPos] <= 'z') || str[validPos] == '_')
                            throw std::invalid_argument("");
                        else break;
                    }
                    Expression* Exp1 = new ConstExp(-tmp);
                    connect(Exp1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
                    operantStack.push(Exp1);
                    operantIndex.push(++overallIndex);
                }
                else throw std::invalid_argument("您输入了一个非法的负号");
            }
            else
            {
                while (!operatorStack.empty() && operatorStack.top() != '(')
                {
                    char tmp = operatorStack.top();
                    operatorStack.pop();
                    int now = operatorIndex.top();
                    operatorIndex.pop();
                    Expression* Exp1 = toLink(tmp, now);
                    connect(Exp1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
                    operantStack.push(Exp1);
                    operantIndex.push(++overallIndex);
                }
                operatorStack.push(ch);
                operatorIndex.push(++overallIndex);
            }
        }
        else if (mode == 5) validPos += 1, operatorStack.push('('), operatorIndex.push(++overallIndex); // (
        else if (mode == 6)  // )
        {
            bool leftParenAppeared = 0;
            validPos += 1;
            while (!operatorStack.empty())
            {
                char tmp = operatorStack.top();
                operatorStack.pop();
                int now = operatorIndex.top();
                operatorIndex.pop();
                if (tmp == '(')
                {
                    leftParenAppeared = 1;
                    break;
                }
                else
                {
                    Expression* Exp1 = toLink(tmp, now);
                    connect(Exp1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
                    operantStack.push(Exp1);
                    operantIndex.push(++overallIndex);
                }
            }
            if (!leftParenAppeared) throw int(7);
        }
        else if (mode == 7)  // */
        {
            validPos += 1;
            while (!operatorStack.empty())
            {
                char tmp = operatorStack.top();
                if (!(tmp == '*' || tmp == '/' || tmp == '%' || tmp == '^')) break;
                operatorStack.pop();
                int now = operatorIndex.top();
                operatorIndex.pop();
                Expression* Exp1 = toLink(tmp, now);
                connect(Exp1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
                operantStack.push(Exp1);
                operantIndex.push(++overallIndex);
            }
            operatorStack.push(ch);
            operatorIndex.push(++overallIndex);
        }
        else if (mode == 8 )  // **
        {
            validPos += 2;
            operatorStack.push(ch);
            operatorIndex.push(++overallIndex);
        }
        //delete all the leading blanks
        while (validPos < len && str[validPos] == ' ') validPos++;
    }
    while (!operatorStack.empty())
    {
        char tmp = operatorStack.top();
        operatorStack.pop();
        int now = operatorIndex.top();
        operatorIndex.pop();
        Expression* Exp1 = toLink(tmp, now);
        connect(Exp1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
        operantStack.push(Exp1);
        operantIndex.push(++overallIndex);
    }
    Expression* retExp = operantStack.top();
    operantStack.pop();
    if (!operantStack.empty()) throw int(7);
    return retExp;
}
void MainWindow::statementHandler_exe()
{
    try
    {
        map<int, string>::iterator pcIt = inputStringMap.begin();
        pc = pcIt->first;
        //it = inputStringMap.end(); it--;
        //int pcEnd = it->first + 10;
        while (pcIt != inputStringMap.end())
        {
            map<int, string>::iterator tmpIt = inputStringMap.begin();
            while (tmpIt != inputStringMap.end() && tmpIt->first != pc) tmpIt++;
            if (tmpIt == inputStringMap.end()) throw int(5);
            string tmpStr = tmpIt->second;
            int nowPC = tmpIt->first;
            pcIt = tmpIt;
            pcIt++;
            if (pcIt != inputStringMap.end()) pc = pcIt->first;
            else pc = 1147483647;
            if (parsedStmMap[nowPC] == nullptr)
            {
                QMessageBox::warning(this, "ERROR", "Runtime Error! The program has stopped.");
                break;
            }
            executeCmdLine(nowPC, tmpStr);
            pcIt = inputStringMap.begin();
            while (pcIt != inputStringMap.end() && pcIt->first != pc) pcIt++;
        }
    }
    catch (int xx)
    {
        qDebug() << "catch: " << xx;
        QMessageBox::warning(this, "ERROR", "Runtime Error! The program has stopped.");
    }
    catch (std::invalid_argument)
    {
        qDebug() << "catch: 非法整数或非法参数";
        QMessageBox::warning(this, "ERROR", "Runtime Error! The program has stopped.");
    }
}
void MainWindow::statementHandler_printTree()
{
    for (map<int, Statement * >::iterator it = parsedStmMap.begin(); it != parsedStmMap.end(); it++)
    {
        Statement* tmpStm = it->second;
        if (tmpStm == nullptr)
        {
            printTree(Qt::black, std::to_string(it->first) + " Error\n");
        }
        else if (tmpStm->getType() == IF)
        {
            printTree(Qt::black, std::to_string(tmpStm->lineNum) + " IF THEN ");
            printTree(Qt::red, std::to_string(tmpStm->Lstat) + " " + std::to_string(tmpStm->Rstat));
            tmpStm->printTree();
            printTree(Qt::black, "\n");
        }
        else if (tmpStm->getType() == LET)
        {
            printTree(Qt::black, std::to_string(tmpStm->lineNum) + " LET = ");
            printTree(Qt::red, std::to_string(tmpStm->stat) + "\n");
            printTree(Qt::black, "    " + tmpStm->sons[0]->toString() + " ");
            printTree(Qt::red, std::to_string(context.symbolStat[tmpStm->sons[0]->toString()]));
            tmpStm->printTree();
            printTree(Qt::black, "\n");
        }
        else if (tmpStm->getType() == REM)
        {
            printTree(Qt::black, std::to_string(tmpStm->lineNum) + " REM ");
            printTree(Qt::red, std::to_string(tmpStm->stat));
            tmpStm->printTree();
            printTree(Qt::black, "\n");
        }
        else if (tmpStm->getType() == PRINT)
        {
            printTree(Qt::black, std::to_string(tmpStm->lineNum) + " PRINT ");
            printTree(Qt::red, std::to_string(tmpStm->stat));
            tmpStm->printTree();
            printTree(Qt::black, "\n");
        }
        else if (tmpStm->getType() == INPUT)
        {
            printTree(Qt::black, std::to_string(tmpStm->lineNum) + " INPUT ");
            printTree(Qt::red, std::to_string(tmpStm->stat));
            tmpStm->printTree();
            printTree(Qt::black, "\n");
        }
        else if (tmpStm->getType() == GOTO)
        {
            printTree(Qt::black, std::to_string(tmpStm->lineNum) + " GOTO ");
            printTree(Qt::red, std::to_string(tmpStm->stat));
            tmpStm->printTree();
            printTree(Qt::black, "\n");
        }
        else if (tmpStm->getType() == END)
        {
            printTree(Qt::black, std::to_string(tmpStm->lineNum) + " END ");
            printTree(Qt::red, std::to_string(tmpStm->stat));
            tmpStm->printTree();
            printTree(Qt::black, "\n");
        }
    }
}
//在用户输入完一行语句后对语句进行初步处理，在总体上控制parse/execute/输出syntax tree等操作
void MainWindow::statementHandler(string ii)
{
    string inputStm = strFormalize(ii);
    if (!inputStm.length()) return;
    if (inputStm == "RUN")
    {
        ui->textBrowser->setText("");
        ui->treeDisplay->setText("");
        if (!inputStringMap.empty())
        {
            //parsing all
            for (map<int, string>::iterator it = inputStringMap.begin(); it != inputStringMap.end(); it++)
            {
                int lineNumber = it->first;
                try
                {
                    stringstream ss;
                    string cmdLineStr = it->second, tmpStr;
                    ss << cmdLineStr;
                    ss >> tmpStr;
                    ss >> tmpStr;
                    if (tmpStr == "REM")
                    {
                        string STR;
                        getline(ss, STR, '\n');
                        STR = strFormalize(STR);
                        Expression* Exp1 = new IdenExp(STR);
                        connect(Exp1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
                        Statement* tmpStm = new RemStm(&context, lineNumber, Exp1);
                        connect(tmpStm, &Statement::printTreeLineSignal, this, &MainWindow::printTreeLine);
                        parsedStmMap[lineNumber] = tmpStm;
                    }
                    else if (tmpStr == "LET")
                    {
                        string LHSSTR, RHSSTR;
                        getline(ss, LHSSTR, '=');
                        getline(ss, RHSSTR, '\n');
                        LHSSTR = strFormalize(LHSSTR);
                        RHSSTR = strFormalize(RHSSTR);
                        if (LHSSTR == "" || RHSSTR == "") throw int(7);
                        Expression* leftIdenExp = parser(LHSSTR);
                        if (leftIdenExp->getType() != IDENTIFIER) throw int(3);
                        connect(leftIdenExp, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
                        Expression* rightValExp = parser(RHSSTR);
                        Statement* tmpStm = new LetStm(&context, lineNumber, leftIdenExp, rightValExp);
                        connect(tmpStm, &Statement::printTreeLineSignal, this, &MainWindow::printTreeLine);
                        parsedStmMap[lineNumber] = tmpStm;
                    }
                    else if (tmpStr == "PRINT")
                    {
                        string STR;
                        getline(ss, STR, '\n');
                        STR = strFormalize(STR);
                        Expression* Exp1 = parser(STR);
                        Statement* tmpStm = new PrintStm(&context, lineNumber, Exp1);
                        connect(tmpStm, &Statement::printTreeLineSignal, this, &MainWindow::printTreeLine);
                        connect(tmpStm, &Statement::printAnsLineSignal, this, &MainWindow::printAnsLine);
                        parsedStmMap[lineNumber] = tmpStm;
                    }
                    else if (tmpStr == "INPUT")
                    {
                        string STR;
                        getline(ss, STR, '\n');
                        STR = strFormalize(STR);
                        Expression* exp1 = parser(STR);
                        if (exp1->getType() != IDENTIFIER) throw int(3);
                        connect(exp1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
                        Statement* tmpStm = new InputStm(&context, lineNumber, exp1);
                        connect(tmpStm, &Statement::printTreeLineSignal, this, &MainWindow::printTreeLine);
                        parsedStmMap[lineNumber] = tmpStm;
                    }
                    else if (tmpStr == "GOTO")
                    {
                        string STR;
                        getline(ss, STR, '\n');
                        STR = strFormalize(STR);
                        Expression* Exp1 = new ConstExp(stoi(STR));
                        connect(Exp1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
                        Statement* tmpStm = new GotoStm(&context, lineNumber, Exp1);
                        connect(tmpStm, &Statement::printTreeLineSignal, this, &MainWindow::printTreeLine);
                        parsedStmMap[lineNumber] = tmpStm;
                    }
                    else if (tmpStr == "IF")
                    {
                        char ch;
                        ss >> std::noskipws >> ch;
                        string EXP1 = "", EXP2 = "", OP, thenStr = "", EXP3;
                        while (ss.rdbuf()->in_avail() && ch != '<' && ch != '=' && ch != '>')
                        {
                            EXP1.push_back(ch);
                            ss >> std::noskipws  >> ch;
                        }
                        if (ch != '<' && ch != '=' && ch != '>') throw int (3);
                        OP = ch;
                        ss >> std::skipws >> EXP2;
                        ss >> thenStr;
                        if (thenStr != "THEN") throw int (3);
                        ss >> EXP3;
                        EXP1 = strFormalize(EXP1);
                        EXP2 = strFormalize(EXP2);
                        OP = strFormalize(OP);
                        EXP3 = strFormalize(EXP3);
                        if (EXP1 == "" || EXP2 == "" || EXP3 == "") throw int(3);
                        Expression* leftExp = parser(EXP1), * opExp = new IdenExp(OP),
                        * rightExp = parser(EXP2), * targetExp = new ConstExp(stoi(EXP3));
                        connect(opExp, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
                        connect(targetExp, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
                        Statement* tmpStm = new IfStm(&context, lineNumber, leftExp, opExp, rightExp, targetExp);
                        connect(tmpStm, &Statement::printTreeLineSignal, this, &MainWindow::printTreeLine);
                        parsedStmMap[lineNumber] = tmpStm;
                    }
                    else if (tmpStr == "END")
                    {
                        char afterEnd = ' ';
                        while (ss.rdbuf()->in_avail() && afterEnd == ' ')
                        {
                            ss  >> afterEnd;
                        }
                        if (afterEnd != ' ') throw int(3);
                        Statement* tmpStm = new EndStm(&context, lineNumber);
                        connect(tmpStm, &Statement::printTreeLineSignal, this, &MainWindow::printTreeLine);
                        parsedStmMap[lineNumber] = tmpStm;
                    }
                    else throw int(6);

                }
                catch (int xx)
                {
                    qDebug() << "catch: " << xx;
                    parsedStmMap[lineNumber] = nullptr;
                }
                catch (std::invalid_argument)
                {
                    qDebug() << "catch: 非法整数或非法参数";
                    parsedStmMap[lineNumber] = nullptr;
                }
            }
            //executing
            statementHandler_exe();
            //outputing the syntax tree with stats
            statementHandler_printTree();
            qDebug() << "RUN ended normally.";
        }
    }
    else if (inputStm == "LOAD")
    {
        QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), "", tr("文本文件 (*.txt);;所有文件 (*.*)"));
        if (!filePath.isEmpty())
        {
            // 读取文件内容
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QTextStream in(&file);
                while (!in.atEnd())
                {
                    QString line = in.readLine();
                    //qDebug()<<"read 1 line successfully";
                    qDebug() << line;
                    ui->cmdLineEdit->setText(line);
                    on_cmdLineEdit_editingFinished();
                }
                file.close();
            }
        }
        ui->cmdLineEdit->setText("RUN");
        on_cmdLineEdit_editingFinished();
    }
    else if (inputStm == "LIST")
    {
        //don't need to do anything
    }
    else if (inputStm == "CLEAR")
    {
        context.symbolMap.clear();
        inputStringMap.clear();
        parsedStmMap.clear();
        ui->CodeDisplay->setText("");
        ui->textBrowser->setText("");
        ui->treeDisplay->setText("");
    }
    else if (inputStm == "HELP")
    {
        QMessageBox::information(this, "Help Message", "The minimal BASIC interpreter implements only six statement forms, which appear in Table 1. The LET, PRINT, and INPUT statements can be executed directly by typing them without a line number, in which case they are evaluated immediately. In addition to the statements listed in Table 1, BASIC accepts the commands shown in Table 2. These commands cannot be part of a program and must therefore be entered without a line number.");
    }
    else if (inputStm == "QUIT")
    {
        QCoreApplication::quit();
    }
    else
    {
        //inputStrings.push_back(inputStm);
        stringstream ss;
        ss << inputStm;
        string tmpStr;
        getline(ss, tmpStr, ' ');
        char tmpChar = tmpStr[0];
        if (tmpChar >= '0' && tmpChar <= '9')
        {
            int tmpInt = stoi(tmpStr);
            if (tmpInt <= 0 || tmpInt > 1000000)
            {
                QMessageBox::information(this, "行号错误", "输入的行号需要为不超过1000000的正整数，请重新输入！");
                return;
            }
            string testStr;
            ss >> testStr;
            if (testStr == "") inputStringMap.erase(tmpInt);
            else inputStringMap[tmpInt] = inputStm;
            codeFlush();
        }
        else
        {
            //处理无行号的第I类语句
            if (tmpStr == "LET")
            {
                string LHSSTR, RHSSTR;
                getline(ss, LHSSTR, '=');
                getline(ss, RHSSTR, '\n');
                LHSSTR = strFormalize(LHSSTR);
                RHSSTR = strFormalize(RHSSTR);
                Expression* leftIdenExp = new IdenExp(LHSSTR);
                //connect(leftIdenExp, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
                Expression* rightValExp = parser(RHSSTR);
                Statement* tmpStm = new LetStm(&context, 0, leftIdenExp, rightValExp);
                //connect(tmpStm, &Statement::printTreeLineSignal, this, &MainWindow::printTreeLine);
                executeCmdLine(tmpStr, tmpStm);
            }
            else if (tmpStr == "PRINT")
            {
                string STR;
                getline(ss, STR, '\n');
                STR = strFormalize(STR);
                Expression* Exp1 = parser(STR);
                //connect(Exp1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
                Statement* tmpStm = new PrintStm(&context, 0, Exp1);
                //connect(tmpStm, &Statement::printTreeLineSignal, this, &MainWindow::printTreeLine);
                connect(tmpStm, &Statement::printAnsLineSignal, this, &MainWindow::printAnsLine);
                executeCmdLine(tmpStr, tmpStm);
            }
            else if (tmpStr == "INPUT")
            {
                string STR;
                getline(ss, STR, '\n');
                STR = strFormalize(STR);
                Expression* exp1 = new IdenExp(STR);
                //connect(exp1, &Expression::printTreeLineSignal, this, &MainWindow::printTreeLine);
                Statement* tmpStm = new InputStm(&context, 0, exp1);
                //connect(tmpStm, &Statement::printTreeLineSignal, this, &MainWindow::printTreeLine);
                executeCmdLine(tmpStr, tmpStm);
            }
            else
            {
                QMessageBox::warning(this, "ERROR", "Runtime Error! The program has stopped.");
            }
        }
    }
}
void MainWindow::on_cmdLineEdit_editingFinished()
{
    try
    {
        QString cmd = ui->cmdLineEdit->text();
        ui->cmdLineEdit->setText("");
        string inputStr = cmd.toStdString();
        inputStr = strFormalize(inputStr);
        if (!isInputting)
        {
            //ui->CodeDisplay->append(cmd);
            statementHandler(inputStr);
        }
        else
        {
            isInputting = 0;
            string numStr(inputStr, 2);
            //qDebug()<<QString::fromStdString(numStr);
            int tmp = stoi(numStr);
            context.setValue(inputName, tmp);
            emit inputFinished();
        }
    }
    catch (int xx)
    {
        qDebug() << "catch: " << xx;
    }
    catch (std::invalid_argument)
    {
        qDebug() << "catch: 非法整数或非法参数";
    }

}
//用于在syntax tree框中打印内容
void MainWindow::printTree(QColor color, string str)
{
    QTextCharFormat charFormat;
    charFormat.setForeground(color);
    QTextCursor cursor(ui->treeDisplay->textCursor());
    cursor.movePosition(QTextCursor::End);
    cursor.setCharFormat(charFormat);
    cursor.insertText(QString::fromStdString(str));
    charFormat.setForeground(Qt::black);
    cursor.setCharFormat(charFormat);
}
//用于在syntax tree框中打印一整行内容
void MainWindow::printTreeLine(int bias, string str)
{
    string tmpStr = "";
    for (int i = 1; i <= bias; i++) tmpStr.push_back(' ');
    tmpStr = tmpStr + str;
    printTree(Qt::black, "\n" + tmpStr);
    //ui->treeDisplay->append(QString::fromStdString(tmpStr));
}
//用于在程序运行结果框中打印一整行内容
void MainWindow::printAnsLine(int ans)
{
    ui->textBrowser->append(QString::number(ans));
}

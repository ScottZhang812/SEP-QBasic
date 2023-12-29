#include "expression.h"

void ComExp::sendTree(int curBias)
{
    string opStr(1, op);
    if (opStr == "%") opStr = "MOD";
    if (opStr == "^") opStr = "**";
    emit printTreeLineSignal(curBias, opStr);
    if (lson) lson->sendTree(curBias + 4);
    if (rson) rson->sendTree(curBias + 4);
}

int ComExp::calc(VarContext* context)
{
    int rightAns = rson->calc(context);
    if (op == '=')
    {
        context->setValue(lson->getIdentifier(), rightAns);
        return rightAns;
    }
    int leftAns = lson->calc(context);
    if (op == '+') return leftAns + rightAns;
    if (op == '-') return leftAns - rightAns;
    if (op == '*') return leftAns * rightAns;
    if (op == '/')
    {
        if (rightAns == 0) throw int(2);
        return leftAns / rightAns;
    }
    if (op == '%')
    {
        int tmp = leftAns % rightAns;
        int bias = abs(rightAns);
        if (tmp > 0 && rightAns < 0) tmp -= bias;
        else if (tmp < 0 && rightAns > 0) tmp += bias;
        return tmp;
    }
    if (op == '^')
    {
        if (rightAns < 0) return 0;
        int ans = 1;
        for (int i = 1; i <= rightAns; i++)
            ans *= leftAns;
        return ans;
    }
    throw int (3);
    return 0;
}

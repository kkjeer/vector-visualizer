#pragma once
#include "Expr.h"
#include <string>
#include <map>
#include <vector>
#include <set>
#include <QTextBrowser>

using namespace Expression;
using namespace std;

class Parser {
enum Error {
  None = 0,
  EmptyString,
  UnbalancedParens,
  IllegalCharacter,
  UnknownOperator,
  NullLeftChild,
  NullRightChild,
  BadLeafNode,
  MultipleDots,
  Max
};

public:
  Parser(QTextBrowser *browser) : debug(browser), error(Error::None) {
    error = Error::None;
    badIndex = -1;
    nullChildOpIndex = -1;
  }
  Expr *Parse(string str);
  string GetErrorMessage(string str);
  bool HasError();

private:
  QTextBrowser *debug;
  Error error;
  size_t badIndex;
  size_t nullChildOpIndex;

  void Print(string s);
  void Debug(string s);
  map<size_t, float> GetOperators(string str);
  Expr *GetExpr(string str, vector<size_t> sortedIndices, map<size_t, float> operators, set<size_t> &processedOps, size_t index);
  Expr *OperatorExpr(char op, size_t index, Expr *left, Expr *right);
  Expr *LeafNodeLeftChild(string str, size_t index);
  Expr *LeafNodeRightChild(string str, size_t index);
  Expr *LeftDigit(string str, size_t index);
  Expr *RightDigit(string str, size_t index);
  Expr *Digit(string str);
  bool IsDigit(char &c);
};
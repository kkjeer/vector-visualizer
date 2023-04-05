#pragma once
#include "Expr.h"
#include "Parsing/Lexer.h"
#include <string>
#include <map>
#include <vector>
#include <set>
#include <QTextBrowser>

using namespace Expression;
using namespace std;

class ParserAlt {
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
  ParserAlt(LexMode mode, QTextBrowser *browser = nullptr) : mode(mode), debug(browser) {}
  Expr *Parse(string str);

private:
  LexMode mode;
  QTextBrowser *debug;
  vector<Lexeme> lexemes;

  void Print(string s);
  void Debug(string s);
  void Error(string s);

  bool CheckBalancedParens();
  Expr *ParseRange(size_t start, size_t end);
  map<size_t, Expr *> GetLeafExprs(size_t start, size_t end, map<size_t, size_t> &closeParens, map<size_t, size_t> &negationChildren);
  Expr *ParseNegation(size_t &exprIndex, size_t end, map<size_t, Expr *> &exprs, map<size_t, size_t> &closeParens, map<size_t, size_t> &negationChildren);
  Expr *ParseTrigOrNegation(size_t &exprIndex, size_t end, map<size_t, Expr *> &exprs, map<size_t, size_t> &closeParens);
  size_t CloseParenIndex(size_t start, size_t end, bool checkTrig, bool &gotIndex);
  map<size_t, float> GetOperators(size_t start, size_t end, map<size_t, size_t> closeParens);
  Expr *LeftChild(size_t index, size_t start, map<size_t, Expr *> &finalExprs);
  Expr *RightChild(size_t index, size_t end, map<size_t, Expr *> &finalExprs);
};

#pragma once
#include <string>
#include <vector>
#include <QTextBrowser>

using namespace std;

enum LexMode {
  NoVariables,
  SingleVariable,
  MultiVariable
};

enum Lex {
  Error,
  Space,
  TVar,
  XVar,
  YVar,
  ZVar,
  Number,
  OpenParen,
  CloseParen,
  Plus,
  Minus,
  Times,
  Divide,
  Power,
  Negative,
  SinFunc,
  CosFunc,
};

struct Lexeme {
  Lex Kind;
  size_t Loc;
  float Val;
  int Precision;
};

class Lexer {
public:
  Lexer(LexMode mode, QTextBrowser *browser) : mode(mode), debug(browser) {}
  vector<Lexeme> Lex(string str);
  static string ToString(string str, vector<Lexeme> lexemes);
  static string LexemeString(Lexeme L);
  static string LexemeStr(Lexeme L);

private:
  LexMode mode;
  QTextBrowser *debug = nullptr;
  void Print(string message);
  void Debug(string message);
  Lexeme GetLexeme(string str, size_t &index);
  bool IsDigit(char &c);
};

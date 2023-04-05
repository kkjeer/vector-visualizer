#include "Parsing/Lexer.h"
#include "Expr.h"

void Lexer::Print(string str) {
  if (debug)
    debug->append(QString(str.c_str()));
}

void Lexer::Debug(string str) {
  if (true) Print(str);
}

vector<Lexeme> Lexer::Lex(string str) {
  vector<Lexeme> Result;

  size_t i = 0;
  while (i < str.length()) {
    Lexeme L = GetLexeme(str, i);
    if (L.Kind == Lex::Error) {
      Print("~~~~~~ ERROR: encountered error in lexer");
      return Result;
    }
    if (L.Kind != Lex::Space) {
      Result.push_back(L);
    }
  }

  auto IsNotOp = [](Lexeme L) {
    switch (L.Kind) {
      case Lex::TVar:
      case Lex::XVar:
      case Lex::YVar:
      case Lex::ZVar:
      case Lex::Number:
        return true;
      default:
        return false;
    }
    return false;
  };

  auto IsFirst = [](Lexeme L) {
    switch (L.Kind) {
      case Lex::TVar:
      case Lex::XVar:
      case Lex::YVar:
      case Lex::ZVar:
      case Lex::Number:
        return true;
      default:
        return false;
    }
    return false;
  };

  auto IsSecond = [](Lexeme L) {
    switch (L.Kind) {
      case Lex::TVar:
      case Lex::XVar:
      case Lex::YVar:
      case Lex::ZVar:
      case Lex::SinFunc:
      case Lex::CosFunc:
        return true;
      default:
        return false;
    }
    return false;
  };

  for (size_t idx = 0; idx < Result.size() - 1; ++idx) {
    Lexeme Curr = Result.at(idx);
    Lexeme Next = Result.at(idx + 1);
    if (IsFirst(Curr) && IsSecond(Next)) {
      size_t Loc = Next.Loc;
      Print("Inserting * at index " + to_string(idx + 1) + " and loc " + to_string(Loc));
      Lexeme Mult = {Lex::Times, Loc};
      Result.insert(Result.begin() + idx + 1, Mult);
      for (size_t j = idx + 2; j < Result.size(); ++j) {
        Result.at(j).Loc++;
      }
    }
  }

  return Result;
}

Lexeme Lexer::GetLexeme(string str, size_t &index) {
  char &c = str.at(index);
  Debug("Current character is: \"" + string(1, c) + "\"");
  if (IsDigit(c)) {
    Debug("Current character is a digit");
    if (c == '.') {
      Print("~~~~~~ ERROR: dot found but not in a numerical value");
      return {Lex::Error};
    }

    string NumberStr = "";
    int Precision = 0;
    bool foundDot = false;
    char &next = c;
    size_t loc = index;
    while (IsDigit(next) && index < str.length()) {
      NumberStr += string(1, next);
      if (next == '.')
        foundDot = true;
      if (next != '.' && foundDot)
        ++Precision;
      ++index;
      if (index < str.length())
        next = str.at(index);
    }
    float Val = stof(NumberStr);
    Debug("NumberStr: " + NumberStr);
    return {Lex::Number, loc, Val, Precision};
  }

  switch (c) {
    case 't': {
      if (mode != LexMode::SingleVariable) {
        Print("~~~ t is only allowed in single-variable functions");
        return {Lex::Error};
      }
      Lexeme L = {Lex::TVar, index};
      ++index;
      return L;
    }
    case 'x': {
      if (mode != LexMode::MultiVariable) {
        Print("~~~~ x is only allowed in multi-variable functions");
        return {Lex::Error};
      }
      Lexeme L = {Lex::XVar, index};
      ++index;
      return L;
    }
    case 'y': {
      if (mode != LexMode::MultiVariable) {
        return {Lex::Error};
      }
      Lexeme L = {Lex::YVar, index};
      ++index;
      return L;
    }
    case 'z': {
      if (mode != LexMode::MultiVariable) {
        return {Lex::Error};
      }
      Lexeme L = {Lex::ZVar, index};
      ++index;
      return L;
    }
    case '+': {
      Lexeme L = {Lex::Plus, index};
      ++index;
      return L;
    }
    case '-': {
      if (index < str.length() - 1) {
        if (str.at(index + 1) == ' ') {
          Lexeme L = {Lex::Minus, index};
          ++index;
          return L;
        }
      }
      Lexeme L = {Lex::Negative, index};
      ++index;
      return L;
    }
    case '*': {
      Lexeme L = {Lex::Times, index};
      ++index;
      return L;
    }
    case '/': {
      Lexeme L = {Lex::Divide, index};
      ++index;
      return L;
    }
    case '^': {
      Lexeme L = {Lex::Power, index};
      ++index;
      return L;
    }
    case '(': {
      Lexeme L = {Lex::OpenParen, index};
      ++index;
      return L;
    }
    case ')': {
      Lexeme L = {Lex::CloseParen, index};
      ++index;
      return L;
    }
    case 's': {
      if (str.length() <= 3) {
        Print("~~~~~~ ERROR: ill-formed sin");
        return {Lex::Error};
      }
      if (index < str.length() - 2) {
        if (str.at(index + 1) == 'i' && str.at(index + 2) == 'n') {
          Lexeme L = {Lex::SinFunc, index};
          index += 3;
          return L;
        }
      }
      Print("~~~~~~ ERROR: ill-formed sin");
      return {Lex::Error};
    }
    case 'c': {
      if (str.length() <= 3) {
        Print("~~~~~~ ERROR: ill-formed cos");
        return {Lex::Error};
      }
      if (index < str.length() - 2) {
        if (str.at(index + 1) == 'o' && str.at(index + 2) == 's') {
          Lexeme L = {Lex::CosFunc, index};
          index += 3;
          return L;
        }
      }
      Print("~~~~~~ ERROR: ill-formed cos");
      return {Lex::Error};
    }
    case ' ': {
      Lexeme L = {Lex::Space};
      ++index;
      return L;
    }
    default: {
      ++index;
      return {Lex::Error};
    }
  }
  ++index;
  return {Lex::Error};
}

bool Lexer::IsDigit(char &c) {
  switch (c) {
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '0':
    case '.': {
      return true;
    }
    default: {
      return false;
    }
  }
  return false;
}

string Lexer::ToString(string str, vector<Lexeme> lexemes) {
  string message = "$$$ Lexemes for " + str + ": $$$\n{ ";
  size_t msgIndex = 0;
  for (auto I = lexemes.begin(); I != lexemes.end(); ++I) {
    ++msgIndex;
    Lexeme L = *I;
    message += Lexer::LexemeString(L);
    if (msgIndex < lexemes.size())
      message += " ";
  }
  message += " }";
  return message;
}

string Lexer::LexemeString(Lexeme L) {
  return "[" + LexemeStr(L) + ", " + to_string(L.Loc) + "]";
}

string Lexer::LexemeStr(Lexeme L) {
  string Result = "";
  switch (L.Kind) {
    case Lex::XVar: return "x";
    case Lex::YVar: return "y";
    case Lex::ZVar: return "z";
    case Lex::TVar: return "t";
    case Lex::Number: {
      return Expression::Precision(L.Val, L.Precision);
    }
    case Lex::OpenParen: return "(";
    case Lex::CloseParen: return ")";
    case Lex::Plus: return "+";
    case Lex::Minus: return "-";
    case Lex::Times: return "*";
    case Lex::Divide: return "/";
    case Lex::Power: return "^";
    case Lex::Negative: return "~";
    case Lex::SinFunc: return "sin";
    case Lex::CosFunc: return "cos";
    case Lex::Error: return "ERROR";
    default: return "Unknown_Lexeme";
  }
  return Result;
}
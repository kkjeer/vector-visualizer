#include "Parsing/ParserAlt.h"

void ParserAlt::Print(string str) {
  if (!debug) return;
  debug->append(QString(str.c_str()));
}

void ParserAlt::Debug(string str) {
  if (false) Print(str);
}

void ParserAlt::Error(string str) {
  Print("~~~~~~ ERROR: " + str);
}

Expr *ParserAlt::Parse(string str) {
  Print("Parsing (alt) string " + str);
  if (str.length() < 1) {
    Error("cannot parse an empty string");
    return nullptr;
  }

  Lexer *lexer = new Lexer(mode, debug);
  lexemes = lexer->Lex(str);

  Print(Lexer::ToString(str, lexemes));

  if (!CheckBalancedParens()) {
    return nullptr;
  }

  Expr *Result = ParseRange(0, lexemes.size());
  return Result;
}

bool ParserAlt::CheckBalancedParens() {
  int parenLevel = 0;
  for (size_t index = 0; index < lexemes.size(); ++index) {
    Lexeme L = lexemes.at(index);
    if (L.Kind == Lex::OpenParen) {
      ++parenLevel;
    } else if (L.Kind == Lex::CloseParen) {
      if (parenLevel <= 0) {
        Error("parenLevel is not positive while processing a close paren - we haven't seen enough open parens");
        return false;
      }
      --parenLevel;
    }
  }
  if (parenLevel != 0) {
    Error("parenLevel " + to_string(parenLevel) + " is nonzero at the end of processing all lexemes, so parens are not balanced");
    return false;
  }
  return true;
}

Expr *ParserAlt::ParseRange(size_t start, size_t end) {
  Print("\n%%% Parsing from start = " + to_string(start) + " to end = " + to_string(end) + " %%%");
  map<size_t, size_t> closeParens;
  map<size_t, size_t> negationChildren;
  map<size_t, Expr *> leafExprs = GetLeafExprs(start, end, closeParens, negationChildren);

  string exprsMsg = "@@@ Leaf exprs: { ";
  size_t msgIndex = 0;
  for (const auto &Pair : leafExprs) {
    ++msgIndex;
    exprsMsg += to_string(Pair.first) + " => " + Pair.second->ToString();
    if (msgIndex < leafExprs.size()) exprsMsg += ", ";
  }
  exprsMsg += " }";
  Print(exprsMsg);

  string closeParensMsg = "@@@ Close paren indices: { ";
  msgIndex = 0;
  for (const auto &Pair : closeParens) {
    ++msgIndex;
    closeParensMsg += to_string(Pair.first) + " => " + to_string(Pair.second);
    if (msgIndex < closeParens.size()) closeParensMsg += ", ";
  }
  closeParensMsg += " }";
  Print(closeParensMsg);

  // If there are no leaf expressions, this is an error.
  if (leafExprs.size() == 0) {
    Error("there are no leaf expressions");
    return nullptr;
  }

  // Erase any children of negation expressions from leafExprs. These should
  // no longer be considered as leaf nodes since their parent negation expression
  // is a leaf node.
  for (const auto &Pair : negationChildren) {
    size_t negationIndex = Pair.first;
    size_t killedIndex = Pair.second;
    auto It = leafExprs.find(killedIndex);
    if (It != leafExprs.end()) {
      Print("Erasing leaf expr " + It->second->ToString() + " as child of negation index " + to_string(negationIndex));
      leafExprs.erase(It);
    }
  }

  exprsMsg = "@@@ Leaf exprs (after erasing children of negations): { ";
  msgIndex = 0;
  for (const auto &Pair : leafExprs) {
    ++msgIndex;
    exprsMsg += to_string(Pair.first) + " => " + Pair.second->ToString();
    if (msgIndex < leafExprs.size()) exprsMsg += ", ";
  }
  exprsMsg += " }";
  Print(exprsMsg);

  // If there is only one expression in leafExprs, then the entire expression
  // from start to end is a leaf expression (there are no operators).
  // Note: a negative expression such as -1, -sin(x + 1), -(y * z), etc. is
  // considered a leaf expression.
  if (leafExprs.size() == 1) {
    Print("@@@ Expr is a leaf expression");
    auto I = leafExprs.begin();
    Expr *FinalResult = I->second;
    if (!FinalResult) {
      Error("leaf expression is null");
      return nullptr;
    }
    Print("%%% End of parsing from start = " + to_string(start) + " to end = " + to_string(end) + ": leaf expression: " + FinalResult->ToString());
    Print("");
    return FinalResult;
  }

  // If there are only two expressions in leafExprs and one of them is a
  // negation operator, then the entire expression from start to end is
  // a leaf expression (there are no operators).
  // if (leafExprs.size() == 2) {
  //   for (const auto &Pair : leafExprs) {
  //     size_t index = Pair.first;
  //     Lexeme L = lexemes.at(index);
  //     if (L.Kind == Lex::Negative) {
  //       Expr *NegExpr = Pair.second;
  //       if (!NegExpr) {
  //         Error("negation expression is null");
  //         return nullptr;
  //       }
  //       Print("@@@ Expr is a negation expression " + NegExpr->ToString());
  //       return NegExpr;
  //     }
  //   }
  // }

  // // Erase any children of negation expressions from leafExprs. These should
  // // no longer be considered as leaf nodes since their parent negation expression
  // // is a leaf node.
  // for (const auto &Pair : negationChildren) {
  //   size_t negationIndex = Pair.first;
  //   size_t killedIndex = Pair.second;
  //   auto It = leafExprs.find(killedIndex);
  //   if (It != leafExprs.end()) {
  //     Print("Erasing leaf expr " + It->second->ToString() + " as child of negation index " + to_string(negationIndex));
  //     leafExprs.erase(It);
  //   }
  // }

  // // Check again for a single leaf expression after erasing children of negation exprs.
  // if (leafExprs.size() == 1) {
  //   Print("@@@ Expr is a leaf expression");
  //   auto I = leafExprs.begin();
  //   Expr *FinalResult = I->second;
  //   if (!FinalResult) {
  //     Error("leaf expression is null");
  //     return nullptr;
  //   }
  //   Print("%%% End of parsing from start = " + to_string(start) + " to end = " + to_string(end) + ": leaf expression: " + FinalResult->ToString());
  //   Print("");
  //   return FinalResult;
  // }

  // Map each operator to its precedence.
  map<size_t, float> operators = GetOperators(start, end, closeParens);
  string opsMsg = "@@@ Operator precedences: { ";
  msgIndex = 0;
  for (const auto &Pair : operators) {
    ++msgIndex;
    opsMsg += Lexer::LexemeString(lexemes.at(Pair.first)) + " => " + Expression::Precision(Pair.second, 1);
    if (msgIndex < operators.size()) opsMsg += ", ";
  }
  opsMsg += " }";
  Print(opsMsg);

  // Sort the operators in decreasing order of precedence.
  // If operator A has higher precedence than operator B, then the expression
  // for A should be created before the expression for B. The expression for A
  // will be closer to the leaf nodes of the final expression (B will be closer
  // to the root).
  vector<size_t> sortedIndices;
  for (const auto &Pair : operators) {
    sortedIndices.push_back(Pair.first);
  }
  sort(sortedIndices.begin(), sortedIndices.end(), [operators](size_t A, size_t B) {
    float precedenceA = operators.at(A);
    float precedenceB = operators.at(B);
    // TODO: figure out associativity issues
    if (precedenceA == precedenceB) return A > B;
    return precedenceA > precedenceB;
  });

  // Print the sorted operators for debugging.
  string d = "@@@ Sorted operators: { ";
  for (auto I = sortedIndices.begin(); I != sortedIndices.end(); ++I) {
    size_t index = *I;
    Lexeme L = lexemes.at(index);
    float precedence = operators[index];
    d += Precision(precedence, 1) + " => " + Lexer::LexemeString(L) + ", ";
  }
  d += "}";
  Print(d);

  // Set up the final map of lexeme to expression.
  map<size_t, Expr *> finalExprs;
  for (const auto &Pair : leafExprs) {
    finalExprs[Pair.first] = Pair.second;
  }

  Expr *FinalResult = nullptr;
  for (auto I = sortedIndices.begin(); I != sortedIndices.end(); ++I) {
    size_t index = *I;
    Lexeme L = lexemes.at(index);
    string LStr = Lexer::LexemeString(L);
    float precedence = operators[index];
    Print("  Processing operator " + LStr + " at index " + to_string(index) + " with precedence " + Precision(precedence, 1));
    Expr *left = LeftChild(index, start, finalExprs);
    if (!left) {
      Error("left child of operator " + LStr + " is null");
      return nullptr;
    }
    Print("&nbsp;&nbsp;&nbsp;&nbsp;left child: <span>" + left->ToString() + "</span>");
    Expr *right = RightChild(index, end, finalExprs);
    if (!right) {
      Error("right child of operator " + LStr + " is null");
      return nullptr;
    }
    Print("&nbsp;&nbsp;&nbsp;&nbsp;right child: <span>" + right->ToString() + "</span>");

    switch(L.Kind) {
      case Lex::Plus: {
        finalExprs[index] = new Add(left, right);
        break;
      }
      case Lex::Minus: {
        finalExprs[index] = new Sub(left, right);
        break;
      }
      case Lex::Times: {
        finalExprs[index] = new Mult(left, right);
        break;
      }
      case Lex::Divide: {
        finalExprs[index] = new Div(left, right);
        break;
      }
      case Lex::Power: {
        finalExprs[index] = new Pow(left, right);
        break;
      }
      default: {
        Error("unexpected operator " + LStr);
        return nullptr;
      }
    }
    FinalResult = finalExprs[index];
  }
  if (!FinalResult) {
    Error("end of parsing from start = " + to_string(start) + " to end = " + to_string(end) + ": FinalResult is null");
    return nullptr;
  }

  Print("%%% End of parsing from start = " + to_string(start) + " to end = " + to_string(end) + ": FinalResult is " + FinalResult->ToString());
  Print("");
  return FinalResult;
}

map<size_t, Expr *> ParserAlt::GetLeafExprs(size_t start, size_t end, map<size_t, size_t> &closeParens, map<size_t, size_t> &negationChildren) {
  map<size_t, Expr *> exprs;
  size_t exprIndex = start;
  while (exprIndex < end) {
    Lexeme L = lexemes.at(exprIndex);
    switch (L.Kind) {
      case Lex::TVar: {
        exprs[exprIndex] = new T();
        ++exprIndex;
        break;
      }
      case Lex::XVar: {
        exprs[exprIndex] = new X();
        ++exprIndex;
        break;
      }
      case Lex::YVar: {
        exprs[exprIndex] = new Y();
        ++exprIndex;
        break;
      }
      case Lex::ZVar: {
        exprs[exprIndex] = new Z();
        ++exprIndex;
        break;
      }
      case Lex::Number: {
        exprs[exprIndex] = new Val(L.Val, L.Precision);
        ++exprIndex;
        break;
      }
      case Lex::SinFunc:
      case Lex::CosFunc: {
        ParseTrigOrNegation(exprIndex, end, exprs, closeParens);
        break;
      }
      case Lex::Negative: {
        ParseNegation(exprIndex, end, exprs, closeParens, negationChildren);
        // Print("Found negation expr " + Lexer::LexemeString(L));
        // bool gotIndex = false;
        // size_t closeParenIndex = CloseParenIndex(exprIndex, end, L.Kind != Lex::Negative, gotIndex);
        // Print("  got close paren index " + to_string(closeParenIndex) + ", gotIndex: " + (gotIndex ? "true" : "false"));
        // if (!gotIndex) {
        //   // The following assumes that the child of a negation expression is
        //   // a variable or a number and so is contained solely in the immediate
        //   // lexeme following the negation lexeme.
        //   // TODO: parse negation followed by trig function, e,g -sin(x)
        //   // TODO: parse negation followed by negation, e.g. --x
        //   Expr *child = ParseRange(exprIndex + 1, exprIndex + 2);
        //   if (!child) {
        //     Error("negation expr has a null child");
        //     return exprs;
        //   }
        //   Print("Child of negation expr: " + child->ToString());
        //   Expr *actualExpr = new Neg(child);
        //   exprs[exprIndex] = actualExpr;
        //   negationChildren[exprIndex] = exprIndex + 1;
        //   ++exprIndex;
        // } else {
        //   ParseTrigOrNegation(exprIndex, end, exprs, closeParens);
        // }
        break;
      }
      default: {
        ++exprIndex;
        break;
      }
    }
  }

  return exprs;
}

Expr *ParserAlt::ParseNegation(size_t &exprIndex, size_t end, map<size_t, Expr *> &exprs, map<size_t, size_t> &closeParens, map<size_t, size_t> &negationChildren) {
  Lexeme L = lexemes.at(exprIndex);
  Print("Found negation expr " + Lexer::LexemeString(L));
  bool gotIndex = false;
  size_t closeParenIndex = CloseParenIndex(exprIndex, end, false, gotIndex);
  Print("  got close paren index " + to_string(closeParenIndex) + ", gotIndex: " + (gotIndex ? "true" : "false"));
  if (!gotIndex) {
    if (exprIndex + 1 >= end) {
      Error("negation is not followed by anything");
      return nullptr;
    }
    Lexeme next = lexemes.at(exprIndex + 1);
    Print("  next lexeme after negation: " + Lexer::LexemeString(next));
    switch (next.Kind) {
      case Lex::SinFunc:
      case Lex::CosFunc: {
        Print("  child of negation expression is a trig function");
        const size_t idx = exprIndex;
        ++exprIndex;
        Expr *child = ParseTrigOrNegation(exprIndex, end, exprs, closeParens);
        if (!child) {
          Error("trig function child of negation is null");
          return nullptr;
        }
        Print("&nbsp;&nbsp;child from parsing negation of trig function: " + child->ToString());
        Print("  exprIndex is now " + to_string(exprIndex) + ", end is " + to_string(end) + ", lexemes size is " + to_string(lexemes.size()));
        if (exprIndex < end) {
          Print("  exprIndex is now " + to_string(exprIndex) + " which is lexeme " + Lexer::LexemeString(lexemes.at(exprIndex)));
        }
        Expr *actualExpr = new Neg(child);
        exprs[idx] = actualExpr;
        negationChildren[idx] = idx + 1;
        return actualExpr;
      }
      case Lex::Negative: {
        const size_t idx = exprIndex;
        ++exprIndex;
        Expr *child = ParseNegation(exprIndex, end, exprs, closeParens, negationChildren);
        if (!child) {
          Error("negation of negation is null");
          return nullptr;
        }
        Print("&nbsp;&nbsp;child from parsing negation of negation: " + child->ToString());
        Print("  exprIndex is now " + to_string(exprIndex) + ", end is " + to_string(end) + ", lexemes size is " + to_string(lexemes.size()));
        if (exprIndex < end) {
          Print("  exprIndex is now " + to_string(exprIndex) + " which is lexeme " + Lexer::LexemeString(lexemes.at(exprIndex)));
        }
        Expr *actualExpr = new Neg(child);
        exprs[idx] = actualExpr;
        negationChildren[idx] = idx + 1;
        return actualExpr;
      }
      default: {
        Expr *child = ParseRange(exprIndex + 1, exprIndex + 2);
        if (!child) {
          Error("negation expr has a null singleton (variable or number) child");
          return nullptr;
        }
        Print("Child of negation expr: " + child->ToString());
        Expr *actualExpr = new Neg(child);
        exprs[exprIndex] = actualExpr;
        negationChildren[exprIndex] = exprIndex + 1;
        ++exprIndex;
        return actualExpr;
      }
    }
  }

  return ParseTrigOrNegation(exprIndex, end, exprs, closeParens);
}

Expr *ParserAlt::ParseTrigOrNegation(size_t &exprIndex, size_t end, map<size_t, Expr *> &exprs, map<size_t, size_t> &closeParens) {
  Lexeme L = lexemes.at(exprIndex);
  bool isTrig = L.Kind == Lex::SinFunc || L.Kind == Lex::CosFunc;
  Print("Parsing trig or negation expr " + Lexer::LexemeString(L));
  bool gotIndex = false;
  size_t closeParenIndex = CloseParenIndex(exprIndex, end, isTrig, gotIndex);
  Print("got close paren index " + to_string(closeParenIndex) + ", gotIndex: " + (gotIndex ? "true" : "false"));
  if (!gotIndex) {
    Error("trig function does not have a closing paren index");
    return nullptr;
  }
  Lexeme closeParenLex = lexemes.at(closeParenIndex);
  Debug("Got close paren index: " + to_string(closeParenIndex) + " which is lexeme " + Lexer::LexemeString(closeParenLex));
  closeParens[exprIndex] = closeParenIndex;
  Expr *child = ParseRange(exprIndex + 1, closeParenIndex + 1);
  if (!child) {
    Error("trig or negation expr at index " + to_string(exprIndex) + " has a null child");
    return nullptr;
  }
  Print("Child of trig or negation expr: " + child->ToString());
  Expr *actualExpr = nullptr;
  if (L.Kind == Lex::SinFunc) actualExpr = new Sin(child);
  else if (L.Kind == Lex::CosFunc) actualExpr = new Cos(child);
  else if (L.Kind == Lex::Negative) actualExpr = new Neg(child);
  exprs[exprIndex] = actualExpr;
  exprIndex = closeParenIndex + 1;
  return actualExpr;
}

size_t ParserAlt::CloseParenIndex(size_t start, size_t end, bool checkTrig, bool &gotIndex) {
  size_t index = start + 1;
  if (index > end) {
    Error("Trig function or negation is not followed by anything");
    gotIndex = false;
    return 0;
  }

  Lexeme next = lexemes.at(index);
  if (next.Kind != Lex::OpenParen) {
    if (checkTrig) Error("Trig function not immediately followed by open paren");
    gotIndex = false;
    return 0;
  }

  int parenLevel = 1;
  while (index < end && parenLevel != 0) {
    ++index;
    next = lexemes.at(index);
    if (next.Kind == Lex::OpenParen) {
      ++parenLevel;
    } else if (next.Kind == Lex::CloseParen) {
      // TODO: check for balanced parens
      --parenLevel;
    }
  }

  if (parenLevel != 0) {
    Error("Unbalanced parens while trying to get trig function close paren index");
    gotIndex = false;
    return 0;
  }

  gotIndex = true;
  return index;
}

map<size_t, float> ParserAlt::GetOperators(size_t start, size_t end, map<size_t, size_t> closeParens) {
  Debug("### Getting operators from " + to_string(start) + " to " + to_string(end) + " ###");
  map<size_t, float> operators;
  int parenLevel = 0;
  for (size_t index = start; index < end; ++index) {
    Debug("getting operators: processing lexeme " + Lexer::LexemeString(lexemes.at(index)) + " at index " + to_string(index));
    auto It = closeParens.find(index);
    if (It != closeParens.end()) {
      size_t closeParenIndex = It->second;
      Debug("!!! index " + to_string(index) + " has trig close paren at " + to_string(closeParenIndex) + " !!!");
      index = closeParenIndex;
      continue;
    }
    Lexeme L = lexemes.at(index);
    switch (L.Kind) {
      case Lex::OpenParen: {
        ++parenLevel;
        break;
      }
      case Lex::CloseParen: {
        --parenLevel;
        // TODO: move balanced paren check into its own pass - don't do this check here
        // if (parenLevel != 0) {
        //   Error("parenLevel is nonzero after processing a close paren, so parentheses are not balanced");
        //   return operators;
        // }
        break;
      }
      case Lex::Plus: {
        operators[index] = parenLevel + 0.0f;
        break;
      }
      case Lex::Minus: {
        operators[index] = parenLevel + 0.0f;
        break;
      }
      case Lex::Times: {
        operators[index] = parenLevel + 0.1f;
        break;
      }
      case Lex::Divide: {
        operators[index] = parenLevel + 0.1f;
        break;
      }
      case Lex::Power: {
        operators[index] = parenLevel + 0.2f;
        break;
      }
      default:
        break;
    }
  }
  if (parenLevel != 0) {
    Error("parenLevel is nonzero at the end of the lexeme chunk, so parentheses are not balanced");
  }
  return operators;
}

Expr *ParserAlt::LeftChild(size_t index, size_t start, map<size_t, Expr *> &finalExprs) {
  size_t left = index - 1;
  while (left >= start) {
    auto It = finalExprs.find(left);
    if (It != finalExprs.end()) {
      Expr *Left = It->second;
      finalExprs.erase(It);
      return Left;
    }
    --left;
  }
  return nullptr;
}

Expr *ParserAlt::RightChild(size_t index, size_t end, map<size_t, Expr *> &finalExprs) {
  size_t right = index + 1;
  while (right < end) {
    auto It = finalExprs.find(right);
    if (It != finalExprs.end()) {
      Expr *Right = It->second;
      finalExprs.erase(It);
      return Right;
    }
    ++right;
  }
  return nullptr;
}

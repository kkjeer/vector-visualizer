#include "Parsing/Parser.h"
#include <iostream>

void Parser::Print(string str) {
  debug->append(QString(str.c_str()));
}

void Parser::Debug(string str) {
  Print(str);
}

Expr *Parser::Parse(string str) {
  Print("Parsing s: " + str);
  error = Error::None;
  Debug("Error: " + to_string(error));

  if (str.length() < 1) {
    error = Error::EmptyString;
    Print("~~~~~~ ERROR: cannot parse an empty string");
    return nullptr;
  }

  map<size_t, float> operators = GetOperators(str);
  if (HasError()) {
    return nullptr;
  }

  if (operators.size() < 1) {
    Debug("   No operators - single node expression");
    string::iterator end_pos = remove(str.begin(), str.end(), ' ');
    str.erase(end_pos, str.end());
    Debug("   String without spaces: " + str);
    if (str == "x")
      return new X();
    else if (str == "y")
      return new Y();
    else if (str == "z")
      return new Z();
    else
      return Digit(str);
  }

  size_t count = 0;
  string mapStr = "{ ";
  for (const auto &Pair : operators) {
    ++count;
    mapStr += "<" + to_string(Pair.first) + ", " + string(1, str.at(Pair.first)) + "> => " + Precision(Pair.second, 1);
    if (count < operators.size())
      mapStr += ", ";
  }
  mapStr += " }";
  Debug("   Operators: " + mapStr);

  // Sort the operators by precedence.
  vector<size_t> sortedIndices;
  for (const auto &Pair : operators) {
    sortedIndices.push_back(Pair.first);
  }
  sort(sortedIndices.begin(), sortedIndices.end(), [operators](size_t A, size_t B) {
    float a = operators.at(A);
    float b = operators.at(B);
    if (a == b) return A > B;
    return a < b;
  });

  // Print the sorted operators for debugging.
  string d = "   Sorted indices: { ";
  for (auto I = sortedIndices.begin(); I != sortedIndices.end(); ++I) {
    d += to_string(*I) + ", ";
  }
  d += "}";
  Debug(d);

  set<size_t> processedOps;
  Expr *Root = GetExpr(str, sortedIndices, operators, processedOps, 0);
  return Root;
}

bool Parser::HasError() {
  return error != Error::None;
}

string Parser::GetErrorMessage(string str) {
  switch (error) {
    case Error::EmptyString:
      return "Cannot create an equation from an empty string.";
    case Error::UnbalancedParens:
      return "The input string contains unbalanced parenthese. TODO: incorporate index of bad paren?";
    case Error::IllegalCharacter: {
      if (badIndex >= 0 && badIndex < str.length()) {
        char &c = str.at(badIndex);
        return "Illegal character '" + string(1, c) + "' at index " + to_string(badIndex + 1) + " in the input string.";
      }
      return "Illegal character.";
      
    }
    case Error::UnknownOperator:
      return "Unknown binary operator (TODO: incoporate operator)";
    case Error::NullLeftChild: {
      if (nullChildOpIndex >= 0 && nullChildOpIndex < str.length()) {
        char &op = str.at(nullChildOpIndex);
        return "Operator " + string(1, op) + " at index " + to_string(nullChildOpIndex + 1) + " in the input string does not have a left child.";
      }
      return "An operator is missing a left child.";
    }
    case Error::NullRightChild: {
      if (nullChildOpIndex >= 0 && nullChildOpIndex < str.length()) {
        char &op = str.at(nullChildOpIndex);
        return "Operator " + string(1, op) + " at index " + to_string(nullChildOpIndex + 1) + " in the input string does not have a right child.";
      }
      return "An operator is missing a right child.";
    }
    case Error::BadLeafNode: {
      return "The input string does not contain any operators and is not x, y, z, or a number.";
    }
    case Error::MultipleDots: {
      return "A numerical value cannot contain more than one decimal point.";
    }
    default:
      return "Could not parse input string. Please try again.";
  }
  return "";
}

map<size_t, float> Parser::GetOperators(string str) {
  map<size_t, float> result;
  int parenLevel = 0;
  for (size_t i = 0; i < str.length(); ++i) {
    char &c = str.at(i);
    switch (c) {
      case '(': {
        ++parenLevel;
        // Debug("   Found open paren, so parenLevel is now " + to_string(parenLevel));
        break;
      }
      case ')': {
        --parenLevel;
        // Debug("   Found close paren, so parenLevel is now " + to_string(parenLevel));
        if (parenLevel != 0) {
          Debug("~~~~~~ ERROR: parenLevel is nonzero after processing a close paren, so parentheses are not balanced");
          error = Error::UnbalancedParens;
          return result;
        }
        break;
      }
      case '+':
      case '-':
      case '*':
      case '/':
      case '^': {
        // Debug("   Found operator " + string(1, c) + " and setting precedence to parenLevel " + to_string(parenLevel));
        result[i] = parenLevel;
        if (c == '+') result[i] += 0.0f;
        if (c == '-') result[i] += 0.0f;
        if (c == '*') result[i] += 0.1f;
        if (c == '/') result[i] += 0.1f;
        if (c == '^') result[i] += 0.2f;
        break;
      }
      case 'x':
      case 'y':
      case 'z':
      case '.':
      // case 's':
      // case 'i':
      // case 'n':
      // case 'c':
      // case 'o':
      case ' ': {
        break;
      }
      // case 's': {
      //   if (i < str.length() - 2) {
      //     if (str.at(i + 1) == 'i' && str.at(i + 2) == 'n') {
      //       Debug("Found sin");
      //       result[i] += 1.1f;
      //       char &next = c;
      //       while (next != ')' && i < str.length()) {
      //         ++i;
      //         if (i < str.length())
      //           next = str.at(i);
      //       }
      //       Debug("i is now " + to_string(i));
      //     } else {
      //       Print("~~~~~~ ERROR: found s but next two characters are not i and n");
      //       return result;
      //     }
      //   } else {
      //     Print("~~~~~~ ERROR: found s but not enough space for a sin");
      //     return result;
      //   }
      //   break;
      // }
      default: {
        if (!IsDigit(c)) {
          Print("~~~~~~ ERROR: found illegal character " + string(1, c) + " while trying to get operator precedences");
          error = Error::IllegalCharacter;
          badIndex = i;
          return result;
        }
        break;
      }
    }
  }
  if (parenLevel != 0) {
    Print("~~~~~~ ERROR: parenLevel is nonzero at the end of the string, so parentheses are not balanced");
    error = Error::UnbalancedParens;
    return result;
  }
  return result;
}

Expr *Parser::GetExpr(string str, vector<size_t> sortedIndices, map<size_t, float> operators, set<size_t> &processedOps, size_t i) {
  size_t index = sortedIndices[i];
  processedOps.emplace(index);
  char c = str.at(index);
  float precedence = operators[index];
  Debug("   Processing index " + to_string(index) + " with operator " + string(1, c) + " and precedence " + Precision(precedence, 1));
  size_t left = -1, right = -1;
  size_t indicesLength = sortedIndices.size();

  // Get the left child index (or right child index)
  if (indicesLength >= 1 && i < indicesLength - 1) {
    size_t nextIndex = sortedIndices[i + 1];
    if (processedOps.find(nextIndex) != processedOps.end()) {
      Debug("      next index " + to_string(nextIndex) + " has already been processed so it cannot be a child of " + to_string(index));
    } else {
      float nextPrecedence = operators[nextIndex];
      if (nextPrecedence < precedence) {
        Debug("      next index " + to_string(nextIndex) + " cannot be a child of index " + to_string(index) + " since its precedence is lower");
      } else {
        if (nextIndex < index) {
          left = i + 1;
          processedOps.emplace(nextIndex);
          Debug("      index " + to_string(index) + " has next left child index " + to_string(nextIndex));
        } else {
          right = i + 1;
          processedOps.emplace(nextIndex);
          Debug("      index " + to_string(index) + " has next right child index " + to_string(nextIndex));
        }
      }
    }
  }

  // If the left or right hasn't been set by the next index (at i + 1), try checking the skip index (at i + 2).
  if (((int)left == -1 || (int)right == -1) && indicesLength >= 2 && i < indicesLength - 2) {
    size_t skipIndex = sortedIndices[i + 2];
    if (processedOps.find(skipIndex) != processedOps.end()) {
      Debug("      skip index " + to_string(skipIndex) + " has already been processed so it cannot be a child of " + to_string(index));
    } else {
      float skipPrecedence = operators[skipIndex];
      if (skipPrecedence < precedence) {
        Debug("      skip index " + to_string(skipIndex) + " cannot be a child of index " + to_string(index) + " since its precedence is lower");
      } else {
        if ((int)left == -1 && skipIndex < index) {
          left = i + 2;
          processedOps.emplace(skipIndex);
          Debug("      index " + to_string(index) + " has skip left child index " + to_string(skipIndex));
        } else if ((int)right == -1 && skipIndex > index) {
          right = i + 2;
          processedOps.emplace(skipIndex);
          Debug("      index " + to_string(index) + " has skip right child index " + to_string(skipIndex));
        }
      }
    }
  }

  Expr *leftChild = nullptr;
  if ((int)left != -1) {
    Debug("      index " + to_string(index) + " has left child rooted at index " + to_string(sortedIndices[left]));
    leftChild = GetExpr(str, sortedIndices, operators, processedOps, left);
  } else {
    Debug("      index " + to_string(index) + " does not have a binary operator left child");
    leftChild = LeafNodeLeftChild(str, index);
  }

  Expr *rightChild = nullptr;
  if ((int)right != -1) {
    Debug("      index " + to_string(index) + " has right child rooted at index " + to_string(sortedIndices[right]));
    rightChild = GetExpr(str, sortedIndices, operators, processedOps, right);
  } else {
    Debug("      index " + to_string(index) + " does not have a binary operator right child");
    rightChild = LeafNodeRightChild(str, index);
  }

  if (leftChild) {
    Debug("Left child of index " + to_string(index) +  ": " + leftChild->ToString());
  }
  if (rightChild) {
    Debug("Right child of index " + to_string(index) +  ": " + rightChild->ToString());
  }

  return OperatorExpr(c, index, leftChild, rightChild);
}

Expr *Parser::OperatorExpr(char op, size_t index, Expr *left, Expr *right) {
  if (!left) {
    Print("~~~~~~ ERROR: left child for op " + string(1, op) + " is nullptr - cannot get operator expr");
    error = Error::NullLeftChild;
    nullChildOpIndex = index;
    return nullptr;
  }
  if (!right) {
    Print("~~~~~~ ERROR: right child for op " + string(1, op) + " is nullptr - cannot get operator expr");
    error = Error::NullRightChild;
    nullChildOpIndex = index;
    return nullptr;
  }

  switch (op) {
    case '+': return new Add(left, right);
    case '-': return new Sub(left, right);
    case '*': return new Mult(left, right);
    case '/': return new Div(left, right);
    case '^': return new Pow(left, right);
    default: {
      Print("~~~~~~ ERROR: unrecognized operator " + string(1, op));
      error = Error::UnknownOperator;
      return nullptr;
    }
  }

  return nullptr;
}

Expr *Parser::LeafNodeLeftChild(string str, size_t index) {
  size_t left = index - 1;
  bool search = true;
  while (search) {
    char c = str[left];
    if (IsDigit(c) && c != '.') {
      search = false;
      break;
    }
    switch (c) {
      // case '1':
      // case '2':
      // case '3':
      // case '4':
      // case '5':
      // case '6':
      // case '7':
      // case '8':
      // case '9':
      // case '0': {
      //   // Debug("      Found digit " + string(1, c));
      //   search = false;
      //   break;
      // }
      case 'x':
      case 'y':
      case 'z': {
        // Debug("      Found variable " + string(1, c));
        search = false;
        break;
      }
      case ' ': {
        --left;
        break;
      }
      // case 's': {
      //   Debug("Left node leaf child is sin");
      //   return nullptr;
      // }
      default: {
        Debug("~~~~~~ ERROR: unexpected character " + string(1, c) + " at index " + to_string(left));
        error = Error::IllegalCharacter;
        badIndex = left;
        return nullptr;
      }
    }
  }
  // Debug("      Left is now " + to_string(left) + " with character " + string(1, str[left]));
  switch (str[left]) {
    case 'x': {
      return new X();
    }
    case 'y': {
      return new Y();
    }
    case 'z': {
      return new Z();
    }
    default: {
      return LeftDigit(str, left);
    }
  }
  return nullptr;
}

Expr *Parser::LeafNodeRightChild(string str, size_t index) {
  size_t right = index + 1;
  bool search = true;
  while (search) {
    char c = str[right];
    if (IsDigit(c) && c != '.') {
      search = false;
      break;
    }
    switch (c) {
      // case '1':
      // case '2':
      // case '3':
      // case '4':
      // case '5':
      // case '6':
      // case '7':
      // case '8':
      // case '9':
      // case '0': {
      //   // Debug("      Found digit " + string(1, c));
      //   search = false;
      //   break;
      // }
      case 'x':
      case 'y':
      case 'z': {
        // Debug("      Found variable " + string(1, c));
        search = false;
        break;
      }
      case ' ': {
        ++right;
        break;
      }
      default: {
        Debug("~~~~~~ ERROR: unexpected character " + string(1, c) + " at index " + to_string(right));
        error = Error::IllegalCharacter;
        badIndex = right;
        return nullptr;
      }
    }
  }
  switch (str[right]) {
    case 'x': {
      return new X();
    }
    case 'y': {
      return new Y();
    }
    case 'z': {
      return new Z();
    }
    default: {
      return RightDigit(str, right);
    }
  }
  return nullptr;
}

Expr *Parser::LeftDigit(string str, size_t index) {
  size_t start = index;
  bool search = true;
  string digits = "";
  int precision = 0;
  while (search) {
    char c = str[start];
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
        if (c == '.') {
          precision = index - start;
        }
        --start;
        digits += string(1, c);
        break;
      }
      default: {
        search = false;
        break;
      }
    }
  }

  reverse(digits.begin(), digits.end());

  // Debug("   digits is now " + digits);
  float val = stof(digits);
  return new Val(val, precision);
}

Expr *Parser::RightDigit(string str, size_t index) {
  size_t end = index;
  bool search = true;
  string digits = "";
  int precision = 0;
  size_t dotPos = -1;
  while (search) {
    char c = str[end];
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
        ++end;
        if (c == '.') {
          dotPos = end;
        }
        digits += string(1, c);
        break;
      }
      default: {
        search = false;
        break;
      }
    }
  }

  if ((int)dotPos != -1)
    precision = end - dotPos;
  Debug("   digits is now " + digits + ", precision is " + to_string(precision));
  float val = stof(digits);
  return new Val(val, precision);
}

Expr *Parser::Digit(string str) {
  Debug("   Trying to turn str " + str + " into a digit");
  string digits = "";
  int precision = 0;
  size_t dotPos = -1;
  bool foundNumber = false;
  for (size_t i = 0; i < str.length(); ++i) {
    char &c = str.at(i);
    if (IsDigit(c)) {
      if (c == '.') {
        if ((int)dotPos != -1) {
          error = Error::MultipleDots;
          return nullptr;
        }
        dotPos = i;
      } else {
        foundNumber = true;
      }
      digits += string(1, c);
      continue;
    }
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
        if (c == '.') {
          if ((int)dotPos != -1) {
            error = Error::MultipleDots;
            return nullptr;
          }
          dotPos = i;
        }
        digits += string(1, c);
        break;
      }
      case ' ': {
        Debug("      Found space - TODO: remove spaces from str");
        break;
      }
      default: {
        Debug("~~~~~~ ERROR: found illegal character " + string(1, c) + " while trying to construct a digit");
        error = Error::BadLeafNode;
        return nullptr;
      }
    }
  }

  if (!foundNumber) {
    error = Error::BadLeafNode;
    return nullptr;
  }

  if ((int)dotPos != -1)
    precision = str.length() - dotPos;
  Debug("   digits is now " + digits + ", precision is " + to_string(precision));
  float val = stof(digits);
  return new Val(val, precision);
}

bool Parser::IsDigit(char &c) {
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

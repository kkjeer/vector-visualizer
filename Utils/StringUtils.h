#pragma once
#include <string>
#include <QString>
#include "Utils/MathUtils.h"
#include "Expr.h"

using namespace std;
using namespace Expression;

namespace StringUtils {
  string Italic(string str);
  string Bold(string str);
  string Superscript(string str);
  string Eqn(string func, string v = "t");
  string Equation(string func, string v1 = "x", string v2 = "y", string v3 = "z");
  string Vec3String(Vec3 vec, string open = "{", string close = "}");
  string TrimZeroes(float f, int precision = 2);
  QString Fancy(string str);
}
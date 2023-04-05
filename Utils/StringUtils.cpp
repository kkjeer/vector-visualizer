#include "Utils/StringUtils.h"

string StringUtils::Italic(string str) {
  return "<i>" + str + "</i>";
}

string StringUtils::Bold(string str) {
  return "<b>" + str + "</b>";
}

string StringUtils::Superscript(string str) {
  return "<sup>" + str + "</sup>";
}

string StringUtils::Eqn(string func, string v) {
  return Bold(func) + "(" + Italic(v) + ") = ";
}

string StringUtils::Equation(string func, string v1, string v2, string v3) {
  return Bold(func) + "(" + Italic(v1) + ", " + Italic(v2) + ", " + Italic(v3) + ") = ";
}

string StringUtils::Vec3String(Vec3 vec, string open, string close) {
  return open + " " + TrimZeroes(vec.x) + ", " + TrimZeroes(vec.y) + ", " + TrimZeroes(vec.z) + " " + close;
}

string StringUtils::TrimZeroes(float f, int precision) {
  string str = Precision(f, precision);
  bool foundDot = false;
  string result = "";
  for (size_t i = 0; i < str.length(); ++i) {
    char &c = str.at(i);
    if (c != '0' || !foundDot)
      result += string(1, c);
    if (c == '.')
      foundDot = true;
  }
  if (result.at(result.length() - 1) == '.') {
    return result.substr(0, result.length() - 1);
  }
  return result;
}

QString StringUtils::Fancy(string str) {
  return QString(str.c_str());
}

#include "Expr.h"

extern string Expression::Precision(float f, int precision) {
  std::stringstream stream;
  stream << std::fixed << std::setprecision(precision) << f;
  std::string s = stream.str();
  return s;
}

extern Expression::Expr *Expression::CreateCos(Expr *Child) {
  return new Expression::Cos(Child);
}
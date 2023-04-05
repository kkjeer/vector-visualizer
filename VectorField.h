#ifndef VECTORFIELD_VECTORFIELD
#define VECTORFIELD_VECTORFIELD
#include "Expr.h"
#include "Utils/MathUtils.h"
#include "Utils/StringUtils.h"
#include <vector>
#include <limits>
#include <string>

using namespace Expression;
using namespace std;

class VectorField {
private:
  Expr *I;
  Expr *J;
  Expr *K;

public:
  VectorField(Expr *i, Expr *j, Expr *k) : I(i), J(j), K(k) {}
  Vec3 Eval(float x, float y, float z);
  Vec3 End(float x, float y, float z);
  void MinMaxLengths(float xRange, float yRange, float zRange, float step, float &minLength, float &maxLength);
  VectorField *Curl();
  string ToString();
};

#endif
#include "VectorField.h"

Vec3 VectorField::Eval(float x, float y, float z) {
  struct Vec3 result;
  result.x = I->Eval(x, y, z);
  result.y = J->Eval(x, y, z);
  result.z = K->Eval(x, y, z);
  return result;
}

Vec3 VectorField::End(float x, float y, float z) {
  struct Vec3 result;
  struct Vec3 eval = Eval(x, y, z);
  result.x = x + eval.x;
  result.y = y + eval.y;
  result.z = z + eval.z;
  return result;
}

void VectorField::MinMaxLengths(float xRange, float yRange, float zRange, float step, float &minLength, float &maxLength) {
  minLength = numeric_limits<float>::max();
  maxLength = numeric_limits<float>::min();

  for (float x = -xRange; x <= xRange; x += step) {
    for (float y = -yRange; y <= yRange; y += step) {
      for (float z = -zRange; z <= zRange; z += step) {
        Vec3 start = {x, y, z};
        Vec3 end = End(start.x, start.y, start.z);
        Vec3 point = Eval(x, y, z);
        float len = Vector::Length(point);
        if (len < minLength) minLength = len;
        if (len > maxLength) maxLength = len;
      }
    }
  }
}

VectorField *VectorField::Curl() {
  // i = dK/dy - dJ/dz
  Expr *i = new Sub(K->Derivative('y'), J->Derivative('z'));

  // j = dK/dx - dI/dz
  Expr *j = new Sub(K->Derivative('x'), I->Derivative('z'));

  // k = dJ/dx - dI/dy
  Expr *k = new Sub(J->Derivative('x'), I->Derivative('y'));
  return new VectorField(i->Simplify(), j->Simplify(), k->Simplify());
}

string VectorField::ToString() {
  string i = StringUtils::Bold("i") + " = " + I->ToString();
  string j = StringUtils::Bold("j") + " = " + J->ToString();
  string k = StringUtils::Bold("k") + " = " + K->ToString();
  string space = "&nbsp;&nbsp;";
  return "{<br/>" + space + i + "<br/>" + space + j + "<br/>" + space + k + "<br/>}";
  // return i + "<br/>" + j + "<br/>" + k;
}

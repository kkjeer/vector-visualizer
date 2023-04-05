#pragma once
#include <math.h>
#include <vector>
#include <string>
#include "Expr.h"

using namespace std;
using namespace Expression;

struct Vec2 {
  float x;
  float y;
};

struct Vec3 {
  float x;
  float y;
  float z;
};

struct Color {
  float r;
  float g;
  float b;
  float a;
};

enum ColorSpace {
  Red,
  Green,
  Blue,
  Orange,
  Pink,
  Cyan,
  Yellow,
  Purple,
  Count
};

struct HSL {
  int h;
  int s;
  int l;
};

namespace Vector {
  Vec3 GetVector(Vec3 start, Vec3 end);
  float Length(Vec3 start, Vec3 end);
  float Length(Vec3 vector);
  Vec3 Normalize(Vec3 vector);
  Vec3 SetLength(Vec3 vector, float length);
  Vec3 Cross(Vec3 vector, Vec3 other);
  float Dot(Vec3 vector, Vec3 other);
  Vec3 Vec3Along(Vec3 start, Vec3 end, float distance);
  Vec3 Perpendicular(Vec3 start, Vec3 end);
}

namespace MathUtils {
  float MapToRange(float value, Vec2 fromRange, Vec2 toRange);
  float Clamp(float value, float min, float max);
  float Min(vector<float> vals);
  float Max(vector<float> vals);
  float Abs(float val);
  Color TweenColor(Color A, Color B, float percentage);
  ColorSpace GetColorSpace(size_t index);
  Color LightColor(ColorSpace space);
  Color MediumColor(ColorSpace space);
  string RGBString(Color color);
  HSL ToHSL(Color rgba);
  Color ToRGBA(HSL hsl);
}
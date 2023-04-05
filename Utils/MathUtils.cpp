#include "Utils/MathUtils.h"
#include <math.h>

Vec3 Vector::GetVector(Vec3 start, Vec3 end) {
  Vec3 vector;
  vector.x = end.x - start.x;
  vector.y = end.y - start.y;
  vector.z = end.z - start.z;
  return vector;
}

float Vector::Length(Vec3 start, Vec3 end) {
  return Length(GetVector(start, end));
}

float Vector::Length(Vec3 vector) {
  return sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

Vec3 Vector::Normalize(Vec3 vector) {
  float length = Length(vector);
  Vec3 result;
  result.x = vector.x / length;
  result.y = vector.y / length;
  result.z = vector.z / length;
  return result;
}

Vec3 Vector::SetLength(Vec3 vector, float length) {
  float len = Length(vector);
  Vec3 result;
  result.x = length * vector.x / len;
  result.y = length * vector.y / len;
  result.z = length * vector.z / len;
  return result;
}

Vec3 Vector::Cross(Vec3 vector, Vec3 other) {
  Vec3 cross;
  cross.x = vector.y * other.z - vector.z * other.y;
  cross.y = vector.z * other.x - vector.x * other.z;
  cross.z = vector.x * other.y - vector.y * other.x;
  return cross;
}

float Vector::Dot(Vec3 vector, Vec3 other) {
  return vector.x * other.x + vector.y * other.y + vector.z * other.z;
}

/**
 * Returns a Vec3 `distance` away from the start Vec3, in the direction of the end Vec3.
 */
Vec3 Vector::Vec3Along(Vec3 start, Vec3 end, float distance) {
  Vec3 vector = GetVector(start, end);
  Vec3 normalized = Normalize(vector);

  Vec3 result;
  result.x = start.x + distance * normalized.x;
  result.y = start.y + distance * normalized.y;
  result.z = start.z + distance * normalized.z;
  return result;
}

/**
 * Returns a vector that is perpendicular to the vector from `start` to `end`.
 */
Vec3 Vector::Perpendicular(Vec3 start, Vec3 end) {
  Vec3 vector = GetVector(start, end);

  // Get the basis vector to use for the cross product.
  // Must not be parallel to `vector`.
  Vec3 basis = { 1, 0, 0 };
  if (vector.y == 0 && vector.z == 0) {
    basis = {0, 1, 0};
  }

  Vec3 cross = Cross(vector, basis);
  return cross;
}

float MathUtils::MapToRange(float value, Vec2 fromRange, Vec2 toRange) {
  float fromSize = Abs(fromRange.y - fromRange.x);
  float toSize = Abs(toRange.y - toRange.x);
  // If either range has a size of approximately zero, value maps to
  // the maximum of the to range.
  // E.g. [10, 10], [0, 5] => 5.
  // E.g. [3, 7], [20, 20] => 20;
  if (fromSize < 0.001 || toSize < 0.001) {
    return toRange.y;
  }

  float percentage = Abs(value - fromRange.x)/fromSize;
  return toRange.x + percentage * toSize;
}

float MathUtils::Clamp(float value, float min, float max) {
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

float MathUtils::Min(vector<float> vals) {
  float min = numeric_limits<float>::max();
  for (const auto &It : vals) {
    float v = It;
    if (v < min) min = v;
  }
  return min;
}

float MathUtils::Max(vector<float> vals) {
  float max = numeric_limits<float>::min();
  for (const auto &It : vals) {
    float v = It;
    if (v > max) max = v;
  }
  return max;
}

float MathUtils::Abs(float val) {
  if (val >= 0) return val;
  return -val;
}

Color MathUtils::TweenColor(Color A, Color B, float percentage) {
  float r = Abs(percentage * B.r + (1.0 - percentage) * A.r);
  float g = Abs(percentage * B.g + (1.0 - percentage) * A.g);
  float b = Abs(percentage * B.b + (1.0 - percentage) * A.b);
  float a = Abs(percentage * B.a + (1.0 - percentage) * A.a);
  return {r, g, b, a};
}

ColorSpace MathUtils::GetColorSpace(size_t index) {
  size_t i = index % ColorSpace::Count;
  return (ColorSpace)i;
}

Color MathUtils::LightColor(ColorSpace space) {
  switch (space) {
    case ColorSpace::Red:
      return {0.8, 0.4, 0.4, 1};
    case ColorSpace::Green:
      return {0.4, 0.8, 0.4, 1};
    case ColorSpace::Blue:
      return {0.3, 0.5, 0.7, 1};
    case ColorSpace::Orange:
      return {0.8, 0.6, 0.4, 1};
    case ColorSpace::Pink:
      return {0.9, 0.5, 0.7, 1};
    case ColorSpace::Cyan:
      return {0.4, 0.7, 0.7, 1};
    case ColorSpace::Yellow:
      return {0.8, 0.8, 0.4, 1};
    case ColorSpace::Purple:
      return {0.7, 0.4, 0.8, 1};
    default:
      return {0.0, 0.0, 0.0, 1};
  }
}

Color MathUtils::MediumColor(ColorSpace space) {
  switch (space) {
    case ColorSpace::Red:
      return {0.7, 0, 0, 1};
    case ColorSpace::Green:
      return {0, 0.6, 0, 1};
    case ColorSpace::Blue:
      return {0, 0.3, 0.7, 1};
    case ColorSpace::Orange:
      return {0.8, 0.5, 0, 1};
    case ColorSpace::Pink:
      return {0.9, 0, 0.5, 1};
    case ColorSpace::Cyan:
      return {0, 0.7, 0.7, 1};
    case ColorSpace::Yellow:
      return {0.7, 0.7, 0, 1};
    case ColorSpace::Purple:
      return {0.6, 0, 0.6, 1};
    default:
      return {0, 0, 0, 1};
  }
}

string MathUtils::RGBString(Color color) {
  float r = color.r * 255.0f;
  float g = color.g * 255.0f;
  float b = color.b * 255.0f;
  r = Clamp(r, 0, 245);
  b = Clamp(b, 0, 245);
  g = Clamp(g, 0, 245);
  return "rgb(" + Precision(r, 1) + ", " + Precision(g, 1) + ", " + Precision(b, 1) + ")";
}

HSL MathUtils::ToHSL(Color rgba) {
  // r, g, and b are fractions of 1
  float r = rgba.r;
  float g = rgba.g;
  float b = rgba.b;

  // Find greatest and smallest channel values
  float cmin = Min({r, g, b}),
        cmax = Max({r, g, b}),
        delta = cmax - cmin;
  int h = 0,
      s = 0,
      l = 0;

  // Calculate hue
  // No difference
  if (delta == 0)
    h = 0;
  // Red is max
  else if (cmax == r)
    h = ((int)((g - b) / delta)) % 6;
  // Green is max
  else if (cmax == g)
    h = (b - r) / delta + 2;
  // Blue is max
  else
    h = (r - g) / delta + 4;
  h = (int)(h * 60);
  // Make negative hues positive behind 360°
  if (h < 0)
    h += 360;

  // Calculate lightness
  l = (int)((cmax + cmin) / 2);

  // Calculate saturation
  if (delta == 0)
    s = 0;
  else {
    int absoluteVal = Abs(2 * l - 1);
    s = (int)(delta / (1 - absoluteVal));
  }
    
  // Multiply l and s by 100
  s *= 100;
  l *= 100;

  return {h, s, l};
}

Color MathUtils::ToRGBA(HSL hsl) {
  int h = hsl.h;
  // s and l must be fractions of 1
  float s = hsl.s / 100;
  float l = hsl.l / 100;

  float c = (1 - Abs(2 * l - 1)) * s,
      x = c * (1 - Abs((h / 60) % 2 - 1)),
      m = l - c/2,
      r = 0,
      g = 0,
      b = 0;

  return {r, g, b, 1};
}
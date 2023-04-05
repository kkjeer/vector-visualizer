#pragma once
#ifdef __APPLE__
/* Defined before OpenGL and GLUT includes to avoid deprecation messages - this doesn't actually work */
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif
#include "Utils/MathUtils.h"
#include <string>

using namespace std;

namespace NumberGraphic {
  enum Direction {
    LToR = 0,
    RToL
  };

  const float width = 0.04f;
  const float space = 0.03f;
  const float dot = 0.0325f;
  const float height = 0.08f;
  const float thickness = 5.0f;

  void DrawNumber(string str, float x, float y, float z, Direction dir);
  void DrawAxis(char c, float x, float y, float z);
  void DrawChar(char &c);
  void Minus();
  void Dot();
  void Zero();
  void One();
  void Two();
  void Three();
  void Four();
  void Five();
  void Six();
  void Seven();
  void Eight();
  void Nine();
  void XAxis();
  void YAxis();
  void ZAxis();
  void Line(struct Vec3 start, struct Vec3 end);
};
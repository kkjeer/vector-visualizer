#include "Number.h"

void NumberGraphic::DrawNumber(string str, float x, float y, float z, Direction dir) {
  size_t len = str.length();
  bool left = dir == Direction::LToR;

  // Determine the position of the leftmost character based on direction.
  float X = x;
  if (!left) {
    for (size_t i = 0; i < len; ++i) {
      char &c = str.at(i);
      if (c == '.') {
        X -= dot;
      } else {
        X -= width + space;
      }
    }
  }

  for (size_t i = 0; i < len; ++i) {
    char &c = str.at(i);
    glPushMatrix();
    glTranslatef(X, y, z);
    DrawChar(c);
    glPopMatrix();
    if (c == '.') {
      X += dot;
    } else {
      X += width + space;
    }
  }
}

void NumberGraphic::DrawAxis(char c, float x, float y, float z) {
  glPushMatrix();
  glTranslatef(x, y, z);
  DrawChar(c);
  glPopMatrix();
}

void NumberGraphic::DrawChar(char &c) {
  switch (c) {
    case '-':
      Minus();
      break;
    case '.':
      Dot();
      break;
    case '0':
      Zero();
      break;
    case '1':
      One();
      break;
    case '2':
      Two();
      break;
    case '3':
      Three();
      break;
    case '4':
      Four();
      break;
    case '5':
      Five();
      break;
    case '6':
      Six();
      break;
    case '7':
      Seven();
      break;
    case '8':
      Eight();
      break;
    case '9':
      Nine();
      break;
    case 'x':
      XAxis();
      break;
    case 'y':
      YAxis();
      break;
    case 'z':
      ZAxis();
      break;
  }
}

void NumberGraphic::Minus() {
  Line({0, 0.5, 0}, {1, 0.5, 0});
}

void NumberGraphic::Dot() {
  Line({0.1, 0, 0}, {0.1, 0.2, 0});
}

void NumberGraphic::Zero() {
  Line({0, 0, 0}, {0, 1, 0});
  Line({0, 1, 0}, {1, 1, 0});
  Line({1, 1, 0}, {1, 0, 0});
  Line({1, 0, 0}, {0, 0, 0});
}

void NumberGraphic::One() {
  Line({0.5, 0, 0}, {0.5, 1, 0});
}

void NumberGraphic::Two() {
  Line({0, 1, 0}, {1, 1, 0});
  Line({1, 1, 0}, {1, 0.5, 0});
  Line({1, 0.5, 0}, {0, 0.5, 0});
  Line({0, 0.5, 0}, {0, 0, 0});
  Line({0, 0, 0}, {1, 0, 0});
}

void NumberGraphic::Three() {
  Line({1, 0, 0}, {1, 1, 0});
  Line({1, 1, 0}, {0, 1, 0});
  Line({1, 0.5, 0}, {0, 0.5, 0});
  Line({1, 0, 0}, {0, 0, 0});
}

void NumberGraphic::Four() {
  Line({0, 1, 0}, {0, 0.5, 0});
  Line({0, 0.5, 0}, {1, 0.5, 0});
  Line({1, 1, 0}, {1, 0, 0});
}

void NumberGraphic::Five() {
  Line({1, 1, 0}, {0, 1, 0});
  Line({0, 1, 0}, {0, 0.5, 0});
  Line({0, 0.5, 0}, {1, 0.5, 0});
  Line({1, 0.5, 0}, {1, 0, 0});
  Line({1, 0, 0}, {0, 0, 0});
}

void NumberGraphic::Six() {
  Line({1, 1, 0}, {0, 1, 0});
  Line({0, 1, 0}, {0, 0, 0});
  Line({0, 0, 0}, {1, 0, 0});
  Line({1, 0, 0}, {1, 0.5, 0});
  Line({1, 0.5, 0}, {0, 0.5, 0});
}

void NumberGraphic::Seven() {
  Line({0, 1, 0}, {1, 1, 0});
  Line({1, 1, 0}, {1, 0, 0});
}

void NumberGraphic::Eight() {
  Line({0, 0, 0}, {0, 1, 0});
  Line({0, 1, 0}, {1, 1, 0});
  Line({1, 1, 0}, {1, 0, 0});
  Line({1, 0, 0}, {0, 0, 0});
  Line({0, 0.5, 0}, {1, 0.5, 0});
}

void NumberGraphic::Nine() {
  Line({1, 0.5, 0}, {0, 0.5, 0});
  Line({0, 0.5, 0}, {0, 1, 0});
  Line({0, 1, 0}, {1, 1, 0});
  Line({1, 1, 0}, {1, 0, 0});
}

void NumberGraphic::XAxis() {
  Line({-0.1, 0, 0}, {1.1, 1, 0});
  Line({-0.1, 1, 0}, {1.1, 0, 0});
}

void NumberGraphic::YAxis() {
  Line({0, 1, 0}, {0.25, 0.25, 0});
  Line({1, 1, 0}, {0, 0, 0});
}

void NumberGraphic::ZAxis() {
  Line({0, 1, 0}, {1, 1, 0});
  Line({1, 1, 0}, {0, 0, 0});
  Line({0, 0, 0}, {1, 0, 0});
}

void NumberGraphic::Line(struct Vec3 start, struct Vec3 end) {
  float x1 = start.x, y1 = start.y, x2 = end.x, y2 = end.y;

  // Vertical lines on the left and right edges.
  if (x1 == y1) {
    if (x1 == 0) {
      x1 += 0.05;
      x2 += 0.05;
    } else if (x1 == 1) {
      x1 -= 0.05;
      x2 -= 0.05;
    }
  }
  // Horizontal lines on the bottom and top edges.
  if (y1 == y2) {
    if (y1 == 0) {
      y1 += 0.06;
      y2 += 0.06;
    } else if (y1 == 1) {
      y1 -= 0.06;
      y2 -= 0.06;
    }
  }

  // Draw the line from start to end.
  glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
  glLineWidth(thickness);
    glBegin(GL_LINES);
    glVertex3f(x1 * width, y1 * height, start.z);
    glVertex3f(x2 * width, y2 * height, end.z);
  glEnd();
}
#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include <QWidget>
#include <QOpenGLWidget>
#include <QTextBrowser>
#include <QTimer>
#include <QMouseEvent>
#include <QWheelEvent>
#include "Utils/MathUtils.h"
#include "VectorField.h"
#include "Graphics/Number.h"
#include "Expr.h"
#include "Utils/StringUtils.h"
#include <string>
#include <vector>
#include <set>
#include <map>
#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
/* Defined before OpenGL and GLUT includes to avoid deprecation messages - this doesn't actually work */
// #define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif

using namespace std;
using namespace Expression;
using namespace StringUtils;

enum GraphicsMode {
  Vectors = 0,
  Function,
  Field
};

struct GraphicsVector {
  struct Vec3 start;
  struct Vec3 end;
  Color color;
  string crossA;
  string crossB;
  float dotProduct;
};

struct BoundingBox {
  Vec3 min;
  Vec3 max;
};

class OGLWidget : public QOpenGLWidget
{
public:
  OGLWidget(QWidget *parent = 0);
  ~OGLWidget();

  void SetDebug(QTextBrowser *d) {
    debug = d;
  }

  void SetMode(GraphicsMode m) {
    mode = m;
    orbitCamera = false;
    if (mode == GraphicsMode::Vectors) {
      ResetCameraVectors();
    } else if (mode == GraphicsMode::Function) {
      ResetCameraFunc();
    } else {
      ResetCameraVectorField();
    }
  }

  void SetBackgroundColor(Color color, bool darkMode) {
    if (darkMode) {
      backgroundColor = { 0.9, 0.9, 0.9, 1 };
    } else {
      backgroundColor = color;
      float diff = -0.02;
      backgroundColor.r += diff;
      backgroundColor.g += diff;
      backgroundColor.b += diff;
    }
  }

  Color AddVector(Vec3 start, Vec3 end) {
    size_t colorIndex = 0;
    for (const auto &Vec : vectors) {
      if (Vec.crossA == "" && Vec.crossB == "") {
        ++colorIndex;
      }
    }
    Color color = MathUtils::MediumColor(MathUtils::GetColorSpace(colorIndex));
    AddVector(start, end, color);
    return color;
  }

  void AddVector(Vec3 start, Vec3 end, Color color, string crossA = "", string crossB = "", float dot = 0.0f) {
    struct GraphicsVector v;
    v.start = start;
    v.end = end;
    v.color = color;
    v.crossA = crossA;
    v.crossB = crossB;
    if (crossA != "" && crossB != "") {
      v.dotProduct = dot;
    }
    vectors.push_back(v);

    Debug("Added vector from start = { " + Precision(start.x) + ", " + Precision(start.y) + ", " + Precision(start.z) + " } to end = { " + Precision(end.x) + ", " + Precision(end.y) + ", " + Precision(end.z) + " }");
    SetBoundingBox();
    Debug("Bounding box: min = { " + Precision(box.min.x) + ", " + Precision(box.min.y) + ", " + Precision(box.min.z) + " }, max = { " + Precision(box.max.x) + ", " + Precision(box.max.y) + ", " + Precision(box.max.z) + " }");
  }

  Color CrossVectors(size_t a, size_t b) {
    GraphicsVector A = vectors.at(a);
    GraphicsVector B = vectors.at(b);
    Vec3 P = Vector::GetVector(A.start, A.end);
    Vec3 Q = Vector::GetVector(B.start, B.end);
    Vec3 C = Vector::Cross(P, Q);
    Color color = MathUtils::TweenColor(A.color, B.color, 0.35);
    string nameA = VectorName(a);
    string nameB = VectorName(b);
    AddVector({0, 0, 0}, C, color, nameA, nameB, Vector::Dot(P, Q));
    return color;
  }

  size_t NumVectors() {
    return vectors.size();
  }

  size_t NumNonDeletedVectors() {
    size_t num = 0;
    for (size_t i = 0; i < vectors.size(); ++i) {
      if (deletedVectors.find(i) == deletedVectors.end()) {
        ++num;
      }
    }
    return num;
  }

  bool IsVectorDeleted(size_t index) {
    return deletedVectors.find(index) != deletedVectors.end();
  }

  GraphicsVector VectorAt(size_t index) {
    return vectors.at(index);
  }

  // Vectors are named A, B, ..., Z, AA, BB, ..., ZZ, AAA, etc.
  string VectorName(size_t index) {
    int numLetters = 26;
    int capitalA = 65;
    int letter = index % numLetters;
    int count = (int)((index - letter)/numLetters);
    char c = (char)(letter + capitalA);
    string str = "";
    for (int i = 0; i <= count; ++i) {
      str += string(1, c);
    }
    return str;
  }

  void SetVectorVisibility(int state, size_t index) {
    if (!state) {
      disabledVectors.emplace(index);
    } else {
      auto I = disabledVectors.find(index);
      if (I != disabledVectors.end()) {
        disabledVectors.erase(I);
      }
    }
    SetBoundingBox();
  }

  void SetVectorNormalized(int state, size_t index) {
    if (state) {
      normalizedVectors.emplace(index);
    } else {
      auto I = normalizedVectors.find(index);
      if (I != normalizedVectors.end()) {
        normalizedVectors.erase(I);
      }
    }
  }

  void SetVectorOrigin(int state, size_t index) {
    if (state) {
      originVectors.emplace(index);
    } else {
      auto I = originVectors.find(index);
      if (I != originVectors.end()) {
        originVectors.erase(I);
      }
    }
  }

  void DeleteVector(size_t index) {
    deletedVectors.emplace(index);
  }

  void SetFunctions(Expr *xF, Expr *yF, Expr *zF, float min, float max, int numVectors) {
    debuggedStrings.clear();
    xFunc = xF;
    yFunc = yF;
    zFunc = zF;
    tMin = min;
    tMax = max;
    minArrLen = numeric_limits<float>::max();
    maxArrLen = numeric_limits<float>::min();
    funcPoints.clear();
    arrowPoints.clear();
    tStep = MathUtils::Abs(tMax - tMin) / (2 * numVectors);
    Debug("tStep: " + Precision(tStep));

    funcBox.min = {-coordSystemLimit, -coordSystemLimit, -coordSystemLimit};
    funcBox.max = {coordSystemLimit, coordSystemLimit, coordSystemLimit};
    vector<float> nums = {coordSystemLimit};
    vector<float> lens = {};

    currTMaxIndex = 0;
    int tIndex = 0;
    for (float t = tMin; t <= tMax + 0.005; t += tStep) {
      float x = xF->Eval(t, 0, 0);
      float y = yF->Eval(t, 0, 0);
      float z = zF->Eval(t, 0, 0);
      nums.push_back(x);
      nums.push_back(y);
      nums.push_back(z);
      funcPoints[tIndex] = {x, y, z};
      // Only draw an arrow at every other point
      if (tIndex % 2 == 0) {
        arrowPoints[tIndex] = {x, y, z};
        lens.push_back(Vector::Length({x, y, z}));
      }
      ++tIndex;
    }

    float maxValue = MathUtils::Max(nums);
    float minValue = MathUtils::Min(nums);
    Debug("maxValue initially: " + Precision(maxValue));
    maxValue = MathUtils::Max({MathUtils::Abs(maxValue), MathUtils::Abs(minValue)});
    float maxValueBeforeRound = maxValue;
    Debug("maxValue after accounting for minValue: " + Precision(maxValue));
    maxValue = round((maxValue * 4.0))/4.0;
    Debug("maxValue after rounding to nearest 0.25: " + Precision(maxValue));
    if (maxValue < maxValueBeforeRound) {
      maxValue += 0.25;
      Debug("maxValue after adding 0.25: " + Precision(maxValue));
    }
    funcBox.min = {-maxValue, -maxValue, -maxValue};
    funcBox.max = {maxValue, maxValue, maxValue};

    maxArrLen = MathUtils::Max(lens);
    minArrLen = MathUtils::Min(lens);
    Debug("minArrLen: " + Precision(minArrLen) + ", maxArrLen: " + Precision(maxArrLen));

    funcTime = 0;
  }

  void SetVectorField(VectorField *field) {
    vectorField = field;
    curl = field->Curl();
    float x = rangeVF.x;
    float y = rangeVF.y;
    float z = rangeVF.z;
    // Debug("Getting min and max lengths on range " + Precision(x, 1) + ", " + Precision(y, 1) + ", " + Precision(z, 1));
    field->MinMaxLengths(
      x, y, z,
      coordSystemGridSize,
      minVectorFieldLength,
      maxVectorFieldLength
    );
    curl->MinMaxLengths(
      x, y, z,
      coordSystemGridSize,
      minCurlLength,
      maxCurlLength
    );
    // Debug("minVectorFieldLength: " + Precision(minVectorFieldLength, 3) + ", maxVectorFieldLength: " + Precision(maxVectorFieldLength, 3));
    // Debug("minCurlLength: " + Precision(minCurlLength, 3) + ", maxCurlLength: " + Precision(maxCurlLength, 3));
  }

  VectorField *Curl() {
    return curl;
  }

  float MinVectorFieldLength() {
    return minVectorFieldLength;
  }

  float MaxVectorFieldLength() {
    return maxVectorFieldLength;
  }

  float MinCurlLength() {
    return minCurlLength;
  }

  float MaxCurlLength() {
    return maxCurlLength;
  }

  void SetCameraPosition(float x, float y, float z) {
    cameraPos = {x, y, z};
  }

  void SetOrbit(bool orbit) {
    orbitCamera = orbit;
    showZMarkers = true;
    showZFuncMarkers = true;
  }

  void LookAtOriginVectors() {
    SetCameraPosition(0, 0, 5);
    cameraTime = 0.0f;
    showZMarkers = false;
  }

  void ResetCameraVectors() {
    // SetCameraPosition(1.5, 1.5, 5.3);
    SetCameraPosition(0, 0, 5);
    cameraTime = 0.0f;
    // showZMarkers = true;
    showZMarkers = false;
  }

  void ResetCameraFunc() {
    SetCameraPosition(0, 0, 5);
    cameraTime = 0.0f;
    showZFuncMarkers = false;
  }

  void ResetCameraVectorField() {
    SetCameraPosition(0, 0, 5);
    cameraTime = 0.0f;
  }

  void SetViewField(int state) {
    viewField = state;
  }

  void SetViewCurl(int state) {
    viewCurl = state;
  }

protected:
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();
  void mousePressEvent(QMouseEvent * event);
  void mouseMoveEvent(QMouseEvent * event);
  void mouseReleaseEvent(QMouseEvent * event);
  void wheelEvent(QWheelEvent *event);

private:
  set<string> debuggedStrings;
  QTextBrowser *debug;
  float time;

  // Which kind of thing to render
  GraphicsMode mode;

  // General rendering properties
  Color backgroundColor;
  GLUquadric *quadric = gluNewQuadric();
  bool IsMousePressed = false;

  // Camera properties
  Vec3 cameraPos;
  bool orbitCamera;
  float cameraTime;

  // Coordinate system properties
  float coordSystemGridSize = 0.5f;
  float coordSystemLimit = 4 * coordSystemGridSize;
  float coordSystemZ = 5 * coordSystemGridSize;

  // Vector properties
  vector<GraphicsVector> vectors;
  set<size_t> disabledVectors;
  set<size_t> normalizedVectors;
  set<size_t> originVectors;
  set<size_t> deletedVectors;
  BoundingBox box;
  bool showZMarkers;

  // Function properties
  Expr *xFunc;
  Expr *yFunc;
  Expr *zFunc;
  float tMin;
  float tMax;
  float minArrLen;
  float maxArrLen;
  int currTMaxIndex;
  map<int, Vec3> funcPoints;
  map<int, Vec3> arrowPoints;
  float tStep;
  BoundingBox funcBox;
  int funcTime;
  bool showZFuncMarkers;

  // Vector field properties
  VectorField *vectorField;
  VectorField *curl;
  float minVectorFieldLength;
  float maxVectorFieldLength;
  float minCurlLength;
  float maxCurlLength;
  bool viewField;
  bool viewCurl;
  string rangevf_X;
  string rangevf_Y;
  string rangevf_Z;
  struct Vec3 rangeVF;

  void Debug(string str) {
    if (debug) {
      if (debuggedStrings.find(str) == debuggedStrings.end()) {
        debuggedStrings.emplace(str);
        debug->append(StringUtils::Fancy(str));
      }
    }
  }

  // Coordinate systems
  void CoordinateSystem();
  void CoordinateSystemFunc();
  void CoordinateSystemVF();

  // Vectors
  void Vectors();
  void SetBoundingBox();

  // Function
  void Function(Expr *xF, Expr *yF, Expr *zF, int tMaxIndex);

  // Vector field
  void Field(VectorField *field, Color A, Color B, float minLength, float maxLength);

  // General drawing helpers
  void Arrow(struct Vec3 start, struct Vec3 end, Color color, float thickness, float maxConeRadius = 0.07f);
  void Line(struct Vec3 start, struct Vec3 end, Color color, float thickness);
  void Sphere(struct Vec3 point, Color color, float radius);

  // Camera
  void UpdateCameraVF();

  // Mouse
  void Mouse(int button, int state, int x, int y);
  void PassiveMotion(int x, int y);
};

#endif // OGLWIDGET_H
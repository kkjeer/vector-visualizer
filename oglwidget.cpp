#include "oglwidget.h"
#include <math.h>
#include <cmath>

using namespace Expression;

float SinColor(float time) {
  return 0.5f * sin(time) + 0.5f;
}

OGLWidget::OGLWidget(QWidget *parent) : QOpenGLWidget(parent), time(0.5f) {
  // Set the minimum size of this widget so it doesn't get squished (invisible) in nested layouts.
  // Actually this doesn't seem to be necessary with the size policy set.
  // setMinimumSize(400, 500);
  // Set the size policy so it can never be less than its minimum size, but can expand.
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

  time = 0.0f;
  mode = GraphicsMode::Vectors;

  // Initialize rendering properties.
  backgroundColor = {0, 0.5, 1, 1};

  // Initialize camera properties.
  ResetCameraVectors();
  orbitCamera = false;
  cameraTime = 0.0f;

  // Initialize vector properties.
  box.min = {-coordSystemLimit, -coordSystemLimit, -coordSystemLimit};
  box.max = {coordSystemLimit, coordSystemLimit, coordSystemLimit};

  // Initialize function properties.
  xFunc = nullptr;
  yFunc = nullptr;
  zFunc = nullptr;
  tMin = 0.0f;
  tMax = 1.0f;
  currTMaxIndex = 0;
  funcBox.min = {-coordSystemLimit, -coordSystemLimit, -coordSystemLimit};
  funcBox.max = {coordSystemLimit, coordSystemLimit, coordSystemLimit};
  funcTime = 0;
  showZFuncMarkers = false;

  // Initialize vector field properties.
  rangevf_X = "10";
  rangevf_Y = "10";
  rangevf_Z = "5";
  rangeVF = {stof(rangevf_X), stof(rangevf_Y), stof(rangevf_Z)};
  vectorField = nullptr;

  // Run the QWidget::update function on an interval.
  // This ensures that paintGL is run on an interval so the graphics actually update.
  QTimer *timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  timer->start(10);

  setMouseTracking(true);
}

OGLWidget::~OGLWidget() {
  debuggedStrings.clear();

  gluDeleteQuadric(quadric);

  vectors.clear();
  disabledVectors.clear();
  normalizedVectors.clear();
  deletedVectors.clear();

  // delete xFunc;
  // delete yFunc;
  // delete zFunc;
  funcPoints.clear();
  arrowPoints.clear();

  // delete vectorField;
  // delete curl;
}

void OGLWidget::mousePressEvent(QMouseEvent * event) {
  int xAtPress = event->x();
  int yAtPress = event->y();
  Debug("xAtPress: " + TrimZeroes(xAtPress));
  Debug("yAtPress: " + TrimZeroes(yAtPress));
  IsMousePressed = true;
}

void OGLWidget::mouseMoveEvent(QMouseEvent * event) {
  if (!IsMousePressed)
    return;
  int xAtMove = event->x();
  int yAtMove = event->y();
  Debug("xAtMove: " + TrimZeroes(xAtMove));
  Debug("yAtMove: " + TrimZeroes(yAtMove));
  if (event->buttons() == Qt::RightButton) {
    Debug("Only right button");
  } else if (event->buttons() == Qt::LeftButton) {
    Debug("Only left button");
  }
}

void OGLWidget::mouseReleaseEvent(QMouseEvent * event) {
  int xAtRelease = event->x();
  int yAtRelease = event->y();
  Debug("xAtRelease: " + TrimZeroes(xAtRelease));
  Debug("yAtRelease: " + TrimZeroes(yAtRelease));
  IsMousePressed = false;
}

void OGLWidget::wheelEvent(QWheelEvent *event) {
  int xAtScroll = event->angleDelta().x();
  int yAtScroll = event->angleDelta().y();
  Debug("xAtScroll: " + to_string(xAtScroll));
  Debug("yAtScroll: " + to_string(yAtScroll));
}

void OGLWidget::initializeGL() {
  // Set the background color
  glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
}

void OGLWidget::paintGL() {
  time += 0.025f;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);


  if (mode == GraphicsMode::Vectors) {
    // Render vectors
    if (orbitCamera) {
      // SetCameraPosition(1.5 * cos(0.2 * (cameraTime)), 0.75 * sin(0.2 * cameraTime) + 1.5, 5.3);
      SetCameraPosition(1.2 * sin(0.2 * (cameraTime)), 1.2 * cos(0.2 * cameraTime), 5);
      cameraTime += 0.025f;
    }
    UpdateCameraVF();

    CoordinateSystem();
    Vectors();
  } else if (mode == GraphicsMode::Function) {
    // Render vector-valued function
    if (orbitCamera) {
      SetCameraPosition(1.2 * sin(0.2 * cameraTime), 1.2 * cos(0.2 * cameraTime), 5);
      cameraTime += 0.025f;
    }
    UpdateCameraVF();

    CoordinateSystemFunc();
    if (xFunc && yFunc && zFunc) {
      if (funcTime % 15 == 0) {
        currTMaxIndex += 2;
      }
      Function(xFunc, yFunc, zFunc, currTMaxIndex);
      funcTime += 1;
    }
  } else {
    // Render vector field
    if (orbitCamera) {
      SetCameraPosition(sin(0.2 * cameraTime), cos(0.2 * cameraTime), 5);
      cameraTime += 0.025f;
    }
    UpdateCameraVF();

    CoordinateSystemVF();
    if (viewField) {
      Color A = {0.0, 0.1, 0.8, 1.0};
      Color B = {0.0, 0.7, 1.0, 1.0};
      Field(vectorField, A, B, minVectorFieldLength, maxVectorFieldLength);
    }
    if (viewCurl) {
      Color C = {0.8, 0.0, 0.0, 1.0};
      Color D = {1.0, 0.65, 0.0, 1.0};
      Field(curl, C, D, minCurlLength, maxCurlLength);
    }
  }
}

void OGLWidget::CoordinateSystem() {
  float min = -3.5;
  float max = 3.0;
  float zRange = 5.0;
  Color axis = {0, 0, 0, 1};
  Color grid = {0.6, 0.6, 0.6, 1};
  float thickness = 3.0;
  float lineHalfSize = 0.07f * coordSystemGridSize;

  // x-axis
  Line({min, 0, 0}, {max, 0, 0}, axis, thickness);
  // y-axis
  Line({0, min, 0}, {0, max, 0}, axis, thickness);
  // z-axis
  Line({0, 0, -zRange}, {0, 0, zRange}, axis, thickness);

  // Lines along the x-axis
  for (float x = min; x <= max; x += coordSystemGridSize) {
    Line({x, -lineHalfSize, 0}, {x, lineHalfSize, 0}, axis, thickness);
  }
  // Lines along the y-axis
  for (float y = min; y <= max; y += coordSystemGridSize) {
    Line({-lineHalfSize, y, 0}, {lineHalfSize, y, 0}, axis, thickness);
  }
  // Lines along the z-axis
  float zGridSize = coordSystemZ/(coordSystemLimit/coordSystemGridSize);
  for (float z = 0; z <= zRange; z += zGridSize) {
    Line({-lineHalfSize, 0, z}, {lineHalfSize, 0, z}, axis, thickness);
  }
  for (float z = -zGridSize; z >= -zRange; z -= zGridSize) {
    Line({-lineHalfSize, 0, z}, {lineHalfSize, 0, z}, axis, thickness);
  }

  // Vertical gray lines in the xy-plane
  for (float x = min; x <= max; x += coordSystemGridSize) {
    Line({x, min, 0}, {x, max, 0}, grid, thickness);
  }
  // Horizontal gray lines in the xy-plane
  for (float y = min; y <= max; y += coordSystemGridSize) {
    Line({min, y, 0}, {max, y, 0}, grid, thickness);
  }

  // X range markers
  string xMin = Precision(box.min.x, 2);
  string xMax = Precision(box.max.x, 2);
  NumberGraphic::DrawAxis('x', 1.92, -0.1, 0.01);
  NumberGraphic::DrawNumber(xMin, -1.97, 0.03, 0.01, NumberGraphic::Direction::LToR);
  NumberGraphic::DrawNumber(xMax, 2.0, 0.03, 0.01, NumberGraphic::Direction::RToL);
  // Y range markers
  string yMin = Precision(box.min.y, 2);
  string yMax = Precision(box.max.y, 2);
  NumberGraphic::DrawAxis('y', -0.07, 1.9, 0.01);
  NumberGraphic::DrawNumber(yMin, 0.02, -1.97, 0.01, NumberGraphic::Direction::LToR);
  NumberGraphic::DrawNumber(yMax, 0.02, 1.9, 0.01, NumberGraphic::Direction::LToR);
  // Z range markers
  if (showZMarkers) {
    string zMin = Precision(box.min.z, 2);
    string zMax = Precision(box.max.z, 2);
    NumberGraphic::DrawAxis('z', -0.1, 0.0, coordSystemZ);
    NumberGraphic::DrawNumber(zMin, 0.04, 0.0, -coordSystemZ, NumberGraphic::Direction::LToR);
    NumberGraphic::DrawNumber(zMax, 0.04, 0.0, coordSystemZ, NumberGraphic::Direction::LToR);
  }
}

void OGLWidget::CoordinateSystemFunc() {
  float min = -2.5;
  float max = 2.5;
  float zRange = 5.0;
  Color axis = {0, 0, 0, 1};
  Color grid = {0.6, 0.6, 0.6, 1};
  float thickness = 3.0;
  float lineHalfSize = 0.07f * coordSystemGridSize;

  // x-axis
  Line({min, 0, 0}, {max, 0, 0}, axis, thickness);
  // y-axis
  Line({0, min, 0}, {0, max, 0}, axis, thickness);
  // z-axis
  Line({0, 0, -5}, {0, 0, 5}, axis, thickness);

  // Lines along the x-axis
  for (float x = min; x <= max; x += coordSystemGridSize) {
    Line({x, -lineHalfSize, 0}, {x, lineHalfSize, 0}, axis, thickness);
  }
  // Lines along the y-axis
  for (float y = min; y <= max; y += coordSystemGridSize) {
    Line({-lineHalfSize, y, 0}, {lineHalfSize, y, 0}, axis, thickness);
  }
  // Lines along the z-axis
  float zGridSize = coordSystemZ/(coordSystemLimit/coordSystemGridSize);
  for (float z = 0; z <= zRange; z += zGridSize) {
    Line({-lineHalfSize, 0, z}, {lineHalfSize, 0, z}, axis, thickness);
  }
  for (float z = -zGridSize; z >= -zRange; z -= zGridSize) {
    Line({-lineHalfSize, 0, z}, {lineHalfSize, 0, z}, axis, thickness);
  }

  // Vertical gray lines in the xy-plane
  for (float x = min; x <= max; x += coordSystemGridSize) {
    Line({x, min, 0}, {x, max, 0}, grid, thickness);
  }
  // Horizontal gray lines in the xy-plane
  for (float y = min; y <= max; y += coordSystemGridSize) {
    Line({min, y, 0}, {max, y, 0}, grid, thickness);
  }

  // X range markers
  string xMin = Precision(funcBox.min.x, 2);
  string xMax = Precision(funcBox.max.x, 2);
  NumberGraphic::DrawAxis('x', 1.92, -0.1, 0.01);
  NumberGraphic::DrawNumber(xMin, -1.97, 0.03, 0.01, NumberGraphic::Direction::LToR);
  NumberGraphic::DrawNumber(xMax, 2.0, 0.03, 0.01, NumberGraphic::Direction::RToL);
  // Y range markers
  string yMin = Precision(funcBox.min.y, 2);
  string yMax = Precision(funcBox.max.y, 2);
  NumberGraphic::DrawAxis('y', -0.07, 1.9, 0.01);
  NumberGraphic::DrawNumber(yMin, 0.02, -1.97, 0.01, NumberGraphic::Direction::LToR);
  NumberGraphic::DrawNumber(yMax, 0.02, 1.9, 0.01, NumberGraphic::Direction::LToR);
  // Z range markers
  if (showZFuncMarkers) {
    string zMin = Precision(funcBox.min.z, 2);
    string zMax = Precision(funcBox.max.z, 2);
    NumberGraphic::DrawAxis('z', -0.1, 0.0, coordSystemZ);
    NumberGraphic::DrawNumber(zMin, 0.04, 0.0, -coordSystemZ, NumberGraphic::Direction::LToR);
    NumberGraphic::DrawNumber(zMax, 0.04, 0.0, coordSystemZ, NumberGraphic::Direction::LToR);
  }
}

void OGLWidget::CoordinateSystemVF() {
  float min = -2.5;
  float max = 2.5;
  Color axis = {0, 0, 0, 1};
  Color grid = {0.6, 0.6, 0.6, 1};
  float thickness = 3.0;

  // x-axis
  Line({min, 0, 0}, {max, 0, 0}, axis, thickness);
  // y-axis
  Line({0, min, 0}, {0, max, 0}, axis, thickness);
  // z-axis
  Line({0, 0, -5}, {0, 0, 5}, axis, thickness);

  // Vertical lines along the x-axis
  for (float x = min; x <= max; x += coordSystemGridSize) {
    Line({x, min, 0}, {x, max, 0}, grid, thickness);
  }
  // Horizontal lines along the y-axis
  for (float y = min; y <= max; y += coordSystemGridSize) {
    Line({min, y, 0}, {max, y, 0}, grid, thickness);
  }

  // Vertical gray lines in the xy-plane
  for (float x = min; x <= max; x += coordSystemGridSize) {
    Line({x, min, 0}, {x, max, 0}, grid, thickness);
  }
  // Horizontal gray lines in the xy-plane
  for (float y = min; y <= max; y += coordSystemGridSize) {
    Line({min, y, 0}, {max, y, 0}, grid, thickness);
  }

  // X range markers
  NumberGraphic::DrawAxis('x', 1.92, -0.1, 0.01);
  NumberGraphic::DrawNumber("-" + rangevf_X, -1.97, 0.03, 0.01, NumberGraphic::Direction::LToR);
  NumberGraphic::DrawNumber(rangevf_X, 2.0, 0.03, 0.01, NumberGraphic::Direction::RToL);
  // Y range markers
  NumberGraphic::DrawAxis('y', -0.07, 1.9, 0.01);
  NumberGraphic::DrawNumber("-" + rangevf_Y, 0.02, -1.97, 0.01, NumberGraphic::Direction::LToR);
  NumberGraphic::DrawNumber(rangevf_Y, 0.02, 1.9, 0.01, NumberGraphic::Direction::LToR);
}

void OGLWidget::Vectors() {
  Vec2 fromX = {box.min.x, box.max.x};
  Vec2 fromY = {box.min.y, box.max.y};
  Vec2 fromZ = {box.min.z, box.max.z};
  Vec2 toXY = {-coordSystemLimit, coordSystemLimit};
  Vec2 toZ = {-coordSystemZ, coordSystemZ};
  for (size_t i = 0; i < vectors.size(); ++i) {
    // Do not render deleted vectors
    if (deletedVectors.find(i) != deletedVectors.end()) {
      continue;
    }

    GraphicsVector v = vectors.at(i);

    // Render the actual vector if it is not disabled
    if (disabledVectors.find(i) == disabledVectors.end()) {
      float xStart = MathUtils::MapToRange(v.start.x, fromX, toXY);
      float yStart = MathUtils::MapToRange(v.start.y, fromY, toXY);
      float zStart = MathUtils::MapToRange(v.start.z, fromZ, toZ);

      float xEnd = MathUtils::MapToRange(v.end.x, fromX, toXY);
      float yEnd = MathUtils::MapToRange(v.end.y, fromY, toXY);
      float zEnd = MathUtils::MapToRange(v.end.z, fromZ, toZ);

      Vec3 start = {xStart, yStart, zStart};
      Vec3 end = {xEnd, yEnd, zEnd};
      float len = Vector::Length(start, end);

      // VERY IMPORTANT: draw the dots BEFORE drawing the arrow.
      // This prevents the dots from doing weird things to the rest of the colors.
      Color dotColor = MathUtils::TweenColor(v.color, {0.1, 0.1, 0.1}, 0.5);
      Sphere(start, dotColor, 0.03);
      Sphere(end, dotColor, 0.03);

      Arrow(start, end, v.color, 10.0 * len);
    }

    // Render the normalized vector if it is set to draw the normalized version
    auto N = normalizedVectors.find(i);
    if (N != normalizedVectors.end()) {
      GraphicsVector n = vectors.at(*N);
      Vec3 normalized = Vector::Normalize(Vector::GetVector(n.start, n.end));
      Vec3 nStart = n.start;
      Vec3 nEnd = {n.start.x + normalized.x, n.start.y + normalized.y, n.start.z + normalized.z};

      float x1 = MathUtils::MapToRange(nStart.x, fromX, toXY);
      float y1 = MathUtils::MapToRange(nStart.y, fromX, toXY);
      float z1 = MathUtils::MapToRange(nStart.z, fromZ, toZ);

      float x2 = MathUtils::MapToRange(nEnd.x, fromX, toXY);
      float y2 = MathUtils::MapToRange(nEnd.y, fromY, toXY);
      float z2 = MathUtils::MapToRange(nEnd.z, fromZ, toZ);

      Vec3 s = {x1, y1, z1};
      Vec3 e = {x2, y2, z2};
      float len = Vector::Length(s, e);
      Color color = MathUtils::TweenColor(v.color, {0.9, 0.9, 0.9}, 0.4);
      Arrow(s, e, color, 8.0 * len);
    }

    // Render the vector start at the origin if it is set to draw the origin version
    auto O = originVectors.find(i);
    if (O != originVectors.end()) {
      GraphicsVector o = vectors.at(*O);
      Vec3 origin = Vector::GetVector(o.start, o.end);
      
      float x2 = MathUtils::MapToRange(origin.x, fromX, toXY);
      float y2 = MathUtils::MapToRange(origin.y, fromY, toXY);
      float z2 = MathUtils::MapToRange(origin.z, fromZ, toZ);

      Vec3 s = {0, 0, 0};
      Vec3 e = {x2, y2, z2};
      float len = Vector::Length(s, e);
      Color color = MathUtils::TweenColor(v.color, {0.9, 0.9, 0.9}, 0.5);
      Arrow(s, e, color, 8.0 * len);
    }
  }
}

void OGLWidget::SetBoundingBox() {
  Debug("Setting bounding box for " + to_string(vectors.size()) + " vectors");
  vector<float> nums = {coordSystemLimit};
  for (size_t index = 0; index < vectors.size(); ++index) {
    if (deletedVectors.find(index) != deletedVectors.end()) {
      continue;
    }
    if (disabledVectors.find(index) != disabledVectors.end()) {
      continue;
    }
    GraphicsVector v = vectors.at(index);
    nums.push_back(v.start.x);
    nums.push_back(v.start.y);
    nums.push_back(v.start.z);
    nums.push_back(v.end.x);
    nums.push_back(v.end.y);
    nums.push_back(v.end.z);
  }

  float max = MathUtils::Max(nums);
  float min = MathUtils::Min(nums);
  max = MathUtils::Max({MathUtils::Abs(max), MathUtils::Abs(min)});
  max = ceil((max * 4.0))/4.0;
  box.min = {-max, -max, -max};
  box.max = {max, max, max};
}

void OGLWidget::Function(Expr *xF, Expr *yF, Expr *zF, int tMaxIndex) {
  if (!xF || !yF || !zF)
    return;

  Vec2 fromX = {funcBox.min.x, funcBox.max.x};
  Vec2 fromY = {funcBox.min.y, funcBox.max.y};
  Vec2 fromZ = {funcBox.min.z, funcBox.max.z};
  Vec2 toXY = {-coordSystemLimit, coordSystemLimit};
  Vec2 toZ = {-coordSystemZ, coordSystemZ};

  auto scale = [fromX, fromY, fromZ, toXY, toZ](Vec3 v) {
    float x = MathUtils::MapToRange(v.x, fromX, toXY);
    float y = MathUtils::MapToRange(v.y, fromY, toXY);
    float z = MathUtils::MapToRange(v.z, fromZ, toZ);
    Vec3 result = {x, y, z};
    return result;
  };

  Color funcColor = {0, 0.3, 1, 1};
  Color arrowA = {0, 0.4, 0.5, 1};
  Color arrowB = {0, 0.7, 0.8, 1};

  auto drawArrow = [this, scale, arrowA, arrowB](int index) {
    auto It = arrowPoints.find(index);
    if (It != arrowPoints.end()) {
      Vec3 arrow = scale(It->second);
      float len = Vector::Length(It->second);
      float percentLen = 1.0f;
      if (maxArrLen - minArrLen > 0.0001) {
        percentLen = (len - minArrLen) / (maxArrLen - minArrLen);
      }
      Color c = MathUtils::TweenColor(arrowA, arrowB, percentLen);
      Arrow({0, 0, 0}, arrow, c, 3.0f, 0.05f);
    }
  };

  // Draw the arrow to the start point of the function
  drawArrow(0);

  // Draw line segements and arrows up to tMaxIndex
  for (int i = 2; i <= tMaxIndex && i <= funcPoints.size() - 1; ++i) {
    // Draw two line segments that lead up to i (i should have an arrow drawn to it)
    Vec3 start = scale(funcPoints[i - 2]);
    Vec3 mid = scale(funcPoints[i - 1]);
    Vec3 end = scale(funcPoints[i]);
    Line(start, mid, funcColor, 10.0f);
    Line(mid, end, funcColor, 10.0f);
    // Draw the arrow at i
    drawArrow(i);
  }
}

void OGLWidget::Field(VectorField *field, Color A, Color B, float minLength, float maxLength) {
  if (!field)
    return;

  const float maxRenderedLength = 0.3f;
  const float minRenderedLength = 0.05f;

  float xRange = rangeVF.x;
  float yRange = rangeVF.y;
  float zRange = rangeVF.z;

  float xRenderedRange = 3 * coordSystemGridSize;
  float yRenderedRange = 3 * coordSystemGridSize;
  float zRenderedRange = 1.75 * coordSystemGridSize;
  float zStep = zRenderedRange;

  Vec2 fromLen = { minLength, maxLength };
  Vec2 toLen = { minRenderedLength, maxRenderedLength };

  Vec2 toRed = { A.r, B.r };
  Vec2 toGreen = { A.g, B.g };
  Vec2 toBlue = { A.b, B.b };

  Debug("mapping from length range " + Precision(fromLen.x) + ", " + Precision(fromLen.y));
  Debug("mapping to length range " + Precision(toLen.x) + ", " + Precision(toLen.y));
  Debug("range size: " + Precision(MathUtils::Abs(fromLen.y - fromLen.x), 6));

  for (float x = -xRenderedRange; x <= xRenderedRange; x += coordSystemGridSize) {
    for (float y = -yRenderedRange; y <= yRenderedRange; y += coordSystemGridSize) {
      for (float z = -zRenderedRange; z <= zRenderedRange; z += zStep) {
        Vec3 start = {x, y, z};

        float evalX = (x / xRenderedRange) * xRange;
        float evalY = (y / yRenderedRange) * yRange;
        float evalZ = (z / zRenderedRange) * zRange;
        evalX = MathUtils::MapToRange(x, {-xRenderedRange, xRenderedRange}, {-xRange, xRange});
        evalY = MathUtils::MapToRange(y, {-yRenderedRange, yRenderedRange}, {-yRange, yRange});
        evalZ = MathUtils::MapToRange(z, {-zRenderedRange, zRenderedRange}, {-zRange, zRange});
        Vec3 point = field->Eval(evalX, evalY, evalZ);
        float len = Vector::Length(point);
        float renderedLength = MathUtils::MapToRange(len, fromLen, toLen);
        // Debug("len: " + Precision(len, 1) + ", renderedLength: " + Precision(renderedLength));
        Vec3 normalizedPoint = Vector::SetLength(point, renderedLength);
        Vec3 end;
        end.x = x + normalizedPoint.x;
        end.y = y + normalizedPoint.y;
        end.z = z + normalizedPoint.z;

        Color renderedColor;
        renderedColor.r = MathUtils::MapToRange(len, fromLen, toRed);
        renderedColor.g = MathUtils::MapToRange(len, fromLen, toGreen);
        renderedColor.b = MathUtils::MapToRange(len, fromLen, toBlue);
        renderedColor.a = 1;
        Arrow(start, end, renderedColor, 2.0);
      }
    }
  }
}

void OGLWidget::Arrow(struct Vec3 start, struct Vec3 end, Color color, float thickness, float maxConeRadius) {
  // Debug("\nDrawing arrow from start = " + StringUtils::Vec3String(start) + " to end = " + StringUtils::Vec3String(end));
  // Start drawing.
  glPushMatrix();

  // Dimensions based on vector length.
  float vecLen = Vector::Length(start, end);
  float coneLen = MathUtils::Clamp(0.32 * vecLen, 0.05, 0.22);
  float scaledVecLen = vecLen - coneLen;
  // Debug("vecLen: " + Precision(vecLen) + ", coneLen: " + Precision(coneLen) + ", " + "scaledVecLen: " + Precision(scaledVecLen));
  Vec3 scaledVec = Vector::SetLength(Vector::GetVector(start, end), scaledVecLen);
  Vec3 scaledEnd = {start.x + scaledVec.x, start.y + scaledVec.y, start.z + scaledVec.z};
  // Debug("scaledEnd = " + StringUtils::Vec3String(scaledEnd));
  float radius = MathUtils::Clamp(0.32 * coneLen, 0.02, maxConeRadius);

  // Draw the line from start to scaledEnd.
  Line(start, scaledEnd, color, thickness);

  // Determine the rotation angle and axis to rotate the cone so it is rotated along the line.
  Vec3 cylinderUp = { 0, 0, 1 };
  Vec3 vector = Vector::GetVector(start, scaledEnd);
  Vec3 cross = Vector::Cross(cylinderUp, vector);
  // It's possible that the cross product could be the zero vector
  // (e.g. vector, like cylinderUp, lies along the z-axis). If this happens,
  // adjust the cross vector so that it's a nonzero vector.
  if (Vector::Length(cross) <= 0.0000001) {
    cross = cylinderUp;
  }
  Vec3 axis = Vector::Normalize(cross);
  // The angle argument to acos must be in the range [-1, 1].
  // Normalize the vector to try to keep the dot product small.
  // If necessary, adjust dot to be in the range [-1, 1].
  float dot = Vector::Dot(cylinderUp, Vector::Normalize(vector));
  if (dot < -1) {
    dot = M_PI;
  } else if (dot > 1) {
    dot = 0.0;
  }
  float radians = acos(dot);
  float degrees = radians * 180.0f / M_PI;
  // Debug("rotating along axis " + StringUtils::Vec3String(axis) + " by " + Precision(degrees) + " degrees");

  // Place the cone at the end of the line and rotate it along the line.
  glTranslatef(scaledEnd.x, scaledEnd.y, scaledEnd.z);
  glRotatef(degrees, axis.x, axis.y, axis.z);

  // Draw a cone with a disk at its base to prevent weirdness with the global scene color (it can be dimmed otherwise).
  GLUquadric *quad = gluNewQuadric();
  gluCylinder(quad, /*bottomRadius=*/radius, /*topRadius=*/0.0f, /*height=*/coneLen, /*slices=*/32.0, /*stacks=*/32.0);
  gluDisk(quad, /*innerRadius=*/0.0f, /*outerRadius*/radius, /*slices=*/16, /*loops*/4);
  gluDeleteQuadric(quad);

  // Stop drawing.
  glPopMatrix();
}

void OGLWidget::Line(struct Vec3 start, struct Vec3 end, Color color, float thickness) {
  // Start drawing.
  glPushMatrix();

  // Set color and thickness.
  glColor4f(color.r, color.g, color.b, color.a);
  glLineWidth(thickness);

  // Draw the line from start to end.
  glBegin(GL_LINES);
    glVertex3f(start.x, start.y, start.z);
    glVertex3f(end.x, end.y, end.z);
  glEnd();

  // Stop drawing.
  glPopMatrix();
}

// This method does weird things to the rest of the colors in the scene
// (unless the dots are drawn before the arrow).
void OGLWidget::Sphere(struct Vec3 point, Color color, float radius) {
  glPushMatrix();
  glColor4f(color.r, color.g, color.b, color.a);
  glTranslatef(point.x, point.y, point.z);
  glutSolidSphere(/*radius*/0.03, /*slices*/16, /*stacks*/16);

  // For testing only: draw a torus
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(color.r, color.g, color.b, 0.8f);
  glutSolidTorus(/*inner radius*/0.03, /*outer radius*/0.16, /*nsides*/32, /*rings*/32);
  glDisable(GL_BLEND);

  glPopMatrix();
}

void OGLWidget::resizeGL(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(
    45, // field of view angle, in degrees, in the y-direction
    (float)w/h, // aspect ratio
    0.01, // distance from the viewer to the near clipping plane
    100.0 // distance from the viewer to the far clipping plane
  );
  UpdateCameraVF();
  return;
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z, // eye
            0, 0, 0, // at
            0, 1, 0); // up
}

void OGLWidget::UpdateCameraVF() {
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z, // eye
            0, 0, 0, // at
            0, 1, 0); // up
}

void OGLWidget::Mouse(int button, int state, int x, int y) {
  // TODO
}

void OGLWidget::PassiveMotion(int x, int y) {
  // TODO
}
#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QtWidgets>
#include <QMenuBar>
#include <QLabel>
#include <QLineEdit>
#include <QTabWidget>
#include <QEvent>
#include <QDoubleSpinBox>
#include "oglwidget.h"
#include <vector>

using namespace std;

class QPushButton;
class QTextBrowser;

class MainWidget : public QWidget {
  Q_OBJECT

public:
  explicit MainWidget(QWidget *parent = 0);
  ~MainWidget();
  virtual void changeEvent(QEvent *event);

private:
  void CreateMenuBar();
  QWidget *GetVectorControls();
  QWidget *GetFunctionControls();
  QWidget *GetVectorFieldControls();
  QWidget *MakeVectorWidget(size_t index);
  void ColorVectorWidgets();
  void ColorGraphicsWidget();
  QWidget *Line(Color left, Color right, float height);
  QWidget *SolidWidget(Color left, Color right, float width, float height);
  QWidget *SolidWidget(Color color, int width, int height);
  QIcon SolidIcon(Color color, int width, int height);

private slots:
  void toggleMinimized();
  void onChangeTab(int index);
  void onAddVector();
  void onCrossVectors();
  void onCreateFunction();
  void onCreateVectorField();
  void onOrbitCheckboxVectors(int state);
  void onResetCameraVectors();
  void onLookAtOriginVectors();
  void onOrbitCheckboxFunction(int state);
  void onResetCameraFunction();
  void onOrbitCheckboxVectorField(int state);
  void onResetCameraVectorField();
  void onChangeVectorVisibility(int state, size_t index);
  void onChangeVectorNormalize(int state, size_t index);
  void onChangeVectorOrigin(int state, size_t index);
  void onDeleteVector(size_t index);
  void onChangeFieldVisibility(int state);
  void onChangeCurlVisibility(int state);

private:
  // Controls for debugging
  QTextBrowser *textBrowser;
  QTextBrowser *funcDebug;
  QTextBrowser *vectorFieldOutput;

  // Menu action for minimizing/restoring the window
  QAction *minimizeAction;

  // Graphics widget to display all 3d graphics
  OGLWidget *oglWidget;

  // Camera controls
  QCheckBox *orbitCameraCheckboxVectors;
  QCheckBox *orbitCameraCheckboxFunction;
  QCheckBox *orbitCameraCheckboxVectorField;

  // Vectors things
  QDoubleSpinBox *startXSpin;
  QDoubleSpinBox *startYSpin;
  QDoubleSpinBox *startZSpin;
  QDoubleSpinBox *endXSpin;
  QDoubleSpinBox *endYSpin;
  QDoubleSpinBox *endZSpin;
  QWidget *crossControls;
  QComboBox *crossStart;
  QComboBox *crossEnd;
  QVBoxLayout *vectorsContainer;
  vector<QWidget *>vectorWidgets;

  // Function things
  QLineEdit *xEdit;
  QLineEdit *yEdit;
  QLineEdit *zEdit;
  QPushButton *funcButton;
  QDoubleSpinBox *tMinSpin;
  QDoubleSpinBox *tMaxSpin;
  QSlider *numVectorsSlider;
  QLabel *funcError;

  // VectorField things
  QLineEdit *iEdit;
  QLineEdit *jEdit;
  QLineEdit *kEdit;
  QPushButton *compileButton;
  QLabel *fieldError;
  QWidget *fieldDivider;
  QWidget *fieldColor;
  QCheckBox *fieldView;
  QLabel *fieldMessage;
  QLabel *minMaxMessage;
  QWidget *curlColor;
  QCheckBox *curlView;
  QLabel *curlMessage;
  QLabel *curlLenMessage;
};

#endif // MAINWIDGET_H
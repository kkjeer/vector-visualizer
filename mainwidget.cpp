#include <QKeySequence>
#include "mainwidget.h"
#include "Expr.h"
#include "Parsing/Parser.h"
#include "Utils/StringUtils.h"
#include "Utils/MathUtils.h"
#include "Parsing/Lexer.h"
#include "Parsing/ParserAlt.h"
#include <string>

using namespace Expression;
using namespace StringUtils;

bool DebugVectors = false;
bool DebugFunction = true;
bool DebugField = false;

// Constructor for main window
MainWidget::MainWidget(QWidget *parent) : QWidget(parent) {
  textBrowser = new QTextBrowser();
  oglWidget = new OGLWidget();
  oglWidget->SetDebug(textBrowser);
  ColorGraphicsWidget();

  orbitCameraCheckboxVectors = new QCheckBox("Orbit camera");
  orbitCameraCheckboxFunction = new QCheckBox("Orbit camera");
  orbitCameraCheckboxVectorField = new QCheckBox("Orbit camera");

  // Function things
  xEdit = new QLineEdit;
  yEdit = new QLineEdit;
  zEdit = new QLineEdit;
  funcDebug = new QTextBrowser;

  // Actual VectorField-relevant things
  iEdit = new QLineEdit;
  jEdit = new QLineEdit;
  kEdit = new QLineEdit;
  vectorFieldOutput = new QTextBrowser;

  CreateMenuBar();

  // Tabbed widget to contain all the right hand side controls
  QTabWidget *tabWidget = new QTabWidget;
  QWidget *vectorWidget = GetVectorControls();
  QWidget *functionWidget = GetFunctionControls();
  QWidget *vectorFieldWidget = GetVectorFieldControls();
  tabWidget->addTab(vectorWidget, "Vectors");
  tabWidget->addTab(functionWidget, "Function");
  tabWidget->addTab(vectorFieldWidget, "Vector Field");
  tabWidget->setMinimumSize(75, 100);
  tabWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);

  // Main horizontal box layout containing the graphics widget on the left and the controls on the right
  QHBoxLayout *hBoxLayout = new QHBoxLayout;
  // Both the tabWidget and the oglWidget should have a stretch so they both adjust their width when the window is resized.
  // hBoxLayout->addWidget(tabWidget, /*stretch=*/2);
  // Or, don't put a stretch on the tabWidget so the controls container always stays the same size
  hBoxLayout->addWidget(tabWidget);
  hBoxLayout->addWidget(oglWidget, /*stretch=*/3);

  setLayout(hBoxLayout);
  setWindowTitle(tr("Vector Visualizer"));

  startXSpin->setFocus(Qt::TabFocusReason);

  connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onChangeTab(int)));
}

// Destructor
MainWidget::~MainWidget() {
  // delete textBrowser;
  // delete funcDebug;
  // delete vectorFieldOutput;

  // delete minimizeAction;
  // delete oglWidget;

  // delete orbitCameraCheckboxVectors;
  // delete orbitCameraCheckboxFunction;
  // delete orbitCameraCheckboxVectorField;

  // delete startXSpin;
  // delete startYSpin;
  // delete startZSpin;
  // delete endXSpin;
  // delete endYSpin;
  // delete endZSpin;
  // delete crossControls;
  // delete vectorsContainer;

  // delete xEdit;
  // delete yEdit;
  // delete zEdit;
  // delete funcButton;
  // delete tMinSpin;
  // delete tMaxSpin;
  // delete numVectorsSlider;
  // delete funcError;

  // delete iEdit;
  // delete jEdit;
  // delete kEdit;
  // delete compileButton;
  // delete fieldError;
  // delete fieldDivider;
  // delete fieldColor;
  // delete fieldView;
  // delete fieldMessage;
  // delete minMaxMessage;
  // delete curlColor;
  // delete curlView;
  // delete curlMessage;
  // delete curlLenMessage;
}

void MainWidget::CreateMenuBar() {
  QMenuBar *menuBar = new QMenuBar;

  // QMenu *file = new QMenu("My Menu");
  // file->addMenu("&File");
  // menuBar->addMenu(file);

  // QMenu *testMenu = new QMenu("Test");
  // testMenu->addAction("My Action");
  // testMenu->addAction("Another Action");
  // menuBar->addMenu(testMenu);

  QMenu *windowMenu = new QMenu("Window");

  QAction *closeAction = windowMenu->addAction("Close");
  connect(closeAction, SIGNAL(triggered()), this, SLOT(close()));
  QKeySequence close(QKeySequence::Close);
  closeAction->setShortcut(close);

  minimizeAction = windowMenu->addAction("Minimize");
  connect(minimizeAction, SIGNAL(triggered()), this, SLOT(toggleMinimized()));
  QKeySequence minimize(tr("Ctrl+M"));
  minimizeAction->setShortcut(minimize);

  menuBar->addMenu(windowMenu);
}

QWidget *MainWidget::GetVectorControls() {
  QVBoxLayout *vectorLayout = new QVBoxLayout;
  vectorLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

  QLabel *header = new QLabel(Fancy("Create a new vector from " + Bold("Start") + " to " + Bold("End") + ":"));
  QFont font = header->font();
  font.setPixelSize(16);
  header->setFont(font);
  vectorLayout->addWidget(header);
  vectorLayout->addSpacing(10);

  // Set up spin boxes to set start and end points
  startXSpin = new QDoubleSpinBox, startYSpin = new QDoubleSpinBox, startZSpin = new QDoubleSpinBox;
  float min = -100.0, max = 100.0, step = 0.1;
  startXSpin->setRange(min, max);
  startXSpin->setSingleStep(step);
  startYSpin->setRange(min, max);
  startYSpin->setSingleStep(step);
  startZSpin->setRange(min, max);
  startZSpin->setSingleStep(step);
  endXSpin = new QDoubleSpinBox, endYSpin = new QDoubleSpinBox, endZSpin = new QDoubleSpinBox;
  endXSpin->setRange(min, max);
  endXSpin->setSingleStep(step);
  endYSpin->setRange(min, max);
  endYSpin->setSingleStep(step);
  endZSpin->setRange(min, max);
  endZSpin->setSingleStep(step);

  // Controls to edit the start point of the new vector
  QHBoxLayout *startLayout = new QHBoxLayout;
  QLabel *startLabel = new QLabel(Fancy(Bold("Start:")));
  startLabel->setFixedWidth(40);
  startLayout->addWidget(startLabel);
  startLayout->addWidget(new QLabel(Fancy(Bold(Italic("x") + " = "))));
  startLayout->addWidget(startXSpin);
  startLayout->addSpacing(10);
  startLayout->addWidget(new QLabel(Fancy(Bold(Italic("y") + " = "))));
  startLayout->addWidget(startYSpin);
  startLayout->addSpacing(10);
  startLayout->addWidget(new QLabel(Fancy(Bold(Italic("z") + " = "))));
  startLayout->addWidget(startZSpin);
  startLayout->addStretch();
  vectorLayout->addLayout(startLayout);

  // Controls to edit the end point of the new vector
  QHBoxLayout *endLayout = new QHBoxLayout;
  QLabel *endLabel = new QLabel(Fancy(Bold("End:")));
  endLabel->setFixedWidth(40);
  endLayout->addWidget(endLabel);
  endLayout->addWidget(new QLabel(Fancy(Bold(Italic("x") + " = "))));
  endLayout->addWidget(endXSpin);
  endLayout->addSpacing(10);
  endLayout->addWidget(new QLabel(Fancy(Bold(Italic("y") + " = "))));
  endLayout->addWidget(endYSpin);
  endLayout->addSpacing(10);
  endLayout->addWidget(new QLabel(Fancy(Bold(Italic("z") + " = "))));
  endLayout->addWidget(endZSpin);
  endLayout->addStretch();
  vectorLayout->addLayout(endLayout);

  // Button to add a new vector
  QHBoxLayout *addVecLayout = new QHBoxLayout;
  QPushButton *addVecButton = new QPushButton("Add vector");
  addVecLayout->addWidget(addVecButton);
  addVecLayout->addStretch();
  vectorLayout->addLayout(addVecLayout);

  // Controls to add the cross product of two existing vectors
  QHBoxLayout *crossVecs = new QHBoxLayout;
  crossVecs->addWidget(new QLabel(Fancy("Add cross product:")));
  crossStart = new QComboBox;
  crossVecs->addWidget(crossStart);
  crossVecs->addWidget(new QLabel(Fancy(Bold("X"))));
  crossEnd = new QComboBox;
  crossVecs->addWidget(crossEnd);
  QPushButton *cross = new QPushButton("Cross vectors");
  crossVecs->addWidget(cross);
  crossVecs->addStretch();
  crossVecs->setContentsMargins(0, 0, 0, 0);
  crossControls = new QWidget;
  crossControls->setLayout(crossVecs);
  crossControls->setVisible(false);
  vectorLayout->addWidget(crossControls);

  vectorLayout->addSpacing(10);

  // Graphics widget vectors controls
  QHBoxLayout *cameraControls = new QHBoxLayout;
  cameraControls->addWidget(orbitCameraCheckboxVectors);
  QPushButton *resetCameraButton = new QPushButton("Reset camera");
  cameraControls->addWidget(resetCameraButton);
  QPushButton *lookAtOriginButton = new QPushButton("Look at origin");
  // cameraControls->addWidget(lookAtOriginButton);
  cameraControls->addStretch();
  vectorLayout->addLayout(cameraControls);

  vectorLayout->addSpacing(10);

  // Container that will hold all the vectors.
  vectorsContainer = new QVBoxLayout;
  vectorsContainer->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  // vectorsContainer->setDirection(QBoxLayout::BottomToTop);
  vectorsContainer->setSpacing(0);
  vectorsContainer->setContentsMargins(0, 0, 0, 0);

  // Put the vectors in a scroll area.
  QWidget *vectorScrollContainment = new QWidget;
  vectorScrollContainment->setLayout(new QVBoxLayout);
  QWidget *vectorsContainerWidget = new QWidget;
  vectorsContainerWidget->setLayout(vectorsContainer);
  QScrollArea *vectorsScroll = new QScrollArea();
  vectorsScroll->setWidget(vectorsContainerWidget);
  vectorsScroll->setWidgetResizable(true);
  vectorScrollContainment->layout()->addWidget(vectorsScroll);
  vectorLayout->addWidget(vectorScrollContainment);

  // TODO: for debugging only.
  if (DebugVectors)
    vectorLayout->addWidget(textBrowser);

  // Connect vector controls to signals.
  connect(addVecButton, SIGNAL(released()), this, SLOT(onAddVector()));
  connect(cross, SIGNAL(released()), this, SLOT(onCrossVectors()));
  connect(orbitCameraCheckboxVectors, SIGNAL(stateChanged(int)), this, SLOT(onOrbitCheckboxVectors(int)));
  connect(resetCameraButton, SIGNAL(released()), this, SLOT(onResetCameraVectors()));
  connect(lookAtOriginButton, SIGNAL(released()), this, SLOT(onLookAtOriginVectors()));

  QWidget *vectorWidget = new QWidget;
  vectorWidget->setLayout(vectorLayout);
  return vectorWidget;
}

QWidget *MainWidget::GetFunctionControls() {
  // Vertical box to contain function controls
  QVBoxLayout *functionLayout = new QVBoxLayout;
  functionLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

  QLabel *header = new QLabel(Fancy("Enter parametric equations for " + Bold("x") + ", " + Bold("y") + ", and " + Bold("z") + " to create a vector-valued function:"));
  QFont font = header->font();
  font.setPixelSize(16);
  header->setFont(font);
  functionLayout->addWidget(header);
  functionLayout->addSpacing(10);

  // Controls to enter function equations
  QGridLayout *eqEditor = new QGridLayout;
  eqEditor->addWidget(new QLabel(Fancy(Eqn("x"))), 0, 0);
  eqEditor->addWidget(xEdit, 0, 1, 1, 8);
  eqEditor->addWidget(new QLabel(Fancy(Eqn("y"))), 1, 0);
  eqEditor->addWidget(yEdit, 1, 1, 1, 8);
  eqEditor->addWidget(new QLabel(Fancy(Eqn("z"))), 2, 0);
  eqEditor->addWidget(zEdit, 2, 1, 1, 8);
  functionLayout->addLayout(eqEditor);

  functionLayout->addSpacing(10);

  // Error message
  funcError = new QLabel;
  funcError->setVisible(false);
  functionLayout->addWidget(funcError);

  // Spin box controls to edit t range
  QHBoxLayout *tSpin = new QHBoxLayout;
  tSpin->addWidget(new QLabel(Fancy("Evaluate the function from " + Italic("t") + " = ")));
  tMinSpin = new QDoubleSpinBox;
  tMinSpin->setRange(-100.0, 100.0);
  tMinSpin->setSingleStep(0.1);
  tSpin->addWidget(tMinSpin);
  tSpin->addWidget(new QLabel("to"));
  tMaxSpin = new QDoubleSpinBox;
  tMaxSpin->setRange(-100.0, 100.0);
  tMaxSpin->setSingleStep(0.1);
  tMaxSpin->setValue(1.00);
  tSpin->addWidget(tMaxSpin);
  tSpin->addStretch();
  functionLayout->addLayout(tSpin);

  // Slider control to edit number of vectors
  QHBoxLayout *numVectors = new QHBoxLayout;
  numVectors->addWidget(new QLabel("Number of vectors to graph:"));
  numVectors->addWidget(new QLabel("4"));
  numVectorsSlider = new QSlider(Qt::Horizontal);
  numVectorsSlider->setTickPosition(QSlider::TicksBelow);
  numVectorsSlider->setMinimum(2);
  numVectorsSlider->setMaximum(10);
  numVectorsSlider->setTickInterval(1);
  numVectorsSlider->setSingleStep(1);
  numVectors->addWidget(numVectorsSlider);
  numVectors->addWidget(new QLabel("20"));
  functionLayout->addLayout(numVectors);

  functionLayout->addSpacing(10);

  // Button to graph the function
  QHBoxLayout *funcBtnLayout = new QHBoxLayout;
  funcButton = new QPushButton(tr("Graph function"));
  funcBtnLayout->addWidget(funcButton);
  funcBtnLayout->addStretch();
  functionLayout->addLayout(funcBtnLayout);

  functionLayout->addSpacing(10);

  // Graphics widget vectors controls
  QHBoxLayout *cameraControls = new QHBoxLayout;
  cameraControls->addWidget(orbitCameraCheckboxFunction);
  QPushButton *resetCameraButton = new QPushButton("Reset camera");
  cameraControls->addWidget(resetCameraButton);
  cameraControls->addStretch();
  functionLayout->addLayout(cameraControls);

  // TODO: for debugging only
  if (DebugFunction)
    functionLayout->addWidget(funcDebug);

  // Connect controls to slots
  connect(funcButton, SIGNAL(released()), this, SLOT(onCreateFunction()));
  connect(orbitCameraCheckboxFunction, SIGNAL(stateChanged(int)), this, SLOT(onOrbitCheckboxFunction(int)));
  connect(resetCameraButton, SIGNAL(released()), this, SLOT(onResetCameraFunction()));

  // Create a widget with the function layout that can be added to a tab widget
  QWidget *functionWidget = new QWidget;
  functionWidget->setLayout(functionLayout);
  return functionWidget;
}

QWidget *MainWidget::GetVectorFieldControls() {
  // Vertical box to contain vector field controls
  QVBoxLayout *vectorFieldLayout = new QVBoxLayout;
  vectorFieldLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

  QLabel *header = new QLabel(Fancy("Enter equations for the components of the vector field " + Bold("v") + ":"));
  QFont font = header->font();
  font.setPixelSize(16);
  header->setFont(font);
  vectorFieldLayout->addWidget(header);
  vectorFieldLayout->addSpacing(10);

  // Controls to enter vector field equations
  QGridLayout *eqEditor = new QGridLayout;
  eqEditor->addWidget(new QLabel(Fancy(Equation("i"))), 0, 0);
  eqEditor->addWidget(iEdit, 0, 1, 1, 7);
  eqEditor->addWidget(new QLabel(Fancy(Equation("j"))), 1, 0);
  eqEditor->addWidget(jEdit, 1, 1, 1, 7);
  eqEditor->addWidget(new QLabel(Fancy(Equation("k"))), 2, 0);
  eqEditor->addWidget(kEdit, 2, 1, 1, 7);
  compileButton = new QPushButton(tr("Create vector field"));
  eqEditor->addWidget(compileButton, 3, 0, 1, 2);
  vectorFieldLayout->addLayout(eqEditor);

  // Error message
  fieldError = new QLabel;
  fieldError->setVisible(false);
  vectorFieldLayout->addWidget(fieldError);

  // Graphics widget vector field controls
  QHBoxLayout *cameraControls = new QHBoxLayout;
  cameraControls->addWidget(orbitCameraCheckboxVectorField);
  QPushButton *resetCameraButton = new QPushButton("Reset camera");
  cameraControls->addWidget(resetCameraButton);
  cameraControls->addStretch();
  vectorFieldLayout->addLayout(cameraControls);

  // Spacing between camera controls and vector field labels
  vectorFieldLayout->addSpacing(15);
  fieldDivider = Line({0.5, 0.5, 0.5, 1}, {0.5, 0.5, 0.5, 1}, 1);
  fieldDivider->setVisible(false);
  vectorFieldLayout->addWidget(fieldDivider);
  vectorFieldLayout->addSpacing(15);

  // Labels for the vector field
  fieldColor = SolidWidget({0.0, 0.7, 1.0, 1}, {0.0, 0.1, 0.8, 1}, 250, 4);
  fieldColor->setVisible(false);
  fieldView = new QCheckBox("View vector field");
  fieldView->setVisible(false);
  fieldMessage = new QLabel;
  fieldMessage->setVisible(false);
  minMaxMessage = new QLabel;
  minMaxMessage->setVisible(false);
  vectorFieldLayout->addWidget(fieldColor);
  vectorFieldLayout->addWidget(fieldView);
  vectorFieldLayout->addWidget(fieldMessage);
  vectorFieldLayout->addWidget(minMaxMessage);

  // Spacing between vector field and curl
  vectorFieldLayout->addSpacing(25);

  // Labels for the curl
  curlColor = SolidWidget({1.0, 0.65, 0.0, 1}, {0.8, 0.0, 0.0, 1}, 250, 4);
  curlColor->setVisible(false);
  curlView = new QCheckBox("View curl");
  curlView->setVisible(false);
  curlMessage = new QLabel;
  curlMessage->setVisible(false);
  curlLenMessage = new QLabel;
  curlLenMessage->setVisible(false);
  vectorFieldLayout->addWidget(curlColor);
  vectorFieldLayout->addWidget(curlView);
  vectorFieldLayout->addWidget(curlMessage);
  vectorFieldLayout->addWidget(curlLenMessage);

  // TODO: for debugging only
  if (DebugField)
    vectorFieldLayout->addWidget(vectorFieldOutput);

  // Connect vector field control widgets to signals
  connect(compileButton, SIGNAL(released()), this, SLOT(onCreateVectorField()));
  connect(orbitCameraCheckboxVectorField, SIGNAL(stateChanged(int)), this, SLOT(onOrbitCheckboxVectorField(int)));
  connect(resetCameraButton, SIGNAL(released()), this, SLOT(onResetCameraVectorField()));
  connect(fieldView, SIGNAL(stateChanged(int)), this, SLOT(onChangeFieldVisibility(int)));
  connect(curlView, SIGNAL(stateChanged(int)), this, SLOT(onChangeCurlVisibility(int)));

  // Create a widget with the vector field layout that can be added to a tab widget
  QWidget *vectorFieldWidget = new QWidget;
  vectorFieldWidget->setLayout(vectorFieldLayout);
  return vectorFieldWidget;
}

QWidget *MainWidget::MakeVectorWidget(size_t index) {
  QVBoxLayout *layout = new QVBoxLayout;

  GraphicsVector vec = oglWidget->VectorAt(index);
  Color color = vec.color;

  // Row 0: colored box, name, length, dot product
  QHBoxLayout *row0 = new QHBoxLayout;
  row0->setSpacing(20);
  QWidget *colorWidget = SolidWidget(color, 30, 20);
  row0->addWidget(colorWidget);

  string nameStr = Bold("Vector " + oglWidget->VectorName(index));
  if (vec.crossA != "" && vec.crossB != "") {
    nameStr += " = " + Bold(vec.crossA) + " X " + Bold(vec.crossB);
  }
  QLabel *name = new QLabel(Fancy(nameStr));
  row0->addWidget(name);

  QLabel *length = new QLabel(Fancy("Length: " + TrimZeroes(Vector::Length(vec.start, vec.end))));
  row0->addWidget(length);

  if (vec.crossA != "" && vec.crossB != "") {
    QLabel *dot = new QLabel(Fancy(Bold(vec.crossA) + "." + Bold(vec.crossB) + " = " + TrimZeroes(vec.dotProduct)));
    row0->addWidget(dot);
  }
  row0->addStretch();
  layout->addLayout(row0);

  // Row 1: visibility, normalize, origin, delete
  QHBoxLayout *row1 = new QHBoxLayout;
  row1->setSpacing(20);
  QCheckBox *visibility = new QCheckBox(Fancy("Visible"));
  visibility->setChecked(true);
  row1->addWidget(visibility);
  QCheckBox *normalize = new QCheckBox(Fancy("Show normalized vector"));
  row1->addWidget(normalize);
  QCheckBox *origin = new QCheckBox(Fancy("Show at origin"));
  row1->addWidget(origin);
  QPushButton *del = new QPushButton("Delete");
  row1->addWidget(del);
  row1->addStretch();
  layout->addLayout(row1);

  // Row 2: start, end, direction
  QHBoxLayout *row2 = new QHBoxLayout;
  row2->setSpacing(20);
  row2->addWidget(new QLabel(Fancy("Start: " + Vec3String(vec.start))));
  row2->addWidget(new QLabel(Fancy("End: " + Vec3String(vec.end))));
  row2->addWidget(new QLabel(Fancy("Direction: " + Vec3String(Vector::GetVector(vec.start, vec.end), "<", ">"))));
  row2->addStretch();
  layout->addLayout(row2);

  // Connect controls to slots
  connect(visibility, &QCheckBox::stateChanged, this, [this, index](int state){ onChangeVectorVisibility(state, index); });
  connect(normalize, &QCheckBox::stateChanged, this, [this, index](int state){ onChangeVectorNormalize(state, index); });
  connect(origin, &QCheckBox::stateChanged, this, [this, index](int state){ onChangeVectorOrigin(state, index); });
  connect(del, &QPushButton::released, this, [this, index](){ onDeleteVector(index); });

  QWidget *widget = new QWidget;
  widget->setProperty("cssClass", "vectorWidget");
  widget->setLayout(layout);
  widget->setMinimumSize(100, 125);
  widget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
  return widget;
}

void MainWidget::ColorVectorWidgets() {
  QColor background = palette().color(QWidget::backgroundRole());
  bool darkMode = background.red() < 100;
  textBrowser->append(Fancy("background palette color: " + to_string(background.red()) + ", " + to_string(background.green()) + ", " + to_string(background.blue())));
  size_t widgetIndex = 0;
  Color baseColor = { background.red()/255.0f, background.green()/255.0f, background.blue()/255.0f, background.alpha()/255.0f };
  for (size_t i = 0; i < vectorWidgets.size(); ++i) {
    if (!oglWidget->IsVectorDeleted(i)) {
      ++widgetIndex;
    }
    // Bottommost widget has a darker color
    float diff = darkMode ? -0.03 : -0.07;
    if (widgetIndex % 2 == 0) {
      diff = darkMode ? 0.05 : -0.04;
    }
    Color bgColor = { baseColor.r + diff, baseColor.g + diff, baseColor.b + diff, 1 };
    textBrowser->append(Fancy("vector widget background " + MathUtils::RGBString(bgColor)));
    vectorWidgets.at(i)->setStyleSheet(Fancy("*[cssClass~=\"vectorWidget\"] { background: " + MathUtils::RGBString(bgColor) + "; }"));
  }
}

void MainWidget::ColorGraphicsWidget() {
  QColor background = palette().color(QWidget::backgroundRole());
  bool darkMode = background.red() < 100;
  Color color = { background.red()/255.0f, background.green()/255.0f, background.blue()/255.0f, background.alpha()/255.0f };
  oglWidget->SetBackgroundColor(color, darkMode);
}

QWidget *MainWidget::Line(Color left, Color right, float height) {
  QWidget *lineWidget = new QWidget;
  lineWidget->setMinimumSize(0.0, height);
  lineWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
  lineWidget->setProperty("cssClass", "lineWidget");
  lineWidget->setStyleSheet(Fancy("*[cssClass~=\"lineWidget\"] { background: qlineargradient( x1:0 y1:0, x2:1 y2:0, stop:0 " + MathUtils::RGBString(left) + ", stop:1 " + MathUtils::RGBString(right) + "); }"));
  return lineWidget;
}

QWidget *MainWidget::SolidWidget(Color left, Color right, float width, float height) {
  QWidget *colorWidget = new QWidget;
  colorWidget->setMinimumSize(width, height);
  colorWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  colorWidget->setProperty("cssClass", "colorWidget");
  colorWidget->setStyleSheet(Fancy("*[cssClass~=\"colorWidget\"] { background: qlineargradient( x1:0 y1:0, x2:1 y2:0, stop:0 " + MathUtils::RGBString(left) + ", stop:1 " + MathUtils::RGBString(right) + "); }"));
  return colorWidget;
}

QWidget *MainWidget::SolidWidget(Color color, int width, int height) {
  QWidget *colorWidget = new QWidget;
  colorWidget->setMinimumSize(width, height);
  colorWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  colorWidget->setProperty("cssClass", "colorWidget");
  colorWidget->setStyleSheet(Fancy("*[cssClass~=\"colorWidget\"] { background:" + MathUtils::RGBString(color) + "; }"));
  return colorWidget;
}

QIcon MainWidget::SolidIcon(Color color, int width, int height) {
  QPixmap pixmap(width, height);
  QColor iconColor((int)(color.r * 255.0f), (int)(color.g * 255.0f), (int)(color.b * 255.0f), (int)(color.a * 255.0f));
  pixmap.fill(iconColor);
  QIcon solid(pixmap);
  return solid;
}

// Override the QWidget::changeEvent method to handle state changes in the widget.
void MainWidget::changeEvent(QEvent *event) {
  switch (event->type()) {
    case QEvent::PaletteChange: {
      textBrowser->append(Fancy("palette changed"));
      ColorVectorWidgets();
      ColorGraphicsWidget();
      break;
    }
    case QEvent::WindowStateChange: {
      if (windowState() == Qt::WindowMinimized) {
        minimizeAction->setText("Restore");
        // TODO: should the keyboard shortcut be changed to Ctrl+R for the Restore action or remain at Ctrl+M?
        // Ctrl+M doesn't really fit with Restore, but Ctrl+R typically means refresh
        // Could look into the weird diamond option that appears when other Mac apps are minimized
        // Downsize is the diamond option doesn't have a keyboard shortcut and keyboard shortcuts are the best
        QKeySequence restore(tr("Ctrl+R"));
        minimizeAction->setShortcut(restore);
      } else {
        minimizeAction->setText("Minimize");
        QKeySequence minimize(tr("Ctrl+M"));
        minimizeAction->setShortcut(minimize);
      }
      break;
    }
    default:
      break;
  }
}

void MainWidget::toggleMinimized() {
  if (windowState() == Qt::WindowMinimized) {
    showNormal();
    // This is equivalent to Command+Option+F
    // showMaximized();
  } else {
    showMinimized();
  }
}

// Handler for changing tab view (Vectors vs Function vs Vector Field)
void MainWidget::onChangeTab(int index) {
  textBrowser->append(Fancy("changed tab to " + to_string(index)));
  orbitCameraCheckboxVectors->setChecked(false);
  orbitCameraCheckboxFunction->setChecked(false);
  orbitCameraCheckboxVectorField->setChecked(false);
  oglWidget->SetMode((GraphicsMode)index);
  if (index == 0) {
    oglWidget->SetDebug(textBrowser);
  } else if (index == 1) {
    oglWidget->SetDebug(funcDebug);
  } else {
    oglWidget->SetDebug(vectorFieldOutput);
  }
}

// Handler for button click to add a vector
void MainWidget::onAddVector() {
  float xStart = startXSpin->value();
  float yStart = startYSpin->value();
  float zStart = startZSpin->value();
  float xEnd = endXSpin->value();
  float yEnd = endYSpin->value();
  float zEnd = endZSpin->value();

  Vec3 start = {xStart, yStart, zStart};
  Vec3 end = {xEnd, yEnd, zEnd};
  Color color = oglWidget->AddVector(start, end);

  startXSpin->setValue(0.00);
  startYSpin->setValue(0.00);
  startZSpin->setValue(0.00);
  endXSpin->setValue(0.00);
  endYSpin->setValue(0.00);
  endZSpin->setValue(0.00);

  startXSpin->setFocus(Qt::TabFocusReason);

  size_t lastIndex = oglWidget->NumVectors() - 1;
  string name = oglWidget->VectorName(lastIndex);
  QIcon solid = SolidIcon(color, 10, 10);
  crossStart->addItem(solid, Fancy(name), QVariant((int)lastIndex));
  crossEnd->addItem(solid, Fancy(name), QVariant((int)lastIndex));
  crossControls->setVisible(true);

  QWidget *widget = MakeVectorWidget(lastIndex);
  vectorsContainer->addWidget(widget);
  vectorWidgets.push_back(widget);
  ColorVectorWidgets();
}

// Handler for button click to add the cross product of two vectors
void MainWidget::onCrossVectors() {
  int start = crossStart->currentData().toInt();
  int end = crossEnd->currentData().toInt();
  textBrowser->append(Fancy("crossing vectors " + to_string(start) + " and " + to_string(end)));
  Color color = oglWidget->CrossVectors(start, end);

  crossStart->setFocus(Qt::OtherFocusReason);

  size_t lastIndex = oglWidget->NumVectors() - 1;
  string name = oglWidget->VectorName(lastIndex);
  QIcon solid = SolidIcon(color, 10, 10);
  crossStart->addItem(solid, Fancy(name), QVariant((int)lastIndex));
  crossEnd->addItem(solid, Fancy(name), QVariant((int)lastIndex));
  crossControls->setVisible(true);

  QWidget *widget = MakeVectorWidget(lastIndex);
  vectorsContainer->addWidget(widget);
  vectorWidgets.push_back(widget);
  ColorVectorWidgets();
}

// Handler for button click to create a function
void MainWidget::onCreateFunction() {
  funcError->setVisible(false);
  funcDebug->clear();
  funcDebug->append(Fancy("Attempting to create function"));

  string errorMsg = "";

  ParserAlt *parser = new ParserAlt(LexMode::SingleVariable, funcDebug);

  string xText = xEdit->text().toStdString();
  Expr *xFunc = parser->Parse(xText);
  if (!xFunc) {
    errorMsg += "Failed to parse function for " + Italic("x") + ".<br/>";
  }

  string yText = yEdit->text().toStdString();
  Expr *yFunc = parser->Parse(yText);
  if (!yFunc) {
    errorMsg += "Failed to parse function for " + Italic("y") + ".<br/>";
  }

  string zText = zEdit->text().toStdString();
  Expr *zFunc = parser->Parse(zText);
  if (!zFunc) {
    errorMsg += "Failed to parse function for " + Italic("z") + ".<br/>";
  }

  if (xFunc && yFunc && zFunc) {
    float min = tMinSpin->value();
    float max = tMaxSpin->value();
    if (max - min < 0.005) {
      funcError->setText(Fancy("Error: maximum " + Italic("t") + " value must be greater than minimum " + Italic("t") + " value."));
      funcError->setVisible(true);
      tMinSpin->setFocus(Qt::OtherFocusReason);
      return;
    }
    int numVectors = 2 * numVectorsSlider->value();
    oglWidget->SetFunctions(xFunc, yFunc, zFunc, min, max, numVectors);
  } else {
    funcError->setText(Fancy(errorMsg));
    funcError->setVisible(true);
    funcDebug->append(Fancy("Failed to parse functions"));
    xEdit->setFocus(Qt::OtherFocusReason);
  }
}

// Handler for button click to create a vector field
void MainWidget::onCreateVectorField() {
  vectorFieldOutput->clear();
  textBrowser->clear();
  fieldError->setVisible(false);
  string errorMsg = "";

  // ParserAlt *parser = new ParserAlt(LexMode::MultiVariable, vectorFieldOutput); // Output debug info to separate text browser
  ParserAlt *parser = new ParserAlt(LexMode::MultiVariable); // No debugging

  std::string iText = iEdit->text().toStdString();
  Expr *iFunc = parser->Parse(iText);
  if (!iFunc) {
    errorMsg += "Failed to parse function for " + Italic("i") + ".<br/>";
  }

  std::string jText = jEdit->text().toStdString();
  Expr *jFunc = parser->Parse(jText);
  if (!jFunc) {
    errorMsg += "Failed to parse function for " + Italic("j") + ".<br/>";
  }

  std::string kText = kEdit->text().toStdString();
  Expr *kFunc = parser->Parse(kText);
  if (!kFunc) {
    errorMsg += "Failed to parse function for " + Italic("k") + ".<br/>";
  }

  if (iFunc && jFunc && kFunc) {
    compileButton->setText("Update vector field");
    VectorField *field = new VectorField(iFunc, jFunc, kFunc);
    vectorFieldOutput->append("Created vector field");
    oglWidget->SetVectorField(field);
    float minLength = oglWidget->MinVectorFieldLength(), maxLength = oglWidget->MaxVectorFieldLength();

    fieldDivider->setVisible(true);
    fieldColor->setVisible(true);
    fieldView->setVisible(true);
    fieldView->setChecked(true);
    fieldMessage->setVisible(true);
    string fieldMsg = Bold("Vector field: ") + Equation("v") + "<br/>" + field->ToString();
    fieldMessage->setText(Fancy(fieldMsg));
    string message = "Minimum vector length of " + Bold("v") + " = " + TrimZeroes(minLength) +
                     "<br/>Maximum vector length of " + Bold("v") + " = " + TrimZeroes(maxLength);
    minMaxMessage->setVisible(true);
    minMaxMessage->setWordWrap(true);
    minMaxMessage->setText(Fancy(message));

    VectorField *curl = oglWidget->Curl();
    if (curl) {
      curlColor->setVisible(true);
      curlView->setVisible(true);
      curlView->setChecked(true);
      curlMessage->setVisible(true);
      string curlMsg = Bold("Curl(v): ") + Equation("c") + "<br/>" + curl->ToString();
      curlMessage->setText(Fancy(curlMsg));
      float min = oglWidget->MinCurlLength(), max = oglWidget->MaxCurlLength();
      string lenMsg = "Minimum vector length of " + Bold("curl(v)") + " = " + TrimZeroes(min) +
                      "<br/>Maximum vector length of " + Bold("curl(v)") + " = " + TrimZeroes(max);
      curlLenMessage->setVisible(true);
      curlLenMessage->setWordWrap(true);
      curlLenMessage->setText(Fancy(lenMsg));
    }
  } else {
    fieldError->setText(Fancy(errorMsg));
    fieldError->setVisible(true);
    iEdit->setFocus(Qt::OtherFocusReason);
  }
}

// Handler for orbit camera checkbox state change - vectors
void MainWidget::onOrbitCheckboxVectors(int state) {
  oglWidget->SetOrbit(state);
}

// Handler for reset camera button click - function
void MainWidget::onResetCameraFunction() {
  orbitCameraCheckboxFunction->setChecked(false);
  oglWidget->ResetCameraFunc();
}

// Handler for reset camera button click - vectors
void MainWidget::onResetCameraVectors() {
  orbitCameraCheckboxVectors->setChecked(false);
  oglWidget->ResetCameraVectors();
}

// Handler for button to look at origin - vectors
void MainWidget::onLookAtOriginVectors() {
  orbitCameraCheckboxVectors->setChecked(false);
  oglWidget->LookAtOriginVectors();
}

// Handler for orbit camera checkbox state change - function
void MainWidget::onOrbitCheckboxFunction(int state) {
  oglWidget->SetOrbit(state);
}

// Handler for orbit camera checkbox state change - vector field
void MainWidget::onOrbitCheckboxVectorField(int state) {
  oglWidget->SetOrbit(state);
}

// Handler for reset camera button click - vector field
void MainWidget::onResetCameraVectorField() {
  oglWidget->ResetCameraVectorField();
  orbitCameraCheckboxVectorField->setChecked(false);
}

// Handler for a checkbox state to change the visibility of the vector at index
void MainWidget::onChangeVectorVisibility(int state, size_t index) {
  oglWidget->SetVectorVisibility(state, index);
}

// Handler for a checkbox state to change the normalization of the vector at index
void MainWidget::onChangeVectorNormalize(int state, size_t index) {
  oglWidget->SetVectorNormalized(state, index);
}

// Handler for a checkbox state to change the "draw at origin" of the vector at index
void MainWidget::onChangeVectorOrigin(int state, size_t index) {
  oglWidget->SetVectorOrigin(state, index);
}

// Handler for a button click to delete the vector at index
void MainWidget::onDeleteVector(size_t index) {
  string name = oglWidget->VectorName(index);
  oglWidget->DeleteVector(index);
  vectorWidgets.at(index)->setVisible(false);

  ColorVectorWidgets();

  int itemToDelete = -1;
  for (int i = 0; i < crossStart->count(); ++i) {
    string str = crossStart->itemText(i).toStdString();
    if (str == name) {
      itemToDelete = i;
      break;
    }
  }

  if (itemToDelete > -1 && itemToDelete <= crossStart->count()) {
    crossStart->removeItem(itemToDelete);
    crossEnd->removeItem(itemToDelete);
  }

  if (oglWidget->NumVectors() < 1) {
    crossControls->setVisible(false);
  } else {
    crossStart->setCurrentIndex(0);
    crossEnd->setCurrentIndex(0);
  }
}

void MainWidget::onChangeFieldVisibility(int state) {
  oglWidget->SetViewField(state);
}

void MainWidget::onChangeCurlVisibility(int state) {
  oglWidget->SetViewCurl(state);
}

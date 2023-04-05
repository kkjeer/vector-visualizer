#include <QtWidgets>
#include "mainwidget.h"

const int WIDTH = 1000;
const int HEIGHT = 700;

int main(int argc, char *argv[]) {
    // Creates an instance of QApplication
    QApplication a(argc, argv);

    a.setStyleSheet("QLineEdit { padding: 2px; } ");

    // This is our MainWidget class containing our GUI and functionality
    MainWidget w;
    // This prevents the user from resizing the app below this width and height, but they can make it bigger
    w.setMinimumSize(WIDTH, HEIGHT);
    // Show main window
    // w.show();
    w.showMaximized();

    // run the application and return execs() return value/code
    return a.exec();
}
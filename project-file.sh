echo "Generating Vector Visualizer.pro file..."
qmake -project
echo "Done generating Vector Visualizer.pro file."
echo "Don't forget to add the line 'QT += widgets core gui openglwidgets' below INCLUDEPATH in the Vector Visualizer.pro file."
echo "Also add the line 'CONFIG+=sdk_no_version_check' below DEFINES in the .pro file to silence warnings when creating the makefile."
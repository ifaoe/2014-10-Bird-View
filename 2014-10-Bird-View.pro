#-------------------------------------------------
#
# Project created by QtCreator 2014-10-09T15:45:54
#
#-------------------------------------------------

QT       += core gui sql xml webkit

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 2014-10-Bird-View
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        mainwindow.cpp\
        ImgCanvas.cpp \
        QgsLayerStack.cpp \
        ConfigHandler.cpp \
        DatabaseHandler.cpp

HEADERS  += mainwindow.h\
		ImgCanvas.h \
		QgsLayerStack.h \
		ConfigHandler.h \
		DatabaseHandler.h

FORMS    += mainwindow.ui

CONFIG+=link_pkgconfig
PKGCONFIG+=opencv

INCLUDEPATH += /usr/include
INCLUDEPATH += /usr/include/qgis
DEPENDPATH += /usr/include
DEPENDPATH += /usr/include/qgis

DEFINES += GUI_EXPORT= CORE_EXPORT=

unix: LIBS += -L/usr/lib/\
 -lgdal \
 -lqgis_core\
 -lqgis_gui\
 -lboost_program_options\
 -L/usr/local/lib\
 /usr/local/lib/libopencv_core.so\
 /usr/local/lib/libopencv_highgui.so\
 /usr/local/lib/libopencv_imgproc.so
 

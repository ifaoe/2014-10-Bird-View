#-------------------------------------------------
#
# Project created by QtCreator 2014-10-09T15:45:54
#
#-------------------------------------------------

QT += core gui sql xml webkit network

QT_QMAKE_EXECUTABLE = /usr/bin/qmake-qt4

TARGET = 2014-10-Bird-View
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

VPATH += ./src
SOURCES += main.cpp\
        mainwindow.cpp\
        ImgCanvas.cpp \
        QgsLayerStack.cpp \
        ConfigHandler.cpp \
        DatabaseHandler.cpp \
        SessionDialog.cpp \
        QCategoryButton.cpp \
        QCategoryCheckButton.cpp \
        IdSelectionDialog.cpp \
        MeasurementDialog.cpp \
        QgsMapMarker.cpp
		
HEADERS  += mainwindow.h\
		ImgCanvas.h \
		QgsLayerStack.h \
		ConfigHandler.h \
		DatabaseHandler.h \
		SessionDialog.h \
		census.hpp \
		QCategoryButton.h \
		QCategoryCheckButton.h \
		IdSelectionDialog.h \
        MeasurementDialog.h \
        QgsMapMarker.h
        
VPATH += ./ui
FORMS    += mainwindow.ui \
			sessiondialog.ui \
			widget_sessions.ui\
			widget_census.ui\
			widget_objects.ui \
			widget_graphics.ui \
			dialog_idselection.ui \
			dialog_measurement.ui \
			widget_census_shared.ui
			
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
 

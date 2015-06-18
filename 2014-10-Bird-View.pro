#-------------------------------------------------
#
# Project created by QtCreator 2014-10-09T15:45:54
#
#-------------------------------------------------

QT       += core gui sql xml webkit network

greaterThan(QT_MAJOR_VERSION, 5): QT += widgets

TARGET = 2014-10-Bird-View
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        mainwindow.cpp\
        ImgCanvas.cpp \
        QgsLayerStack.cpp \
        ConfigHandler.cpp \
        DatabaseHandler.cpp \
        SessionDialog.cpp \
        Stuk4Dialog.cpp \
        QCategoryButton.cpp \
        QCategoryCheckButton.cpp \
        GroupSelection.cpp \
        MeasurementDialog.cpp \
        QgsMapMarker.cpp \
        QgsMapLabel.cpp
		
HEADERS  += mainwindow.h\
		ImgCanvas.h \
		QgsLayerStack.h \
		ConfigHandler.h \
		DatabaseHandler.h \
		SessionDialog.h \
		census.hpp \
		Stuk4Dialog.h \
		QCategoryButton.h \
		QCategoryCheckButton.h \
		GroupSelection.h \
        MeasurementDialog.h \
        QgsMapMarker.h \
        QgsMapLabel.h

FORMS    += mainwindow.ui \
			sessiondialog.ui \
			stuk4codes.ui \
			widget_sessions.ui\
			widget_census.ui\
			widget_objects.ui \
			widget_graphics.ui \
			dialog_groupid.ui \
			dialog_measurement.ui \
			widget_multicensus.ui
			
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
 

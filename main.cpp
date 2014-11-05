#include <QApplication>
#include <qgsapplication.h>
#include "ConfigHandler.h"
#include "mainwindow.h"
#include "DatabaseHandler.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ConfigHandler *cfg = new ConfigHandler(argc, argv);
    DatabaseHandler *db = new DatabaseHandler(cfg);

    // Qgis Pfad setzen und Provider laden
    QgsApplication::setPrefixPath("/usr", true);
    QgsApplication::initQgis();

    MainWindow w(cfg, db);


    w.showMaximized();
//    w.show();
    int result = a.exec();

    QgsApplication::exitQgis();
    return result;
}

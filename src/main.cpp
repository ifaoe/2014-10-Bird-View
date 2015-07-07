//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QApplication>
#include <QMessageBox>
#include <qgsapplication.h>
#include "ConfigHandler.h"
#include "mainwindow.h"
#include "SessionDialog.h"
#include "DatabaseHandler.h"

int main(int argc, char *argv[])
{
    QApplication::setDesktopSettingsAware(false);
    QApplication::setStyle("GTK+");
    QApplication a(argc, argv);
    ConfigHandler *cfg = new ConfigHandler(argc, argv);

    // Qgis Pfad setzen und Provider laden
    QgsApplication::setPrefixPath("/usr", true);
    QgsApplication::initQgis();

    SessionDialog d(cfg);
    d.exec();

    DatabaseHandler *db = new DatabaseHandler(cfg);

    MainWindow w(cfg, db);
    w.showMaximized();

    int result = a.exec();

    QgsApplication::exitQgis();
    return result;
}
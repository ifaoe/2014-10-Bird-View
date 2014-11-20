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
#include "DatabaseHandler.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ConfigHandler *cfg = new ConfigHandler(argc, argv);
    DatabaseHandler *db = new DatabaseHandler(cfg);

    // Qgis Pfad setzen und Provider laden
    QgsApplication::setPrefixPath("/usr", true);
    QgsApplication::initQgis();

//    QMessageBox msgBox;
//	msgBox.setText("Auswahl Bildbetrachter");
//	msgBox.setInformativeText("Vor- oder Endbestimmer?");
//	QAbstractButton *preButton = msgBox.addButton("Vorbestimmer", QMessageBox::YesRole);
//	QAbstractButton *endButton = msgBox.addButton("Endbestimmer", QMessageBox::NoRole);
//	msgBox.exec();
//	if(msgBox.clickedButton() == preButton) {
//		cfg->censor = 1;
//	} else if (msgBox.clickedButton() == endButton) {
//		cfg->censor = 2;
//	} else {
//		exit(1);
//	}

    MainWindow w(cfg, db);


    w.showMaximized();
//    w.show();
    int result = a.exec();

    QgsApplication::exitQgis();
    return result;
}

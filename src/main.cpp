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
	QCoreApplication::setOrganizationName("ifaoe");
	QCoreApplication::setOrganizationDomain("ifaoe.de");
	QCoreApplication::setApplicationName("daisi-bird-view");
	QIcon::setThemeName("gnome");
	QStringList theme_paths;
	theme_paths << "/usr/share/icons/";
	QIcon::setThemeSearchPaths(theme_paths);
    QApplication app(argc, argv);
    QApplication::setDesktopSettingsAware(false);
    QApplication::setStyle("GTK+");
    ConfigHandler *config = new ConfigHandler;
    config->InitSettings();

    // Qgis Pfad setzen und Provider laden
    QgsApplication::setPrefixPath("/usr", true);
    QgsApplication::initQgis();

    DatabaseHandler *db = new DatabaseHandler(config);

    MainWindow main_window(config, db);
    if (config->getAppMaximized())
    	main_window.showMaximized();
    else
    	main_window.show();

    QgsApplication::exitQgis();
    return app.exec();;
}

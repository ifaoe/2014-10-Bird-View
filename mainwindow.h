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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QDial>
#include <QScrollArea>
#include <QWebView>
#include <qgsproviderregistry.h>
#include <qgsmaplayerregistry.h>
#include "ImgCanvas.h"
#include "DatabaseHandler.h"
#include "ConfigHandler.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(ConfigHandler *cfgArg = 0, DatabaseHandler *dbArg = 0, QWidget *parent = 0);
    ~MainWindow();

private slots:
	void handleSessionButton();
	void handleBirdSave();
	void handleMammalSave();
	void objectUpdateSelection();
	void handleMapToolButton();
	void handleNoSightingButton();
	void handleOneToOneZoom();
	void handleDirDial();

private:
	QgsProviderRegistry *prvRegistry = 0;
	QgsMapLayerRegistry *lyrRegistry = 0;
	ImgCanvas *imgcvs = 0;
    Ui::MainWindow *ui;
	ConfigHandler *cfg;
    DatabaseHandler *db;
    QItemSelectionModel *objSelector;
    QString session = "";
    census * curObj = 0;
    int currentRow = -1;
    int mapMode = 0;
    QVBoxLayout *lytFrmImg;
    QWebView * geoMap;

    QPushButton * btnZoomOneOne;
    QPushButton * btnMapModeImg;
    QPushButton * btnMapModeGeo;
    QDial * dirDial;

    void selectButtonByString(QButtonGroup * btnGrp, QString str);
    void colorTableReady(int censor);
    void initMapView();
protected:
    void resizeEvent(QResizeEvent *event);
};

#endif // MAINWINDOW_H

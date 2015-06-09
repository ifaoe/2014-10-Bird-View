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
#include <QSignalMapper>
#include <QWebView>
#include <qgsproviderregistry.h>
#include <qgsmaplayerregistry.h>
#include "ImgCanvas.h"
#include "DatabaseHandler.h"
#include "ConfigHandler.h"
#include "ui_widget_sessions.h"
#include "ui_widget_objects.h"
#include "ui_widget_census.h"
#include "ui_widget_graphics.h"
#include "QCategoryCheckButton.h"

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
	void populateObjectTable();

	void objectUpdateSelection();
	void handleMapToolButton();
	void handleOneToOneZoom();
	void handleDirDial();
	void handleUsrSelect();
	void handleBrightnessSlider();
	void handleContrastSlider();
	void handleBrightnessReset();
	void handleContrastReset();
	void handleLineEditFilter();
	void handleTypeFilter(int index);
	void handleCensusFilter(int index);
	void handleCamFilter(int index);
	void handleCensorFilter();
	void handleDeleteButton();
	void handleSaveButton();
	void handleSortingHeader(int section);
	void handleStuk4Selection();
	void handleBirdSpanMeasurement();
	void handleBirdLengthMeasurement();
	void handleMammalLengthMeasurement();
	void handleGroupSelection();
private:
	bool msm_running = false;
	QgsProviderRegistry *prvRegistry = 0;
	QgsMapLayerRegistry *lyrRegistry = 0;
	ImgCanvas *imgcvs = 0;
    Ui::MainWindow *ui;
	ConfigHandler *cfg;
    DatabaseHandler *db;
    QItemSelectionModel *objSelector;
    QString session = "";
    QString session_path = "/net/daisi";
    QStringList censorList;
    census * curObj = 0;
    int currentRow = -1;
    int mapMode = 0;
    QVBoxLayout *lytFrmImg;
    QWebView * geoMap;

    QPushButton * btnZoomOneOne;
    QPushButton * btnToggleSource;
    QPushButton * btnMapModeImg;
    QPushButton * btnMapModeGeo;
    QDial * dirDial;
    bool dialChecked = false;

    // Filter Widgets
    QComboBox * cmbFilterCensus;
    QComboBox * cmbFilterType;
    QComboBox * cmbFilterCam;
    QLineEdit * pteFilterImg;
    QLineEdit * pteFilterId;
    QMap<QString, QString> filterMap;

    // Treewidgets
    Ui::wdgSessions * wdgSession;
    Ui::wdgObjects * wdgObjects;
    Ui::wdgCensus * wdgCensus;
    Ui::wdgGraphics * wdgGraphics;

    QTreeWidgetItem* twgSession = 0;
	QTreeWidgetItem* twgObjects = 0;
	QTreeWidgetItem* twgCensus = 0;
	QTreeWidgetItem* twgGraphics = 0;

	QCategoryCheckButton* cbtSession = 0;
	QCategoryCheckButton* cbtObjects = 0;
	QCategoryCheckButton* cbtCensus = 0;
	QCategoryCheckButton* cbtGraphics = 0;

    QSet<int> sortSet;

    void selectButtonByString(QButtonGroup * btnGrp, QString str);
    void colorTableRow(QColor color, int row);
    void initMapView();
    void uiPreSelection(census * cobj);
    void initFilters();
    bool compareResults(census * cobj, census * pobj);
protected:
    void resizeEvent(QResizeEvent *event);
};

#endif // MAINWINDOW_H

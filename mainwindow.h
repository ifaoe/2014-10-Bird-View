#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
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

private:
	QgsProviderRegistry *prvRegistry = 0;
	QgsMapLayerRegistry *lyrRegistry = 0;
	ImgCanvas *imgcvs = 0;
	ConfigHandler *cfg;
    DatabaseHandler *db;
    Ui::MainWindow *ui;
    QItemSelectionModel *objSelector;
    QString session = "";
    census * curObj;
    int currentRow = -1;
    int mapMode = 0;
    QVBoxLayout *lytFrmImg;
    QWebView * geoMap;

    void selectButtonByString(QButtonGroup * btnGrp, QString str);
    void colorTableReady(int censor);
};

#endif // MAINWINDOW_H

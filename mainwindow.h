#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QScrollArea>
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

private:
	QgsProviderRegistry *prvRegistry = 0;
	QgsMapLayerRegistry *lyrRegistry = 0;
	ImgCanvas *imgcvs = 0;
	ConfigHandler *cfg;
    DatabaseHandler *db;
    Ui::MainWindow *ui;
    QItemSelectionModel *objSelector;
    QString session = "";

    int currentRow = -1;

    void populateObjectTable();

};

#endif // MAINWINDOW_H

#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ImgCanvas.h"
#include <QSqlQuery>

MainWindow::MainWindow( ConfigHandler *cfgArg, DatabaseHandler *dbArg,QWidget *parent) :
	ui(new Ui::MainWindow), cfg(cfgArg), db(dbArg) ,QMainWindow(0)
{
	ui->setupUi(this);

	ui->tblObjects->setHorizontalHeaderLabels(QStringList() << "ID" << "TP" <<"CAM" << "IMG" << "USER");
	ui->tblObjects->setColumnWidth(0, 75);
	ui->tblObjects->setColumnWidth(1, 40);
	ui->tblObjects->setColumnWidth(2, 40);
	ui->tblObjects->setColumnWidth(3, 80);
	ui->tblObjects->setColumnWidth(4, 90);

    prvRegistry = QgsProviderRegistry::instance();
    lyrRegistry = QgsMapLayerRegistry::instance();
	QStringList providers = prvRegistry->providerList();
	if (providers.size()==0) {
		qFatal("Die Providerliste ist leer.");
	}
	bool doneGdal = false;
	bool doneOgr = false;
	bool donePGIS = false;
	for (int i=0; i<providers.size(); ++i) {
		if (!doneGdal) { doneGdal = (providers.at(i) == QString("gdal")); }
		if (!doneOgr)  { doneOgr  = (providers.at(i) == QString("ogr")); }
		if (!donePGIS) { donePGIS = (providers.at(i) == QString("postgres")); }
	}
	if (!doneGdal) {
		qFatal("GDAL");
	}

	if (!doneOgr) {
		qFatal("OGR");
	}
	if (!donePGIS) {
		qFatal("Postgresql/PostGIS");
	}

	objSelector = ui->tblObjects->selectionModel();
	ui->tblObjects->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tblObjects->setSelectionBehavior(QAbstractItemView::SelectRows);
	connect( objSelector, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(objectUpdateSelection()));

    imgcvs = new ImgCanvas(ui->wdgImg, ui);
    QVBoxLayout *lytFrmImg = new QVBoxLayout;
    lytFrmImg->setMargin(0);
    lytFrmImg->addWidget(imgcvs);
    ui->wdgImg->setLayout(lytFrmImg);

    // Populate session list
    ui->cmbSession->addItems(db->getSessionList());

    ui->cmbMammal->addItems(cfg->mmList);

    connect(ui->btnSession, SIGNAL(released()), this, SLOT(handleSessionButton()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleSessionButton() {
	QSqlQuery *query = db->getObjectResult( ui->cmbSession->currentText() );
	session = ui->cmbSession->currentText();
	ui->tblObjects->setRowCount(query->size());
	int row = 0;
	while(query->next()) {
		QTableWidgetItem * id = new QTableWidgetItem(query->value(0).toString());
		QTableWidgetItem * type = new QTableWidgetItem(query->value(1).toString());
		QTableWidgetItem * cam = new QTableWidgetItem(query->value(2).toString());
		QTableWidgetItem * img = new QTableWidgetItem(query->value(3).toString());
		QTableWidgetItem * user = new QTableWidgetItem(query->value(4).toString());
		id->setTextAlignment(Qt::AlignHCenter);
		type->setTextAlignment(Qt::AlignHCenter);
		cam->setTextAlignment(Qt::AlignHCenter);
		img->setTextAlignment(Qt::AlignHCenter);
		user->setTextAlignment(Qt::AlignHCenter);
		ui->tblObjects->setItem(row,0,id);
		ui->tblObjects->setItem(row,1,type);
		ui->tblObjects->setItem(row,2,cam);
		ui->tblObjects->setItem(row,3,img);
		ui->tblObjects->setItem(row,4,user);
		row++;
	}
	delete query;
}

void MainWindow::objectUpdateSelection() {
	currentRow = objSelector->selectedRows().at(0).row();
	QString prjDir = "";
	QString obj = ui->tblObjects->item(currentRow, 0)->text();
	QString cam = ui->tblObjects->item(currentRow, 2)->text();
	QString img = ui->tblObjects->item(currentRow, 3)->text();
	QString date = session.left(10);
	QDir base = QDir(cfg->imgPath);
	base.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
	QStringList subdirs = base.entryList();
	for(int i=0; i<subdirs.size(); i++) {
		if (subdirs.at(i).left(10).compare(date) == 0) {
			prjDir = subdirs.at(i);
			break;
		}
	}

	QString file = cfg->imgPath + "/" + prjDir + "/cam" + cam + "/geo/" + img + ".tif";
	imgcvs->loadObject(file, db->getObjectPosition(obj));
}


void MainWindow::handleBirdSave() {
	int objId = ui->tblObjects->item(currentRow, 0)->text().toInt();
	census * object = db->getObjectData(objId);
//	QString
}

void MainWindow::handleMammalSave() {

}

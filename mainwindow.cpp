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

	ui->cmbSession->addItems(db->getSessionList());

	objSelector = ui->tblObjects->selectionModel();
	ui->tblObjects->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tblObjects->setSelectionBehavior(QAbstractItemView::SelectRows);

    imgcvs = new ImgCanvas(ui->wdgImg, ui);
    geoMap = new QWebView(ui->wdgImg);
    lytFrmImg = new QVBoxLayout;
    lytFrmImg->setMargin(0);
    lytFrmImg->addWidget(imgcvs);
    ui->wdgImg->setLayout(lytFrmImg);

    connect( objSelector, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(objectUpdateSelection()));
    connect(ui->btnSession, SIGNAL(released()), this, SLOT(handleSessionButton()));
    connect(ui->btnBirdReady, SIGNAL(released()), this, SLOT(handleBirdSave()));
    connect(ui->btnMammalReady, SIGNAL(released()), this, SLOT(handleMammalSave()));
    connect(ui->btnMapMode, SIGNAL(released()), this, SLOT(handleMapToolButton()));
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
		int censor = query->value(5).toInt();
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
		if (censor == 1) {
			id->setBackgroundColor(Qt::yellow);
			type->setBackgroundColor(Qt::yellow);
			cam->setBackgroundColor(Qt::yellow);
			img->setBackgroundColor(Qt::yellow);
			user->setBackgroundColor(Qt::yellow);
		} else if (censor > 1) {
			id->setBackgroundColor(Qt::green);
			type->setBackgroundColor(Qt::green);
			cam->setBackgroundColor(Qt::green);
			img->setBackgroundColor(Qt::green);
			user->setBackgroundColor(Qt::green);
		}
	}
	delete query;
}

void MainWindow::objectUpdateSelection() {
	currentRow = objSelector->selectedRows().at(0).row();
	QString prjDir = "";
	QString objId = ui->tblObjects->item(currentRow, 0)->text();
	QString cam = ui->tblObjects->item(currentRow, 2)->text();
	QString img = ui->tblObjects->item(currentRow, 3)->text();
	curObj = db->getRawObjectData(objId);
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

	if(curObj->type.left(1) == "B" ) { // Bird Tab
		ui->wdgTabTypes->setCurrentIndex(0);
		selectButtonByString(ui->btngBirdType, curObj->type);
		selectButtonByString(ui->btngBirdQual, QString::number(curObj->quality));
		selectButtonByString(ui->btngBirdBhv, curObj->behavior);
		selectButtonByString(ui->btngBirdGnd, curObj->gender);
		selectButtonByString(ui->btngBirdAge, curObj->age);
		ui->cmbBird->setEditText(curObj->name);
		ui->txtBirdRemarks->setPlainText(curObj->remarks);
	} else if (curObj->type.left(1) == "M") { // Mammal Tab
		ui->wdgTabTypes->setCurrentIndex(1);
		selectButtonByString(ui->btngMammalType, curObj->type);
		selectButtonByString(ui->btngMammalQual, QString::number(curObj->quality));
		selectButtonByString(ui->btngMammalBhv, curObj->behavior);
		selectButtonByString(ui->btngMammalAge, curObj->age);
		ui->cmbMammal->setEditText(curObj->name);
		ui->txtMammalRemarks->setPlainText(curObj->remarks);
	}

	QString file = cfg->imgPath + "/" + prjDir + "/cam" + cam + "/geo/" + img + ".tif";
	imgcvs->loadObject(file, db->getObjectPosition(objId));
}


void MainWindow::handleBirdSave() {
	QString objId = ui->tblObjects->item(currentRow, 0)->text();
	curObj->type = ui->btngBirdType->checkedButton()->property("dbvalue").toString();
	curObj->quality = ui->btngBirdQual->checkedButton()->property("dbvalue").toInt();
	curObj->behavior = ui->btngBirdBhv->checkedButton()->property("dbvalue").toString();
	if (ui->gbxBirdGender->isChecked())
		curObj->gender = ui->btngBirdGnd->checkedButton()->property("dbvalue").toString();
	if (ui->gbxBirdAge->isChecked())
		curObj->age = ui->btngBirdAge->checkedButton()->property("dbvalue").toString();
	curObj->remarks = ui->txtBirdRemarks->toPlainText();
	curObj->name = ui->cmbBird->currentText();
	curObj->censor++;
	db->writeCensus(curObj);
	colorTableReady(curObj->censor);

	delete curObj;

	QModelIndex newIndex = objSelector->model()->index(currentRow+1, 0);
	objSelector->select(newIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void MainWindow::handleMammalSave() {
	QString objId = ui->tblObjects->item(currentRow, 0)->text();
	curObj->type = ui->btngMammalType->checkedButton()->property("dbvalue").toString();
	curObj->quality = ui->btngMammalQual->checkedButton()->property("dbvalue").toInt();
	curObj->behavior = ui->btngMammalBhv->checkedButton()->property("dbvalue").toString();
	curObj->gender = "";
	if (ui->gbxMammalAge->isChecked())
		curObj->age = ui->btngMammalAge->checkedButton()->property("dbvalue").toString();
	curObj->remarks = ui->txtMammalRemarks->toPlainText();
	curObj->name = ui->cmbMammal->currentText();
	curObj->censor++;
	db->writeCensus(curObj);
	colorTableReady(curObj->censor);
	delete curObj;

	QModelIndex newIndex = objSelector->model()->index(currentRow+1, 0);
	objSelector->select(newIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void MainWindow::selectButtonByString(QButtonGroup * btnGrp, QString str) {
	QList<QAbstractButton*> btnList = btnGrp->buttons();
	if(str != "") {
		btnGrp->checkedButton()->setChecked(true);
	}
	for(int i=0; i<btnList.size(); i++) {
		if(btnList.at(i)->property("dbvalue") == str) {
			btnList.at(i)->setChecked(true);
		}
	}
}

void MainWindow::handleMapToolButton() {
	QString scale="8";
	QString url = "";
	url += "http://www.openstreetmap.org/?mlat=" + QString::number(curObj->ly) + "&mlon=" + QString::number(curObj->lx);
	url += "#map=" + scale + "/" + QString::number(curObj->ly) + "/" + QString::number(curObj->lx);
	if (mapMode == 0) {
		mapMode = 1;
		qDebug() << "Load URL: " << url;
		geoMap->load(QUrl(url));
		geoMap->show();
		lytFrmImg->removeWidget(imgcvs);
		lytFrmImg->addWidget(geoMap);
	} else {
		mapMode = 0;
		lytFrmImg->removeWidget(geoMap);
		lytFrmImg->addWidget(imgcvs);
		geoMap->hide();
	}
}

void MainWindow::handleNoSightingButton() {
	QModelIndex newIndex = objSelector->model()->index(currentRow+1, 0);
	objSelector->select(newIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void MainWindow::colorTableReady(int censor) {
	if (censor == 1) {
		ui->tblObjects->item(currentRow, 0)->setBackgroundColor(Qt::yellow);
		ui->tblObjects->item(currentRow, 1)->setBackgroundColor(Qt::yellow);
		ui->tblObjects->item(currentRow, 2)->setBackgroundColor(Qt::yellow);
		ui->tblObjects->item(currentRow, 3)->setBackgroundColor(Qt::yellow);
		ui->tblObjects->item(currentRow, 4)->setBackgroundColor(Qt::yellow);
	} else if (censor > 1) {
		ui->tblObjects->item(currentRow, 0)->setBackgroundColor(Qt::green);
		ui->tblObjects->item(currentRow, 1)->setBackgroundColor(Qt::green);
		ui->tblObjects->item(currentRow, 2)->setBackgroundColor(Qt::green);
		ui->tblObjects->item(currentRow, 3)->setBackgroundColor(Qt::green);
		ui->tblObjects->item(currentRow, 4)->setBackgroundColor(Qt::green);
	}
}

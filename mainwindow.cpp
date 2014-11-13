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

#include <QDebug>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ImgCanvas.h"
#include <QSqlQuery>
#include <QMotifStyle>
#include <QMessageBox>

MainWindow::MainWindow( ConfigHandler *cfgArg, DatabaseHandler *dbArg, QWidget *parent) :
	QMainWindow(0), ui(new Ui::MainWindow), cfg(cfgArg), db(dbArg)
{
	ui->setupUi(this);

	ui->tblObjects->setHorizontalHeaderLabels(QStringList() << "ID" << "TP" <<"CAM" << "IMG" << "USER");
	ui->tblObjects->setColumnWidth(0, 75);
	ui->tblObjects->setColumnWidth(1, 80);
	ui->tblObjects->setColumnWidth(2, 40);
	ui->tblObjects->setColumnWidth(3, 80);
	ui->tblObjects->setColumnWidth(4, 50);

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
//	db->getBirdTypeList(ui->cmbBird);
	ui->cmbBird->addItems(db->getBirdTypeList());
	ui->cmbMammal->addItems(cfg->mmList);

	objSelector = ui->tblObjects->selectionModel();
	ui->tblObjects->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tblObjects->setSelectionBehavior(QAbstractItemView::SelectRows);

	initMapView();

    // connect signals
    connect( objSelector, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(objectUpdateSelection()));
    connect(ui->btnSession, SIGNAL(released()), this, SLOT(handleSessionButton()));
    connect(ui->btnBirdReady, SIGNAL(released()), this, SLOT(handleBirdSave()));
    connect(ui->btnMammalReady, SIGNAL(released()), this, SLOT(handleMammalSave()));
    connect(btnMapModeImg , SIGNAL(released()), this, SLOT(handleMapToolButton()));
    connect(btnMapModeGeo , SIGNAL(released()), this, SLOT(handleMapToolButton()));
    connect(ui->btnNoSightReady, SIGNAL(released()), this, SLOT(handleNoSightingButton()));
    connect(btnZoomOneOne, SIGNAL(released()), this, SLOT(handleOneToOneZoom()));
    connect(dirDial, SIGNAL(sliderReleased()), this, SLOT(handleDirDial()));

    // TODO: Button for "agree" -> next object (spacebar?)

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
		QTableWidgetItem * type;
		if (query->value(6).toString() == "") {
			type = new QTableWidgetItem(query->value(1).toString());
		} else {
			type = new QTableWidgetItem(query->value(6).toString());
		}
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
	// TODO: Cleanup.
	ui->lblMsmTool->setText("Zum Beginn der Messung ersten Punkt anklicken.");
	currentRow = objSelector->selectedRows().at(0).row();
	QString prjDir = "";
	QString objId = ui->tblObjects->item(currentRow, 0)->text();
	QString cam = ui->tblObjects->item(currentRow, 2)->text();
	QString img = ui->tblObjects->item(currentRow, 3)->text();
	QString shTp = ui->tblObjects->item(currentRow,1)->text().left(1);
	curObj = db->getRawObjectData(objId);
	if (curObj->direction >= 0 ) {
		dirDial->setValue((curObj->direction+180)%360);
	} else {
		dirDial->setValue(180);
	}
	if (curObj->imageQuality > 0) {
		ui->chbImgQuality->setChecked(true);
	} else {
		ui->chbImgQuality->setChecked(false);
	}
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

	if(curObj->type.left(1) == "B" || shTp == "V" ) { // Bird Tab
		ui->wdgTabTypes->setCurrentIndex(0);
		int index = ui->cmbBird->findText(curObj->name);
		ui->cmbBird->setCurrentIndex(index);
		selectButtonByString(ui->btngBirdQual, QString::number(curObj->quality));
		selectButtonByString(ui->btngBirdBhv, curObj->behavior);
		if(curObj->gender != "") {
			ui->gbxBirdGender->setChecked(true);
			selectButtonByString(ui->btngBirdGnd, curObj->gender);
		} else {
			ui->gbxBirdGender->setChecked(false);
		}
		if(curObj->age != "") {
			ui->gbxBirdAge->setChecked(true);
			selectButtonByString(ui->btngBirdAge, curObj->age);
		} else {
			ui->gbxBirdAge->setChecked(false);
		}
		ui->txtBirdRemarks->setPlainText(curObj->remarks);
	} else if (curObj->type.left(1) == "M" || shTp == "M") { // Mammal Tab
		ui->wdgTabTypes->setCurrentIndex(1);
		int index = ui->cmbMammal->findText(curObj->name);
		ui->cmbMammal->setCurrentIndex(index);
		selectButtonByString(ui->btngMammalQual, QString::number(curObj->quality));
		selectButtonByString(ui->btngMammalBhv, curObj->behavior);
		if (curObj->age != "") {
			ui->gbxMammalAge->setChecked(true);
			selectButtonByString(ui->btngMammalAge, curObj->age);
		} else {
			ui->gbxMammalAge->setChecked(false);
		}
		ui->txtMammalRemarks->setPlainText(curObj->remarks);
	} else {
		ui->wdgTabTypes->setCurrentIndex(2);
		selectButtonByString(ui->btngNoSightQual, QString::number(curObj->quality));
		ui->txtNoSightRemarks->setPlainText(curObj->remarks);
	}

	QString file = cfg->imgPath + "/" + prjDir + "/cam" + cam + "/geo/" + img + ".tif";
	imgcvs->loadObject(file, db->getObjectPosition(objId));
}


void MainWindow::handleBirdSave() {
	if (ui->btngBirdBhv->checkedButton()->property("dbvalue").toString() == "FLY" && curObj->direction < 0) {
		QMessageBox * msgBox = new QMessageBox();
		msgBox->setText(trUtf8("Bitte Flugrichtung bestimmen."));
		QAbstractButton *nextButton = msgBox->addButton(trUtf8("Ok"), QMessageBox::YesRole);
		msgBox->exec();
		if(msgBox->clickedButton() == nextButton) {
			return;
		}
	} else if (ui->btngBirdBhv->checkedButton()->property("dbvalue").toString() != "FLY") {
		curObj->direction = -1;
	}
	QString objId = ui->tblObjects->item(currentRow, 0)->text();
	curObj->type = ui->wdgTabTypes->currentWidget()->property("dbvalue").toString();
	curObj->quality = ui->btngBirdQual->checkedButton()->property("dbvalue").toInt();
	curObj->behavior = ui->btngBirdBhv->checkedButton()->property("dbvalue").toString();
	if (ui->gbxBirdGender->isChecked())
		curObj->gender = ui->btngBirdGnd->checkedButton()->property("dbvalue").toString();
	if (ui->gbxBirdAge->isChecked())
		curObj->age = ui->btngBirdAge->checkedButton()->property("dbvalue").toString();
	curObj->remarks = ui->txtBirdRemarks->toPlainText();
	curObj->name = ui->cmbBird->currentText();
//	curObj->name = ui->cmbBird->itemData(ui->cmbBird->currentIndex()).toString();
	curObj->censor = ui->btngCensor->checkedButton()->property("dbvalue").toInt();
	if (ui->chbImgQuality->isChecked())
		curObj->imageQuality = 1;
	// write object data to db
	db->writeCensus(curObj);
	// refresh object table
	colorTableReady(curObj->censor);
	ui->tblObjects->item(currentRow, 1)->setText(curObj->type);
	ui->tblObjects->item(currentRow, 4)->setText(curObj->usr);
	// delete object structure
	delete curObj;
	// clear remark box
	ui->txtBirdRemarks->clear();
	// select next object in table
	if(currentRow < ui->tblObjects->rowCount()) {
		QModelIndex newIndex = objSelector->model()->index(currentRow+1, 0);
		objSelector->select(newIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	}
}

void MainWindow::handleMammalSave() {
	QString objId = ui->tblObjects->item(currentRow, 0)->text();
	curObj->type = ui->wdgTabTypes->currentWidget()->property("dbvalue").toString();
	curObj->quality = ui->btngMammalQual->checkedButton()->property("dbvalue").toInt();
	curObj->behavior = ui->btngMammalBhv->checkedButton()->property("dbvalue").toString();
	curObj->gender = "";
	if (ui->gbxMammalAge->isChecked())
		curObj->age = ui->btngMammalAge->checkedButton()->property("dbvalue").toString();
	if (ui->chbImgQuality->isChecked())
		curObj->imageQuality = 1;
	curObj->remarks = ui->txtMammalRemarks->toPlainText();
	curObj->name = ui->cmbMammal->currentText();
//	curObj->name = ui->cmbMammal->itemData(ui->cmbMammal->currentIndex()).toString();
	curObj->censor = ui->btngCensor->checkedButton()->property("dbvalue").toInt();

	db->writeCensus(curObj);
	colorTableReady(curObj->censor);
	ui->tblObjects->item(currentRow, 1)->setText(curObj->type);
	ui->tblObjects->item(currentRow, 4)->setText(curObj->usr);
	delete curObj;
	ui->txtMammalRemarks->clear();
	if(currentRow < ui->tblObjects->rowCount()) {
		QModelIndex newIndex = objSelector->model()->index(currentRow+1, 0);
		objSelector->select(newIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	}
}

void MainWindow::handleNoSightingButton() {
	QString objId = ui->tblObjects->item(currentRow, 0)->text();
	curObj->type = "NOSIGHT";
	curObj->censor = ui->btngCensor->checkedButton()->property("dbvalue").toInt();
	curObj->quality = ui->btngNoSightQual->checkedButton()->property("dbvalue").toInt();
	curObj->remarks = ui->txtNoSightRemarks->toPlainText();
	curObj->direction = -1;
	db->writeCensus(curObj);
	colorTableReady(curObj->censor);
	ui->tblObjects->item(currentRow, 1)->setText(curObj->type);
	ui->tblObjects->item(currentRow, 4)->setText(curObj->usr);

	delete curObj;
	ui->txtNoSightRemarks->clear();
	if(currentRow < ui->tblObjects->rowCount()) {
		QModelIndex newIndex = objSelector->model()->index(currentRow+1, 0);
		objSelector->select(newIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	}
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
	if (curObj == 0) return;
	// Show Geo Location on Map
	// TODO: Use proper map API
	// TODO: Google or OpenStreetMaps ?
	QString scale="8";
	QString url = "";
	url += "http://www.openstreetmap.org/?mlat=" + QString::number(curObj->ly) + "&mlon=" + QString::number(curObj->lx);
	url += "#map=" + scale + "/" + QString::number(curObj->ly) + "/" + QString::number(curObj->lx);
//	url += "http://maps.google.com/maps?q=" + QString::number(curObj->ly) + "," + QString::number(curObj->lx);
	if (sender() == btnMapModeImg) {
		qDebug() << "Load URL: " << url;
		geoMap->load(QUrl(url));
		geoMap->show();
		lytFrmImg->removeWidget(imgcvs);
		lytFrmImg->addWidget(geoMap);
	} else {
		lytFrmImg->removeWidget(geoMap);
		lytFrmImg->addWidget(imgcvs);
		geoMap->hide();
	}
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

void MainWindow::handleOneToOneZoom() {
	if (curObj == 0) return;
	imgcvs->centerOnWorldPosition(curObj->ux, curObj->uy, 1.0);
}

void MainWindow::initMapView() {
	// Setup image and map
	imgcvs = new ImgCanvas(ui->wdgImg, ui);
	geoMap = new QWebView(ui->wdgImg);
	lytFrmImg = new QVBoxLayout;
	lytFrmImg->setMargin(0);
	lytFrmImg->addWidget(imgcvs);
	ui->wdgImg->setLayout(lytFrmImg);

	// Setup buttons
	btnMapModeImg = new QPushButton(imgcvs);
	btnMapModeImg->setText("Karte");


	btnMapModeGeo = new QPushButton(geoMap);
	btnMapModeGeo->setText("Bild");


	btnZoomOneOne = new QPushButton(imgcvs);
	btnZoomOneOne->setText("1:1");

	dirDial = new QDial(imgcvs);
	dirDial->setFixedSize(80,80);
	dirDial->move(10,10);
	dirDial->setMaximum(359);
	dirDial->setMinimum(0);
	dirDial->setWrapping(true);
	dirDial->setNotchesVisible(false);
	dirDial->setStyle(new QMotifStyle);
}

void MainWindow::resizeEvent(QResizeEvent * event) {

	int wdgSizeX = ui->wdgImg->size().width();
	int wdgSizeY = ui->wdgImg->size().height();
	btnMapModeImg->move(wdgSizeX-100,40);
	btnMapModeGeo->move(wdgSizeX-160,80);
	btnZoomOneOne->move(wdgSizeX-100,10);
}

void MainWindow::handleDirDial() {
	curObj->direction = (dirDial->value() + 180)%360;
}

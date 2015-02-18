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
#include "FilterDialog.h"
#include "ImgCanvas.h"
#include <QSqlQuery>
#include <QMotifStyle>
#include <QMessageBox>
#include <qgsmultibandcolorrenderer.h>

MainWindow::MainWindow( ConfigHandler *cfgArg, DatabaseHandler *dbArg, QWidget *parent) :
	QMainWindow(0), ui(new Ui::MainWindow), cfg(cfgArg), db(dbArg)
{
	Q_UNUSED(parent);
	ui->setupUi(this);

	ui->tblObjects->setColumnCount(7);
	ui->tblObjects->setHorizontalHeaderLabels(QStringList() << "ID" << "IMG" << "CAM" << "TP"
			<< "USR" << "CEN" << "CNT");
	ui->tblObjects->setColumnWidth(0, 75);
	ui->tblObjects->setColumnWidth(1, 80);
	ui->tblObjects->setColumnWidth(2, 40);
	ui->tblObjects->setColumnWidth(3, 80);
	ui->tblObjects->setColumnWidth(4, 50);
	ui->tblObjects->hideColumn(4);
	ui->tblObjects->hideColumn(5);
	ui->tblObjects->hideColumn(6);

	ui->sldBrightness->setMinimum(0);
	ui->sldBrightness->setMaximum(100);
	ui->sldBrightness->setValue(0);

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
	ui->cmbBird->addItems(db->getBirdTypeList());
	ui->cmbMammal->addItems(db->getMammalTypeList());

	objSelector = ui->tblObjects->selectionModel();
	ui->tblObjects->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tblObjects->setSelectionBehavior(QAbstractItemView::SelectRows);

	initMapView();

    // connect signals
	btnBirdMapper = new QSignalMapper;
	btnMammalMapper = new QSignalMapper;
	btnNoSightMapper = new QSignalMapper;

    connect( objSelector, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(objectUpdateSelection()));
    connect(ui->btnSession, SIGNAL(released()), this, SLOT(populateObjectTable()));
    connect(btnMapModeImg , SIGNAL(released()), this, SLOT(handleMapToolButton()));
    connect(btnMapModeGeo , SIGNAL(released()), this, SLOT(handleMapToolButton()));
    connect(btnZoomOneOne, SIGNAL(released()), this, SLOT(handleOneToOneZoom()));
    connect(dirDial, SIGNAL(sliderReleased()), this, SLOT(handleDirDial()));
    connect(ui->btnUserSelect, SIGNAL(released()), this, SLOT(handleUsrSelect()));
    connect(ui->sldBrightness, SIGNAL(sliderReleased()), this, SLOT(handleBrightnessSlider()));

    connect(ui->btnBirdSave, SIGNAL(released()), this, SLOT(handleBirdSave()));
    connect(ui->btnMammalSave, SIGNAL(released()), this, SLOT(handleMammalSave()));
    connect(ui->btnNoSightSave, SIGNAL(released()), this, SLOT(handleNoSightingSave()));

    connect(ui->tblObjects->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), this,
    		SLOT(handleHeaderFilter()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::populateObjectTable(QString filter) {
	currentRow = -1;
	objSelector->clearSelection();
	ui->tblObjects->clear();
	ui->tblObjects->setHorizontalHeaderLabels(QStringList() << "ID" << "IMG" << "CAM" << "TP");
	ui->tblObjects->model()->removeRows(0,ui->tblObjects->rowCount());
	QSqlQuery *query = db->getObjectResult( ui->cmbSession->currentText(), filter);
	session = ui->cmbSession->currentText();
	int row = 0;
	QMap<int, QString> usrCensus = db->getUserCensus(cfg->user(), ui->cmbSession->currentText());
	QMap<int, QString> finalCensus = db->getFinalCensus(ui->cmbSession->currentText());
	while(query->next()) {
		ui->tblObjects->insertRow( ui->tblObjects->rowCount() );
		QTableWidgetItem * id = new QTableWidgetItem(query->value(0).toString());
		// Check wether or not there is a better type definition available
		QString tstr;
		if (finalCensus.contains(query->value(0).toInt())) {
			tstr = finalCensus[query->value(0).toInt()];
		} else if (usrCensus.contains(query->value(0).toInt())) {
			tstr = usrCensus[query->value(0).toInt()];
		} else {
			tstr = query->value(1).toString();
		}
		QTableWidgetItem * type = new QTableWidgetItem(tstr);
		QTableWidgetItem * cam = new QTableWidgetItem(query->value(2).toString());
		QTableWidgetItem * img = new QTableWidgetItem(query->value(3).toString());
		QTableWidgetItem * user = new QTableWidgetItem(query->value(4).toString());
		QTableWidgetItem * censor = new QTableWidgetItem(query->value(5).toString());
		QTableWidgetItem * count = new QTableWidgetItem(query->value(6).toString());

		id->setTextAlignment(Qt::AlignHCenter);
		type->setTextAlignment(Qt::AlignHCenter);
		cam->setTextAlignment(Qt::AlignHCenter);
		img->setTextAlignment(Qt::AlignHCenter);
		censor->setTextAlignment(Qt::AlignHCenter);
		count->setTextAlignment(Qt::AlignHCenter);
		user->setTextAlignment(Qt::AlignHCenter);
		id->setFlags(id->flags() & ~Qt::ItemIsEditable);
		type->setFlags(id->flags() & ~Qt::ItemIsEditable);
		cam->setFlags(id->flags() & ~Qt::ItemIsEditable);
		img->setFlags(id->flags() & ~Qt::ItemIsEditable);
		user->setFlags(id->flags() & ~Qt::ItemIsEditable);
		censor->setFlags(id->flags() & ~Qt::ItemIsEditable);
		count->setFlags(id->flags() & ~Qt::ItemIsEditable);
		ui->tblObjects->setItem(row,0,id);
		ui->tblObjects->setItem(row,1,img);
		ui->tblObjects->setItem(row,2,cam);
		ui->tblObjects->setItem(row,3,type);
		ui->tblObjects->setItem(row,4,user);
		ui->tblObjects->setItem(row,5,censor);
		ui->tblObjects->setItem(row,6,count);
		colorTableReady(query->value(5).toInt(), row);
		row++;
	}
	delete query;
	cfg->image_path = db->getProjectPath(session);
}

void MainWindow::objectUpdateSelection() {
	// TODO: Cleanup.
	// TODO: Fix: Crash on empty line
	dialChecked = false;
	ui->lblMsmTool->setText("Zum Beginn der Messung ersten Punkt anklicken.");
	if (objSelector->selectedRows().isEmpty()) return;
	currentRow = objSelector->selectedRows().at(0).row();
	QString objId = ui->tblObjects->item(currentRow, 0)->text();
	QString cam = ui->tblObjects->item(currentRow, 2)->text();
	QString img = ui->tblObjects->item(currentRow, 1)->text();
	QString type = ui->tblObjects->item(currentRow, 3)->text();
	ui->cmbUsers->clear();
	curObj = db->getRawObjectData(objId, cfg->user());
	if (curObj->type.isEmpty()) curObj->type = type;
	censorList = db->getUserList(objId);
	ui->cmbUsers->addItems(censorList);
	uiPreSelection(curObj);

	if (!imgcvs->loadObject(curObj, db->getObjectPosition(objId))) return;
	handleBrightnessSlider();
}


/*
 * Handle Save Buttons
 * TODO: Put all in one save routine
 * TODO: PUT ALL IN ONE SAVE ROUTINE!
 */

void MainWindow::saveRoutine(QString type) {
	qDebug() << "Trying to save as user: " << curObj->usr;
	QString objId = ui->tblObjects->item(currentRow, 0)->text();

	if (type == "bird") {
		curObj->type = ui->wdgTabTypes->currentWidget()->property("dbvalue").toString();
		curObj->quality = ui->btngBirdQual->checkedButton()->property("dbvalue").toInt();
		curObj->behavior = ui->btngBirdBhv->checkedButton()->property("dbvalue").toString();
		if (ui->gbxBirdGender->isChecked()) {
			curObj->gender = ui->btngBirdGnd->checkedButton()->property("dbvalue").toString();
		} else {
			curObj->gender = "";
		}
		if (ui->gbxBirdAge->isChecked()) {
			curObj->age = ui->btngBirdAge->checkedButton()->property("dbvalue").toString();
		} else {
			curObj->age = "";
		}

		curObj->remarks = ui->txtBirdRemarks->toPlainText();
		curObj->name = ui->cmbBird->currentText();
	} else if (type == "mammal") {
		curObj->type = ui->wdgTabTypes->currentWidget()->property("dbvalue").toString();
		curObj->name = ui->cmbMammal->currentText();
		curObj->quality = ui->btngMammalQual->checkedButton()->property("dbvalue").toInt();
		curObj->behavior = ui->btngMammalBhv->checkedButton()->property("dbvalue").toString();
		if (ui->gbxMammalAge->isChecked()) {
			curObj->age = ui->btngMammalAge->checkedButton()->property("dbvalue").toString();
		} else {
			curObj->age = "";
		}
		curObj->gender = "";
		curObj->remarks = ui->txtMammalRemarks->toPlainText();
	} else if (type == "nosight") {
		curObj->type = "NOSIGHT";
		curObj->name = "";
		curObj->quality = ui->btngNoSightQual->checkedButton()->property("dbvalue").toInt();
		curObj->behavior = "";
		curObj->age = "";
		curObj->gender = "";
		curObj->remarks = ui->txtNoSightRemarks->toPlainText();
		curObj->direction = -1;
	} else {
		return;
	}

	if (db->getMaxCensor(QString::number(curObj->id), curObj->usr) < 1) {
		qDebug() << "Erster Bestimmer.";
		curObj->censor = 1;
	} else if (db->getCensorCount(QString::number(curObj->id), "1", curObj->usr) == 1) {
		qDebug() << "Zweiter Bestimmer.";
		census * cenObj = db->getCensusData(QString::number(curObj->id));
		curObj->censor = 2;
		bool agree = true;
		agree = agree && (curObj->name == cenObj->name);
		agree = agree && (curObj->type == cenObj->type);
		if (!agree) {
			QMessageBox * msgBox = new QMessageBox();
			msgBox->setText(QString::fromUtf8("Keine Übereinstimmung zum Erstbestimmer.\n"
					" Noch keine Endbestimmung möglich.\n"
					"Bestimmung als Vorbestimmer."));
			msgBox->addButton(trUtf8("Ok"), QMessageBox::YesRole);
			msgBox->exec();
			delete msgBox;
			curObj->censor = 1;
		}
	} else if (db->getMaxCensor(QString::number(curObj->id), curObj->usr) >= 2) {
		qDebug() << "Zusätzlicher Bestimmer.";
		curObj->censor = 0;
		QMessageBox * msgBox = new QMessageBox();
		msgBox->setText(trUtf8("Objekt bereits Endbestimmt. Abspeichern als zusätzliche Bestimmung."));
		msgBox->addButton(trUtf8("Ok"), QMessageBox::YesRole);
		msgBox->exec();
		delete msgBox;
	} else if (db->getCensorCount(QString::number(curObj->id), "1", curObj->usr) >= 2){
		qDebug() << "Ditter Bestimmer.";
		curObj->censor = 2;
		QMessageBox * msgBox = new QMessageBox();
		msgBox->setText("Endbestimmung als " + QString::number(censorList.size()) + ". Bestimmer. \n"
				+ "Bitte mit " + censorList.join(", ") + " abstimmen.");
		msgBox->addButton(trUtf8("Ok"), QMessageBox::YesRole);
		QAbstractButton *noButton = msgBox->addButton(trUtf8("Abbrechen"), QMessageBox::NoRole);
		msgBox->exec();
		if (msgBox->clickedButton() == noButton) {
			delete msgBox;
			return;
		}
	} else {
		// Never come here!
		qDebug() << "You should have never come here!";
		exit(1);
	}

	if (ui->chbImgQuality->isChecked()) {
		curObj->imageQuality = 1;
	} else {
		curObj->imageQuality = 0;
	}

	// write object data to db
	db->writeCensus(curObj);
	// refresh object table
	colorTableReady(curObj->censor, currentRow);
	ui->tblObjects->item(currentRow, 3)->setText(curObj->type);
	// delete object structure
	delete curObj;
	// select next object in table
	if(currentRow < ui->tblObjects->rowCount()) {
		QModelIndex newIndex = objSelector->model()->index(currentRow+1, 0);
		ui->tblObjects->scrollTo(newIndex);
		objSelector->select(newIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
	}

}

void MainWindow::handleBirdSave() {
	if ((ui->btngBirdBhv->checkedButton()->property("dbvalue").toString() == "FLY") && (dialChecked == false)) {
		QMessageBox * msgBox = new QMessageBox();
		msgBox->setText(trUtf8("Bitte Flugrichtung bestimmen, oder als unbestimmt markieren."));
		QAbstractButton *nextButton = msgBox->addButton(trUtf8("Ok"), QMessageBox::NoRole);
		QAbstractButton *noDirButton = msgBox->addButton(trUtf8("Unbestimmt"), QMessageBox::YesRole);
		msgBox->exec();
		if (msgBox->clickedButton() == nextButton) {
			delete msgBox;
			return;
		} else if (msgBox->clickedButton() == noDirButton) {;
			curObj->direction = -1;
		}
		delete msgBox;
	} else if (ui->btngBirdBhv->checkedButton()->property("dbvalue").toString() != "FLY") {
		curObj->direction = -1;
	} else {

	}
	if (ui->cmbBird->currentText() == "") {
		QMessageBox * msgBox = new QMessageBox();
		msgBox->setText(trUtf8("Bitte Art auswählen!"));
		QAbstractButton *nextButton = msgBox->addButton(trUtf8("Ok"), QMessageBox::YesRole);
		msgBox->exec();
		if(msgBox->clickedButton() == nextButton) {
			delete msgBox;
			return;
		}
		delete msgBox;
	}
	saveRoutine("bird");
}

void MainWindow::handleMammalSave() {
	if (ui->cmbMammal->currentText() == "") {
		QMessageBox * msgBox = new QMessageBox();
		msgBox->setText(trUtf8("Bitte Art auswählen!"));
		QAbstractButton *nextButton = msgBox->addButton(trUtf8("Ok"), QMessageBox::YesRole);
		msgBox->exec();
		if(msgBox->clickedButton() == nextButton) {
			return;
		}
	}
	if (dialChecked == false) {
		QMessageBox * msgBox = new QMessageBox();
		msgBox->setText(trUtf8("Bitte Schwimmrichtung bestimmen, oder als unbestimmt markieren."));
		QAbstractButton *nextButton = msgBox->addButton(trUtf8("Ok"), QMessageBox::NoRole);
		QAbstractButton *noDirButton = msgBox->addButton(trUtf8("Unbestimmt"), QMessageBox::YesRole);
		msgBox->exec();
		if (msgBox->clickedButton() == nextButton) {
			delete msgBox;
			return;
		} else if (msgBox->clickedButton() == noDirButton) {;
			curObj->direction = -1;
		}
		delete msgBox;
	}
	saveRoutine("mammal");
}

void MainWindow::handleNoSightingSave() {
	saveRoutine("nosight");
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

/*
 * Handle Inet Map Button
 * Set Url and Load via Browser Widget
 * TODO: Use proper map API
 * TODO: Google or OpenStreetMaps ?
 */
void MainWindow::handleMapToolButton() {
	if (curObj == 0) return;
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

/*
 * Color the current row of the table corresponding to the censor value
 */
void MainWindow::colorTableReady(int censor, int row) {
	int c = ui->tblObjects->columnCount();
	QColor col;
	if (censor == 1) {
		col =Qt::yellow;
	} else if (censor > 1) {
		col = Qt::green;
	} else {
		col = Qt::white;
	}
	for (int i=0; i<c; i++) {
		ui->tblObjects->item(row, i)->setBackgroundColor(col);
	}
}

/*
 * 1:1 Zoom Button handler
 */
void MainWindow::handleOneToOneZoom() {
	if (curObj == 0) return;
	imgcvs->centerOnWorldPosition(curObj->ux, curObj->uy, 1.0);
}

void MainWindow::initMapView() {
	// Setup image and map
	imgcvs = new ImgCanvas(ui->wdgImg, ui, cfg);
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

//	btnToggleSource = new QPushButton(imgcvs);
//	btnToggleSource->setText("Quelle");

	// Add QDial for direction
	dirDial = new QDial(imgcvs);
	dirDial->setFixedSize(80,80);
	dirDial->move(10,50);
	dirDial->setMaximum(359);
	dirDial->setMinimum(0);
	dirDial->setWrapping(true);
	dirDial->setNotchesVisible(false);
	dirDial->setStyle(new QMotifStyle);
}

/*
 * Dynamically place the buttons and elements on the image canvas
 */
void MainWindow::resizeEvent(QResizeEvent * event) {
	Q_UNUSED(event);
	int wdgSizeX = ui->wdgImg->size().width();
	btnMapModeImg->move(wdgSizeX-100,40);
	btnMapModeGeo->move(wdgSizeX-160,80);
	btnZoomOneOne->move(wdgSizeX-100,10);
}

/*
 * Recalculate direction value depending on QDial value
 * Set the direction value only when dial is touched
 */
void MainWindow::handleDirDial() {
	qDebug() << "Handle direction dial with value: " << dirDial->value();
	curObj->direction = (dirDial->value() + 180)%360;
	dialChecked=true;
	qDebug() << curObj->direction;
}

/*
 * Function which selects Ui elements depending on the data in
 * the census struct.
 */
void MainWindow::uiPreSelection(census * cobj) {
	// handle user selection
	if (db->getCensorCount(QString::number(cobj->id), "1", cfg->user()) >= 2
			|| db->getMaxCensor(QString::number(cobj->id)) >= 2) {
		ui->cmbUsers->show();
		ui->btnUserSelect->show();
	} else {
		ui->cmbUsers->hide();
		ui->btnUserSelect->hide();
	}

	// clear remark boxes
	ui->txtBirdRemarks->clear();
	ui->txtMammalRemarks->clear();
	ui->txtNoSightRemarks->clear();

	// Recalculate values of the QDial to 0=North
	qDebug() << "Dir: " << cobj->direction;
	if (cobj->direction >= 0 ) {
		dirDial->setValue((cobj->direction+180)%360);
	} else {
		dirDial->setValue(180);
	}
	if (cobj->type == "BIRD" && cobj->behavior == "FLY"){
		dialChecked = true;
		handleDirDial();
	} else {
		dialChecked = false;
	}

	// Checkbox for very good objects in image quality
	// which could be used as example pictures
	if (cobj->imageQuality > 0) {
		ui->chbImgQuality->setChecked(true);
	} else {
		ui->chbImgQuality->setChecked(false);
	}

	// Find out which type is listed and switch to the corresponding tab
//	QString shTp = ui->tblObjects->item(ui->tblObjects->currentRow(),3)->text().left(1);
	QString shTp = cobj->type.left(1);
	if (shTp == "" ) {
		shTp = ui->tblObjects->item(ui->tblObjects->currentRow(),3)->text().left(1);
	}
	// by default, all options are off
	ui->gbxMammalAge->setChecked(false);
	ui->gbxBirdAge->setChecked(false);
	ui->gbxBirdGender->setChecked(false);

	if(shTp == "B" || shTp == "V" ) { // Bird Tab
			ui->wdgTabTypes->setCurrentIndex(0);
			int index = ui->cmbBird->findText(cobj->name);
			ui->cmbBird->setCurrentIndex(index);
			selectButtonByString(ui->btngBirdQual, QString::number(cobj->quality));
			selectButtonByString(ui->btngBirdBhv, cobj->behavior);
			if(cobj->gender != "") {
				ui->gbxBirdGender->setChecked(true);
				selectButtonByString(ui->btngBirdGnd, cobj->gender);
			}
			if(cobj->age != "") {
				ui->gbxBirdAge->setChecked(true);
				selectButtonByString(ui->btngBirdAge, cobj->age);
			}
			ui->txtBirdRemarks->setPlainText(cobj->remarks);
			ui->cmbBird->setFocus();
		} else if (shTp == "M" ) { // Mammal Tab
			ui->wdgTabTypes->setCurrentIndex(1);
			int index = ui->cmbMammal->findText(cobj->name);
			ui->cmbMammal->setCurrentIndex(index);
			selectButtonByString(ui->btngMammalQual, QString::number(cobj->quality));
			selectButtonByString(ui->btngMammalBhv, cobj->behavior);
			if (cobj->age != "") {
				ui->gbxMammalAge->setChecked(true);
				selectButtonByString(ui->btngMammalAge, cobj->age);
			}
			ui->txtMammalRemarks->setPlainText(cobj->remarks);
			ui->cmbMammal->setFocus();
		} else { //NoSighting tab
			ui->wdgTabTypes->setCurrentIndex(2);
			selectButtonByString(ui->btngNoSightQual, QString::number(cobj->quality));
			ui->txtNoSightRemarks->setPlainText(cobj->remarks);
		}
}

/*
 * Read results for the selected censor and set the Ui elements respectively
 */
void MainWindow::handleUsrSelect() {
	census * obj;
	if (ui->cmbUsers->currentIndex() == -1) return;
	obj = db->getRawObjectData(QString::number(curObj->id), ui->cmbUsers->currentText());
	uiPreSelection(obj);
	delete obj;
}

void MainWindow::handleBrightnessSlider() {
	qDebug() << "Changing Brightness";
	double scale = 1.0 - double(ui->sldBrightness->value())/100.0;
	int maxval = int(scale * 65000.);
//	int minval = int((1-scale) * 32000.);
	int minval = 0;
	qDebug() << "Scale: " << scale << "Max. value: " << maxval;
	QgsRasterLayer * imgLayer = imgcvs->getImageLayer();
	QgsRasterDataProvider * provider = imgLayer->dataProvider();
    QgsContrastEnhancement* qgsContrastEnhRed = new QgsContrastEnhancement(QGis::UInt16);
    qgsContrastEnhRed->setMinimumValue(minval);
    qgsContrastEnhRed->setMaximumValue(maxval);
    qgsContrastEnhRed->setContrastEnhancementAlgorithm ( QgsContrastEnhancement::StretchToMinimumMaximum);

    QgsContrastEnhancement* qgsContrastEnhGreen = new QgsContrastEnhancement(QGis::UInt16);
    qgsContrastEnhGreen->setMinimumValue(minval);
    qgsContrastEnhGreen->setMaximumValue(maxval);
    qgsContrastEnhGreen->setContrastEnhancementAlgorithm ( QgsContrastEnhancement::StretchToMinimumMaximum);

    QgsContrastEnhancement* qgsContrastEnhBlue = new QgsContrastEnhancement(QGis::UInt16);
    qgsContrastEnhBlue->setMinimumValue(minval);
    qgsContrastEnhBlue->setMaximumValue(maxval);
    qgsContrastEnhBlue->setContrastEnhancementAlgorithm ( QgsContrastEnhancement::StretchToMinimumMaximum);

    QgsMultiBandColorRenderer* renderer = new QgsMultiBandColorRenderer( provider , 1, 2, 3,
    		qgsContrastEnhRed, qgsContrastEnhGreen, qgsContrastEnhBlue);

    imgLayer->setRenderer(renderer);
    imgcvs->refresh();

    qDebug() << "Done.";
}

void MainWindow::handleHeaderFilter() {
	FilterDialog * dlg = new FilterDialog(db, session);
	dlg->exec();
	populateObjectTable(dlg->getFilter());
	delete dlg;
}

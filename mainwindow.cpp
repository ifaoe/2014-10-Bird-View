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
#include <qgsmultibandcolorrenderer.h>
#include <QLineEdit>

MainWindow::MainWindow( ConfigHandler *cfgArg, DatabaseHandler *dbArg, QWidget *parent) :
	QMainWindow(0), ui(new Ui::MainWindow), cfg(cfgArg), db(dbArg)
{
	Q_UNUSED(parent);
	ui->setupUi(this);
	ui->btnSave->setEnabled(false);
	ui->btnDelete->setEnabled(false);
	ui->tblObjects->setColumnCount(5);
	ui->tblObjects->setHorizontalHeaderLabels(QStringList() << "ID" << "IMG" << "CAM" << "TP" <<
			"CEN");
	ui->tblObjects->setColumnWidth(0, 75);
	ui->tblObjects->setColumnWidth(1, 80);
	ui->tblObjects->setColumnWidth(2, 40);
	ui->tblObjects->setColumnWidth(3, 50);
	ui->tblObjects->setColumnWidth(4, 50);

	initFilters();

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

    connect(ui->btnSave, SIGNAL(released()), this, SLOT(handleSaveButton()));

    connect(ui->btnDelete, SIGNAL(released()), this, SLOT(handleDeleteButton()));

    connect(ui->tblFilters->horizontalHeader(), SIGNAL(sectionClicked(int)), this,
    		SLOT(handleSortingHeader(int)));
    connect(ui->btnBirdSizeSpan, SIGNAL(clicked()), this, SLOT(handleBirdSpanMeasurement()));
    connect(ui->btnBirdSizeLength, SIGNAL(clicked()), this, SLOT(handleBirdLengthMeasurement()));
    connect(ui->btnMammalSizeLength, SIGNAL(clicked()), this, SLOT(handleMammalLengthMeasurement()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::populateObjectTable() {
	sortSet.clear();
	ui->btnSave->setEnabled(false);
	ui->btnDelete->setEnabled(false);
	QString filter = "WHERE TRUE" + QStringList(filterMap.values()).join("");
	session = ui->cmbSession->currentText();
	currentRow = -1;
	objSelector->clearSelection();
	ui->tblObjects->clear();
	ui->tblObjects->model()->removeRows(0,ui->tblObjects->rowCount());
	QSqlQuery *query = db->getObjectResult( session, cfg->user(), filter);
	int row = 0;
	QMap<int, QString> usrCensus = db->getUserCensus(cfg->user(), session);
	QMap<int, QString> finalCensus = db->getFinalCensus(session);

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
			tstr = "";
		}
		QTableWidgetItem * type = new QTableWidgetItem(query->value(1).toString());
		QTableWidgetItem * cam = new QTableWidgetItem(query->value(2).toString());
		QTableWidgetItem * img = new QTableWidgetItem(query->value(3).toString());
		QTableWidgetItem * census = new QTableWidgetItem(query->value(6).toString());


		id->setTextAlignment(Qt::AlignHCenter);
		type->setTextAlignment(Qt::AlignHCenter);
		cam->setTextAlignment(Qt::AlignHCenter);
		img->setTextAlignment(Qt::AlignHCenter);
		census->setTextAlignment(Qt::AlignHCenter);

		id->setFlags(id->flags() & ~Qt::ItemIsEditable);
		type->setFlags(id->flags() & ~Qt::ItemIsEditable);
		cam->setFlags(id->flags() & ~Qt::ItemIsEditable);
		img->setFlags(id->flags() & ~Qt::ItemIsEditable);
		census->setFlags(id->flags() & ~Qt::ItemIsEditable);

		ui->tblObjects->setItem(row,0,id);
		ui->tblObjects->setItem(row,1,img);
		ui->tblObjects->setItem(row,2,cam);
		ui->tblObjects->setItem(row,3,type);
		ui->tblObjects->setItem(row,4,census);


		if (query->value(4).toInt() > 1)
			colorTableRow(Qt::green, row);
		else if (query->value(4).toInt() == 1 && query->value(5).toInt() > 1)
			colorTableRow(Qt::red, row);
		else if (usrCensus.contains(query->value(0).toInt()))
			colorTableRow(Qt::yellow, row);
		else if (query->value(4).toInt() > 0)
			colorTableRow(Qt::gray, row);

		row++;
	}
	delete query;
	cfg->image_path = db->getProjectPath(session);
}

void MainWindow::objectUpdateSelection() {
	// TODO: Cleanup.
	// TODO: Fix: Crash on empty line
	dialChecked = false;
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

	// handle user selection
	if ((curObj->censor > 0) && (db->getMaxCensor(QString::number(curObj->id),cfg->user()) > 1)) {
		ui->btnDelete->setEnabled(false);
		ui->btnSave->setEnabled(false);
	} else {
		ui->btnDelete->setEnabled(true);
		ui->btnSave->setEnabled(true);
	}
	if (curObj->censor < 0)
		ui->btnDelete->setEnabled(false);

	if (db->getCensorCount(QString::number(curObj->id), "1", cfg->user()) >= 2
			|| db->getMaxCensor(QString::number(curObj->id)) >= 2) {
		ui->cmbUsers->setDisabled(false);
		ui->btnUserSelect->setDisabled(false);
	} else {
		ui->cmbUsers->clear();
		ui->cmbUsers->setDisabled(true);
		ui->btnUserSelect->setDisabled(true);
	}

	if (!imgcvs->loadObject(curObj, db->getObjectPosition(objId))) {
		ui->btnSave->setEnabled(false);
		ui->btnDelete->setEnabled(false);
		return;
	}
	// If project done can't change anything
	if (!db->getSessionActive(session)) {
		ui->btnSave->setEnabled(false);
		ui->btnDelete->setEnabled(false);
	}
	handleBrightnessSlider();
}


/*
 * Handle Save Buttons
 * TODO: Put all in one save routine
 * TODO: PUT ALL IN ONE SAVE ROUTINE!
 */

void MainWindow::handleSaveButton() {
	qDebug() << "Trying to save as user: " << curObj->usr;
//	QString objId = ui->tblObjects->item(currentRow, 0)->text();

	curObj->type = ui->wdgTabTypes->currentWidget()->property("dbvalue").toString();

	if(curObj->type == "BIRD") {
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
	}else if (curObj->type == "MAMMAL") {
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
		curObj->name = ui->cmbMammal->currentText();
		curObj->quality = ui->btngMammalQual->checkedButton()->property("dbvalue").toInt();

		if (ui->gbxMammalBehaviour->isChecked()) {
			curObj->behavior = ui->btngMammalBhv->checkedButton()->property("dbvalue").toString();
		} else {
			curObj->behavior = "";
		}
		if (ui->gbxMammalAge->isChecked()) {
			curObj->age = ui->btngMammalAge->checkedButton()->property("dbvalue").toString();
		} else {
			curObj->age = "";
		}
		curObj->gender = "";
		curObj->remarks = ui->txtMammalRemarks->toPlainText();
	} else if (curObj->type == "NOSIGHT") {
		curObj->name = "";
		curObj->quality = ui->btngNoSightQual->checkedButton()->property("dbvalue").toInt();
		curObj->behavior = "";
		curObj->age = "";
		curObj->gender = "";
		curObj->remarks = ui->txtNoSightRemarks->toPlainText();
		curObj->direction = -1;
	} else if (curObj->type == "TRASH") {
		curObj->name = "";
		curObj->quality = ui->btngTrashQual->checkedButton()->property("dbvalue").toInt();
		curObj->behavior = "";
		curObj->age = "";
		curObj->gender = "";
		curObj->remarks = ui->pteTrashRemarks->toPlainText();
		curObj->direction = -1;
	} else if (curObj->type == "ANTHRO") {
		curObj->name = "";
		curObj->quality = ui->btngAnthroQual->checkedButton()->property("dbvalue").toInt();
		curObj->behavior = "";
		curObj->age = "";
		curObj->gender = "";
		curObj->remarks = ui->pteAnthroRemarks->toPlainText();
		curObj->direction = -1;
	} else {
		qDebug() << "Invalid save type. Aborting.";
		return;
	}


	int tmpcensor = 0;
	if (db->getMaxCensor(QString::number(curObj->id), curObj->usr) >= 2)
		tmpcensor = 0;
	else if (db->getMaxCensor(QString::number(curObj->id), curObj->usr) == 1) {
		if (db->getCensorCount(QString::number(curObj->id), "1", cfg->user()) > 1) {
			tmpcensor = 3;
		} else {
			tmpcensor = 2;
		}
	} else if (db->getMaxCensor(QString::number(curObj->id), curObj->usr) < 1) {
		tmpcensor = 1;
	} else {
		tmpcensor = -1;
	}

	switch (tmpcensor) {
		case -1: {
			qDebug() << "Kann Nuzter nicht bestimmen!";
			curObj->censor = -1;
			break;
		} case 0: {
			qDebug() << "Zusätzlicher Bestimmer.";
			curObj->censor = 0;
			QMessageBox * msgBox = new QMessageBox();
			msgBox->setText(trUtf8("Objekt bereits Endbestimmt. Abspeichern als zusätzliche Bestimmung."));
			msgBox->addButton(trUtf8("Ok"), QMessageBox::YesRole);
			msgBox->exec();
			delete msgBox;
			break;
		} case 1: {
			qDebug() << "Erster Bestimmer.";
			curObj->censor = 1;
			break;
		} case 2: {
			qDebug() << "Zweiter Bestimmer.";
			curObj->censor = 2;
			census * cenObj = db->getCensusData(QString::number(curObj->id));
			bool agree = compareResults(curObj, cenObj);
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
			break;
		} case 3: {
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
			break;
		} default: {
			qDebug() << "Exit route on switch!";
			exit(1);
		}
	}

	if (ui->chbImgQuality->isChecked()) {
		curObj->imageQuality = 1;
	} else {
		curObj->imageQuality = 0;
	}

	// write object data to db
	if (!db->writeCensus(curObj)) {
		QMessageBox * msgBox = new QMessageBox();
		msgBox->setText(QString::fromUtf8("Fehler beim schreiben in die Datenbank."
				"Der Datensatz wurde möglicherweise nicht gespeichert."));
		msgBox->addButton(trUtf8("Ok"), QMessageBox::YesRole);
		msgBox->exec();
		delete msgBox;
		return;
	}
	// refresh object table
	if (curObj->censor != 1)
		colorTableRow(Qt::green, currentRow);
	else
		colorTableRow(Qt::yellow, currentRow);
	if (ui->tblObjects->item(currentRow, 4)->text() == "")
		ui->tblObjects->item(currentRow, 4)->setText(curObj->type);
	else
		ui->tblObjects->item(currentRow, 4)->setText(
			ui->tblObjects->item(currentRow, 4)->text() + "," + curObj->type);
	// delete object structure
	delete curObj;
	ui->btnSave->setEnabled(false);
	ui->btnDelete->setEnabled(false);
	// select next object in table
	if(currentRow < ui->tblObjects->rowCount()) {
		QModelIndex newIndex = objSelector->model()->index(currentRow+1, 0);
		ui->tblObjects->scrollTo(newIndex);
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
void MainWindow::colorTableRow(QColor color, int row) {
	int c = ui->tblObjects->columnCount();
	for (int i=0; i<c; i++) {
		ui->tblObjects->item(row, i)->setBackgroundColor(color);
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

	// clear remark boxes
	ui->txtBirdRemarks->clear();
	ui->txtMammalRemarks->clear();
	ui->txtNoSightRemarks->clear();
	ui->pteTrashRemarks->clear();
	ui->pteAnthroRemarks->clear();

	// clear size box
	ui->lblMammalSizeLength->clear();
	ui->lblBirdSizeLength->clear();
	ui->lblBirdSizeSpan->clear();

	// Recalculate values of the QDial to 0=North
	qDebug() << "Dir: " << cobj->direction;
	if (cobj->direction >= 0 ) {
		dirDial->setValue((cobj->direction+180)%360);
		handleDirDial();
	} else {
		dirDial->setValue(180);
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
			if (cobj->length > 0 ) ui->lblBirdSizeLength->setText(QString::number(cobj->length));
			if (cobj->length > 0 ) ui->lblBirdSizeSpan->setText(QString::number(cobj->span));
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
			if (cobj->length > 0 )ui->lblMammalSizeLength->setText(QString::number(cobj->length));
			ui->cmbMammal->setFocus();
		} else if (shTp == "T" ) { // Trash Tab
			ui->wdgTabTypes->setCurrentIndex(3);
			selectButtonByString(ui->btngTrashQual, QString::number(cobj->quality));
			ui->pteTrashRemarks->setPlainText(cobj->remarks);
		} else if (shTp == "A" ) { // Anthro Tab
			ui->wdgTabTypes->setCurrentIndex(4);
			selectButtonByString(ui->btngAnthroQual, QString::number(cobj->quality));
			ui->pteAnthroRemarks->setPlainText(cobj->remarks);
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

void MainWindow::initFilters() {
	ui->tblFilters->setHorizontalHeaderLabels(QStringList() << "ID" << "IMG" << "CAM" << "Typ" <<
				"Bestimmung");
	ui->tblFilters->horizontalHeader()->setHighlightSections(false);
	ui->tblFilters->setSelectionMode(QAbstractItemView::NoSelection);

	ui->tblFilters->setColumnWidth(0, 75);
	ui->tblFilters->setColumnWidth(1, 80);
	ui->tblFilters->setColumnWidth(2, 40);
	ui->tblFilters->setColumnWidth(3, 50);
	ui->tblFilters->setColumnWidth(4, 80);

	ui->tblFilters->horizontalHeader()->setStretchLastSection(true);

//	ui->tblFilters->horizontalHeaderItem(0)->setData()

	ui->tblFilters->setRowCount(1);

	cmbFilterCam = new QComboBox();
	cmbFilterType = new QComboBox();
	cmbFilterCensus = new QComboBox();
	pteFilterImg = new QLineEdit();
	pteFilterId = new QLineEdit();


	ui->tblFilters->setCellWidget(0,0,pteFilterId);
	ui->tblFilters->setCellWidget(0,1,pteFilterImg);
	ui->tblFilters->setCellWidget(0,2,cmbFilterCam);
	ui->tblFilters->setCellWidget(0,3,cmbFilterType);
	ui->tblFilters->setCellWidget(0,4,cmbFilterCensus);

	ui->cmbFilterCensor->addItem(trUtf8(""), QVariant(""));
	ui->cmbFilterCensor->addItem(trUtf8("Unbearbeitet"),
			QVariant(" AND tp IS NULL AND (mc<2 OR mc IS NULL)"));
	ui->cmbFilterCensor->addItem(trUtf8("Bearbeitet"),QVariant(" AND tp IS NOT NULL"));
	ui->cmbFilterCensor->addItem(trUtf8("Endbestimmt"),QVariant(" AND mc>1"));
	ui->cmbFilterCensor->addItem(trUtf8("Unstimmigkeiten"),QVariant(" AND mc=1 AND cnt>1"));

	cmbFilterCam->addItem(trUtf8(""), QVariant(""));
	cmbFilterCam->addItem(trUtf8("1"), QVariant(" AND cam='1'"));
	cmbFilterCam->addItem(trUtf8("2"), QVariant(" AND cam='2'"));

	cmbFilterType->addItem(trUtf8(""), QVariant(""));
	cmbFilterType->addItems(db->getTypeList());

	cmbFilterCensus->addItem(trUtf8(""), QVariant(""));
	cmbFilterCensus->addItems(db->getCensusList());

	connect(pteFilterImg, SIGNAL(returnPressed()), this, SLOT(handleLineEditFilter()));
	connect(pteFilterId, SIGNAL(returnPressed()), this, SLOT(handleLineEditFilter()));
	connect(cmbFilterCam, SIGNAL(currentIndexChanged(int)), this, SLOT(handleCamFilter(int)));
	connect(cmbFilterType, SIGNAL(currentIndexChanged(int)), this, SLOT(handleTypeFilter(int)));
	connect(cmbFilterCensus, SIGNAL(currentIndexChanged(int)), this, SLOT(handleCensusFilter(int)));
	connect(ui->cmbFilterCensor, SIGNAL(currentIndexChanged(int)), this, SLOT(handleCensorFilter(int)));
}

void MainWindow::handleLineEditFilter() {
	if (pteFilterImg->text().isEmpty())
		filterMap["Img"] = " AND TRUE";
	else
		filterMap["Img"] = " AND img like '%" + pteFilterImg->text() + "'";
	if (pteFilterId->text().isEmpty())
		filterMap["Id"] = " AND TRUE";
	else
		filterMap["Id"] = " AND ot.rcns_id=" + pteFilterId->text() + "";
	populateObjectTable();
}

void MainWindow::handleCensorFilter(int index) {
	filterMap["Censor"] = ui->cmbFilterCensor->itemData(index).toString();
	populateObjectTable();
}

void MainWindow::handleTypeFilter(int index) {
	if (index > 0)
		filterMap["Type"] = " AND pre_tp ='" + cmbFilterType->currentText() + "'";
	else
		filterMap["Type"] = "";
	populateObjectTable();
}

void MainWindow::handleCensusFilter(int index) {
	if (index > 0)
		filterMap["Census"] = " AND otp LIKE '%" + cmbFilterCensus->currentText() + "%'";
	else
		filterMap["Census"] = "";
	populateObjectTable();
}

void MainWindow::handleCamFilter(int index) {
	filterMap["Cam"] = cmbFilterCam->itemData(index).toString();
	populateObjectTable();
}

void MainWindow::handleDeleteButton() {
	db->deleteCensusData(QString::number(curObj->id), cfg->user());
	ui->btnDelete->setEnabled(false);
	populateObjectTable();
}

bool MainWindow::compareResults(census * curObj, census * cenObj) {
	bool agree = true;
	agree = agree && (curObj->name == cenObj->name);
	agree = agree && (curObj->type == cenObj->type);
	if (curObj->quality == 4 || cenObj->quality == 4)
		agree = agree && (curObj->quality == cenObj->quality);
	return agree;
}

void MainWindow::handleSortingHeader(int section) {
	if (sortSet.contains(section)) {
		ui->tblObjects->sortByColumn(section, Qt::DescendingOrder);
		sortSet.remove(section);
	} else {
		ui->tblObjects->sortByColumn(section, Qt::AscendingOrder);
		sortSet.insert(section);
	}
}

void MainWindow::handleBirdSpanMeasurement() {
	if (!msm_running) {
		imgcvs->beginMeasurement();
		ui->lblBirdSizeSpan->clear();
		curObj->span = -1.0;
		msm_running = true;
	} else {
		curObj->span = imgcvs->endMeasurement();
		ui->lblBirdSizeSpan->setText(QString::number(curObj->span));
		msm_running = false;
	}

};

void MainWindow::handleBirdLengthMeasurement() {
	if (!msm_running) {
		imgcvs->beginMeasurement();
		ui->lblBirdSizeLength->clear();
		curObj->length = -1.0;
		msm_running = true;
	} else {
		curObj->length = imgcvs->endMeasurement();
		ui->lblBirdSizeLength->setText(QString::number(curObj->length));
		msm_running = false;
	}
};

void MainWindow::handleMammalLengthMeasurement() {
	if (!msm_running) {
		imgcvs->beginMeasurement();
		ui->lblMammalSizeLength->clear();
		curObj->length=-1.0;
		msm_running = true;
	} else {
		curObj->length = imgcvs->endMeasurement();
		ui->lblMammalSizeLength->setText(QString::number(curObj->length));
		msm_running = false;
	}
};



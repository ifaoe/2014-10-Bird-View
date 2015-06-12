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
#include "Stuk4Dialog.h"
#include "GroupSelection.h"

MainWindow::MainWindow( ConfigHandler *cfgArg, DatabaseHandler *dbArg, QWidget *parent) :
	QMainWindow(0), ui(new Ui::MainWindow), cfg(cfgArg), db(dbArg)
{
	Q_UNUSED(parent);
	ui->setupUi(this);

	initCollapsibleMenu();
	initFilters();

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

	wdgSession->cmbSession->addItems(db->getSessionList());
	wdgCensus->cmbBird->addItems(db->getBirdTypeList());
	wdgCensus->cmbMammal->addItems(db->getMammalTypeList());
	db->getAnthroObjectList(wdgCensus->cmbAnthroName);

	objSelector = wdgObjects->tblObjects->selectionModel();
	wdgObjects->tblObjects->setSelectionMode(QAbstractItemView::SingleSelection);
	wdgObjects->tblObjects->setSelectionBehavior(QAbstractItemView::SelectRows);

	initMapView();

	measurementWindow = new MeasurementDialog(imgcvs);

    connect( objSelector, SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
    		this, SLOT(objectUpdateSelection()));
    connect(wdgSession->btnSession, SIGNAL(released()), this, SLOT(populateObjectTable()));
//    connect(btnMapModeImg , SIGNAL(released()), this, SLOT(handleMapToolButton()));
//    connect(btnMapModeGeo , SIGNAL(released()), this, SLOT(handleMapToolButton()));
//    connect(ui->actionMapView, SIGNAL(triggered()), this, SLOT(handleMapToolButton()));
    connect(ui->btnMapView, SIGNAL(clicked()), this, SLOT(handleMapToolButton()));
    connect(ui->btnZoomOneOne, SIGNAL(clicked()), this, SLOT(handleOneToOneZoom()));
//    connect(ui->action1_1_Zoom, SIGNAL(triggered()), this, SLOT(handleOneToOneZoom()));
    connect(dirDial, SIGNAL(sliderReleased()), this, SLOT(handleDirDial()));
    connect(wdgCensus->btnUserSelect, SIGNAL(released()), this, SLOT(handleUsrSelect()));
    connect(wdgGraphics->sldBrightness, SIGNAL(sliderReleased()),
    		this, SLOT(handleBrightnessSlider()));
    connect(wdgGraphics->sldContrast, SIGNAL(sliderReleased()),
    		this, SLOT(handleContrastSlider()));
    connect(wdgGraphics->btnBrightnessReset, SIGNAL(clicked()),
    		this, SLOT(handleBrightnessReset()));
    connect(wdgGraphics->btnContrastReset, SIGNAL(clicked()), this, SLOT(handleContrastReset()));

    connect(wdgCensus->btnSave, SIGNAL(released()), this, SLOT(handleSaveButton()));

    connect(wdgCensus->btnDelete, SIGNAL(released()), this, SLOT(handleDeleteButton()));

    connect(wdgObjects->tblFilters->horizontalHeader(), SIGNAL(sectionClicked(int)), this,
    		SLOT(handleSortingHeader(int)));

    connect(wdgCensus->tbtStuk4CodesBird, SIGNAL(clicked()), this, SLOT(handleStuk4Selection()));
    connect(wdgCensus->tbtStuk4CodesMammal, SIGNAL(clicked()), this, SLOT(handleStuk4Selection()));

    connect(wdgCensus->btnBirdSizeSpan, SIGNAL(clicked()), this, SLOT(handleBirdSpanMeasurement()));
    connect(wdgCensus->btnBirdSizeLength, SIGNAL(clicked()), this, SLOT(handleBirdLengthMeasurement()));
    connect(wdgCensus->btnMammalSizeLength, SIGNAL(clicked()), this, SLOT(handleMammalLengthMeasurement()));


    connect(wdgCensus->tbtGroupsMammal, SIGNAL(clicked()), this, SLOT(handleGroupSelection()));
    connect(wdgCensus->tbtGroupsBird, SIGNAL(clicked()), this, SLOT(handleGroupSelection()));

    connect(ui->btnMiscMeasurement, SIGNAL(clicked()), this, SLOT(handleMiscMeasurement()));

    wdgCensus->btnBirdSizeLength->setEnabled(false);
    wdgCensus->btnBirdSizeSpan->setEnabled(false);
    wdgCensus->btnMammalSizeLength->setEnabled(false);

    ui->statusBar->showMessage("Bereit. Kein Objekt geladen.");

}

MainWindow::~MainWindow()
{
    delete ui;
    measurementWindow->close();
    delete measurementWindow;
}

void MainWindow::populateObjectTable() {
	sortSet.clear();
	wdgCensus->btnSave->setEnabled(false);
	wdgCensus->btnDelete->setEnabled(false);
	QString filter = "WHERE TRUE" + QStringList(filterMap.values()).join("");
	session = wdgSession->cmbSession->currentText();
	currentRow = -1;
	objSelector->clearSelection();
	wdgObjects->tblObjects->clear();
	wdgObjects->tblObjects->model()->removeRows(0,wdgObjects->tblObjects->rowCount());
	wdgObjects->tblObjects->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);

	QSqlQuery *query = db->getObjectResult( session, cfg->user(), filter);
	int row = 0;
	QMap<int, QString> usrCensus = db->getUserCensus(cfg->user(), session);
	QMap<int, QString> finalCensus = db->getFinalCensus(session);

	while(query->next()) {
		wdgObjects->tblObjects->insertRow( wdgObjects->tblObjects->rowCount() );
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


		id->setTextAlignment(Qt::AlignCenter);
		type->setTextAlignment(Qt::AlignCenter);
		cam->setTextAlignment(Qt::AlignCenter);
		img->setTextAlignment(Qt::AlignCenter);
		census->setTextAlignment(Qt::AlignCenter);

		id->setFlags(id->flags() & ~Qt::ItemIsEditable);
		type->setFlags(id->flags() & ~Qt::ItemIsEditable);
		cam->setFlags(id->flags() & ~Qt::ItemIsEditable);
		img->setFlags(id->flags() & ~Qt::ItemIsEditable);
		census->setFlags(id->flags() & ~Qt::ItemIsEditable);

		wdgObjects->tblObjects->setItem(row,0,id);
		wdgObjects->tblObjects->setItem(row,1,img);
		wdgObjects->tblObjects->setItem(row,2,cam);
		wdgObjects->tblObjects->setItem(row,3,type);
		wdgObjects->tblObjects->setItem(row,4,census);


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
	if (wdgObjects->tblObjects->rowCount() > 0) {
		wdgObjects->tblObjects->setMinimumSize(wdgObjects->tblObjects->width(), 50);
		wdgObjects->tblObjects->setMaximumSize(wdgObjects->tblObjects->width(),
				wdgObjects->tblObjects->rowHeight(0)*6);
		wdgObjects->wdgObjectsTable->adjustSize();
	} else {
		wdgObjects->wdgObjectsTable->adjustSize();
	}

	if (!cbtSession->isChecked())
		twgSession->setExpanded(false);
	if (!cbtObjects->isChecked())
		twgObjects->setExpanded(true);
}

void MainWindow::objectUpdateSelection() {
	wdgGraphics->sldBrightness->setValue(0);
	wdgGraphics->sldContrast->setValue(0);

	dialChecked = false;
	if (objSelector->selectedRows().isEmpty()) return;
	currentRow = objSelector->selectedRows().at(0).row();
	QString objId = wdgObjects->tblObjects->item(currentRow, 0)->text();
	QString cam = wdgObjects->tblObjects->item(currentRow, 2)->text();
	QString img = wdgObjects->tblObjects->item(currentRow, 1)->text();
	QString type = wdgObjects->tblObjects->item(currentRow, 3)->text();
	wdgCensus->cmbUsers->clear();
	curObj = db->getRawObjectData(objId, cfg->user());

	if (curObj->type.isEmpty()) curObj->type = type;
	censorList = db->getUserList(objId);
	wdgCensus->cmbUsers->addItems(censorList);
	uiPreSelection(curObj);

	// handle user selection
	if ((curObj->censor > 0) && (db->getMaxCensor(QString::number(curObj->id),cfg->user()) > 1)) {
		wdgCensus->btnDelete->setEnabled(false);
		wdgCensus->btnSave->setEnabled(false);
	} else {
		wdgCensus->btnDelete->setEnabled(true);
		wdgCensus->btnSave->setEnabled(true);
	}
	if (curObj->censor < 0)
		wdgCensus->btnDelete->setEnabled(false);

	if (db->getCensorCount(QString::number(curObj->id), "1", cfg->user()) >= 2
			|| db->getMaxCensor(QString::number(curObj->id)) >= 2) {
		wdgCensus->cmbUsers->setDisabled(false);
		wdgCensus->btnUserSelect->setDisabled(false);
	} else {
		wdgCensus->cmbUsers->clear();
		wdgCensus->cmbUsers->setDisabled(true);
		wdgCensus->btnUserSelect->setDisabled(true);
	}

	if (!imgcvs->loadObject(curObj)) {
		wdgCensus->btnSave->setEnabled(false);
		wdgCensus->btnDelete->setEnabled(false);
		wdgCensus->btnBirdSizeLength->setEnabled(false);
		wdgCensus->btnBirdSizeSpan->setEnabled(false);
		wdgCensus->btnMammalSizeLength->setEnabled(false);
		return;
	}
	wdgCensus->btnBirdSizeLength->setEnabled(true);
	wdgCensus->btnBirdSizeSpan->setEnabled(true);
	wdgCensus->btnMammalSizeLength->setEnabled(true);
	// If project done can't change anything
	if (!db->getSessionActive(session)) {
		wdgCensus->btnSave->setEnabled(false);
		wdgCensus->btnDelete->setEnabled(false);
	}
	handleBrightnessSlider();
	if (!cbtObjects->isChecked())
		twgObjects->setExpanded(false);
	if (!cbtCensus->isChecked())
		twgCensus->setExpanded(true);


	ui->wdgFrameTree->scrollToItem(twgCensus);

//	wdgMultiCensus->tbvMultiCensus->setModel(db->getImageObjects(curObj));
//	wdgMultiCensus->tbvMultiCensus->selectionModel()->clearSelection();
//
//	for(int i=0; i<wdgMultiCensus->tbvMultiCensus->model()->rowCount(); i++) {
//		QModelIndex ind = wdgMultiCensus->tbvMultiCensus->model()->index(i,0);
//		if (wdgMultiCensus->tbvMultiCensus->model()->data(ind).toInt() == curObj->id) {
//			wdgMultiCensus->tbvMultiCensus->selectRow(i);
//			break;
//		}
//	}

	ui->statusBar->showMessage(
			QString("Project: %1, Kamera: %2, Bild: %3, Objekt ID: %4")
			.arg(curObj->session).arg(curObj->camera).arg(curObj->image).arg(curObj->id)
			);
}


/*
 * Handle Save Buttons
 * TODO: Put all in one save routine
 * TODO: PUT ALL IN ONE SAVE ROUTINE!
 */

void MainWindow::handleSaveButton() {
	qDebug() << "Trying to save as user: " << curObj->usr;

	curObj->type = wdgCensus->wdgTabTypes->currentWidget()->property("dbvalue").toString();

	if(curObj->type == "BIRD") {
		if ((wdgCensus->btngBirdBeh->checkedButton()->property("dbvalue").toString() == "FLY")
				&& (dialChecked == false)) {
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
			} else if (wdgCensus->btngBirdBeh->checkedButton()->property("dbvalue").toString() != "FLY") {
				curObj->direction = -1;
			} else {

			}
			if (wdgCensus->cmbBird->currentText() == "") {
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
			curObj->quality = wdgCensus->btngBirdQual->checkedButton()->property("dbvalue").toInt();
			curObj->behavior = wdgCensus->btngBirdBeh->checkedButton()->property("dbvalue").toString();
			if (wdgCensus->gbxBirdGender->isChecked()) {
				curObj->gender = wdgCensus->btngBirdSex->checkedButton()->property("dbvalue").toString();
			} else {
				curObj->gender = "";
			}
			if (wdgCensus->gbxBirdAge->isChecked()) {
				curObj->age = wdgCensus->btngBirdAge->checkedButton()->property("dbvalue").toString();
			} else {
				curObj->age = "";
			}

			curObj->remarks = wdgCensus->txtBirdRemarks->toPlainText();
			curObj->name = wdgCensus->cmbBird->currentText();
	}else if (curObj->type == "MAMMAL") {
		if (wdgCensus->cmbMammal->currentText() == "") {
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
		curObj->name = wdgCensus->cmbMammal->currentText();
		curObj->quality = wdgCensus->btngMammalQual->checkedButton()->property("dbvalue").toInt();
		curObj->behavior = wdgCensus->btngMammalBeh->checkedButton()->property("dbvalue").toString();
		if (wdgCensus->gbxMammalAge->isChecked()) {
			curObj->age = wdgCensus->btngMammalAge->checkedButton()->property("dbvalue").toString();
		} else {
			curObj->age = "";
		}
		curObj->gender = "";
		curObj->remarks = wdgCensus->txtMammalRemarks->toPlainText();
	} else if (curObj->type == "NOSIGHT") {
		curObj->name = "";
		curObj->quality = wdgCensus->btngNoSightQual->checkedButton()->property("dbvalue").toInt();
		curObj->behavior = "";
		curObj->age = "";
		curObj->gender = "";
		curObj->remarks = wdgCensus->txtNoSightRemarks->toPlainText();
		curObj->direction = -1;
	} else if (curObj->type == "TRASH") {
		curObj->name = "";
		curObj->quality = wdgCensus->btngTrashQual->checkedButton()->property("dbvalue").toInt();
		curObj->behavior = "";
		curObj->age = "";
		curObj->gender = "";
		curObj->remarks = wdgCensus->pteTrashRemarks->toPlainText();
		curObj->direction = -1;
	} else if (curObj->type == "ANTHRO") {
		curObj->name = wdgCensus->cmbAnthroName->itemData(
				wdgCensus->cmbAnthroName->currentIndex()).toString();
		curObj->quality = wdgCensus->btngAnthroQual->checkedButton()->property("dbvalue").toInt();
		curObj->behavior = "";
		curObj->age = "";
		curObj->gender = "";
		curObj->remarks = wdgCensus->pteAnthroRemarks->toPlainText();
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

	if (wdgGraphics->chbImgQuality->isChecked()) {
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
	if (wdgObjects->tblObjects->item(currentRow, 4)->text() == "")
		wdgObjects->tblObjects->item(currentRow, 4)->setText(curObj->type);
	else
		wdgObjects->tblObjects->item(currentRow, 4)->setText(
			wdgObjects->tblObjects->item(currentRow, 4)->text() + "," + curObj->type);
	// delete object structure
	delete curObj;
	wdgCensus->btnSave->setEnabled(false);
	wdgCensus->btnDelete->setEnabled(false);
	// select next object in table
	if(currentRow < wdgObjects->tblObjects->rowCount()) {
		QModelIndex newIndex = objSelector->model()->index(currentRow+1, 0);
		wdgObjects->tblObjects->scrollTo(newIndex);
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
	url += "http://www.openstreetmap.org/?mlat="
			+ QString::number(curObj->ly) + "&mlon=" + QString::number(curObj->lx);
	url += "#map=" + scale + "/" + QString::number(curObj->ly) + "/" + QString::number(curObj->lx);

	if (ui->btnMapView->isChecked()) {
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
	int c = wdgObjects->tblObjects->columnCount();
	for (int i=0; i<c; i++) {
		wdgObjects->tblObjects->item(row, i)->setBackgroundColor(color);
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
	imgcvs = new ImgCanvas(ui->wdgImg, ui, cfg, db);
	geoMap = new QWebView(ui->wdgImg);
	lytFrmImg = new QVBoxLayout;
	lytFrmImg->setMargin(0);
	lytFrmImg->addWidget(imgcvs);
	ui->wdgImg->setLayout(lytFrmImg);

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

	// handle those differently -- courtesy of the stuk4 table widget
	curObj->stuk4_ass = cobj->stuk4_ass;
	curObj->stuk4_beh = cobj->stuk4_beh;

	// clear remark boxes
	wdgCensus->txtBirdRemarks->clear();
	wdgCensus->txtMammalRemarks->clear();
	wdgCensus->txtNoSightRemarks->clear();
	wdgCensus->pteTrashRemarks->clear();
	wdgCensus->pteAnthroRemarks->clear();

	// clear size box
	wdgCensus->lblMammalSizeLength->clear();
	wdgCensus->lblBirdSizeLength->clear();
	wdgCensus->lblBirdSizeSpan->clear();

	//clear Stuk4 Code Labels
	wdgCensus->lblStuk4BehBird->setText("Verhalten:");
	wdgCensus->lblStuk4AssBird->setText("Assoziationen:");
	wdgCensus->lblStuk4BehMammal->setText("Verhalten:");
	wdgCensus->lblStuk4AssMammal->setText("Assoziationen:");

	//clear group list labels
	wdgCensus->lblGroupBirdObjects->setText("Gruppe:");
	wdgCensus->lblGroupMammalObjects->setText("Gruppe:");

	wdgCensus->lblFamilyBird->setText("Familienv.:");
	wdgCensus->lblFamilyMammal->setText("Familienv.:");

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
		wdgGraphics->chbImgQuality->setChecked(true);
	} else {
		wdgGraphics->chbImgQuality->setChecked(false);
	}

	// Find out which type is listed and switch to the corresponding tab
//	QString shTp = wdgObjects->tblObject->item(wdgObjects->tblObject->currentRow(),3)->text().left(1);
	QString shTp = cobj->type.left(1);
	if (shTp == "" ) {
		shTp = wdgObjects->tblObjects->item(wdgObjects->tblObjects->currentRow(),3)->text().left(1);
	}
	// by default, all options are off
	wdgCensus->gbxMammalAge->setChecked(false);
	wdgCensus->gbxBirdAge->setChecked(false);
	wdgCensus->gbxBirdGender->setChecked(false);

	if(shTp == "B" || shTp == "V" ) { // Bird Tab
			wdgCensus->wdgTabTypes->setCurrentIndex(0);
			int index = wdgCensus->cmbBird->findText(cobj->name);
			wdgCensus->cmbBird->setCurrentIndex(index);
			selectButtonByString(wdgCensus->btngBirdQual, QString::number(cobj->quality));
			selectButtonByString(wdgCensus->btngBirdBeh, cobj->behavior);
			if(cobj->gender != "") {
				wdgCensus->gbxBirdGender->setChecked(true);
				selectButtonByString(wdgCensus->btngBirdSex, cobj->gender);
			}
			if(cobj->age != "") {
				wdgCensus->gbxBirdAge->setChecked(true);
				selectButtonByString(wdgCensus->btngBirdAge, cobj->age);
			}
			wdgCensus->txtBirdRemarks->setPlainText(cobj->remarks);
			if (cobj->length > 0 ) wdgCensus->lblBirdSizeLength->setText(QString::number(cobj->length));
			if (cobj->span > 0 ) wdgCensus->lblBirdSizeSpan->setText(QString::number(cobj->span));

			wdgCensus->lblStuk4BehBird->setText("Verhalten: " + cobj->stuk4_beh.join(", "));
			wdgCensus->lblStuk4AssBird->setText("Assoziationen: " + cobj->stuk4_ass.join(", "));
			wdgCensus->lblGroupBirdObjects->setText("Gruppe: " + cobj->group.join(", "));
			wdgCensus->lblFamilyBird->setText("Familienv.: " + cobj->family.join(", "));

			wdgCensus->cmbBird->setFocus();
		} else if (shTp == "M" ) { // Mammal Tab
			wdgCensus->wdgTabTypes->setCurrentIndex(1);
			int index = wdgCensus->cmbMammal->findText(cobj->name);
			wdgCensus->cmbMammal->setCurrentIndex(index);
			selectButtonByString(wdgCensus->btngMammalQual, QString::number(cobj->quality));
			selectButtonByString(wdgCensus->btngMammalBeh, cobj->behavior);
			if (cobj->age != "") {
				wdgCensus->gbxMammalAge->setChecked(true);
				selectButtonByString(wdgCensus->btngMammalAge, cobj->age);
			}
			wdgCensus->txtMammalRemarks->setPlainText(cobj->remarks);
			if (cobj->length > 0 )
				wdgCensus->lblMammalSizeLength->setText(QString::number(cobj->length));
			wdgCensus->lblStuk4BehMammal->setText("Verhalten: " + cobj->stuk4_beh.join(", "));
			wdgCensus->lblStuk4AssMammal->setText("Assoziationen: " + cobj->stuk4_ass.join(", "));
			wdgCensus->lblGroupMammalObjects->setText("Gruppe: " +cobj->group.join(", "));
			wdgCensus->lblFamilyMammal->setText("Familienv.: " + cobj->family.join(", "));

			wdgCensus->cmbMammal->setFocus();
		} else if (shTp == "T" ) { // Trash Tab
			wdgCensus->wdgTabTypes->setCurrentIndex(3);
			selectButtonByString(wdgCensus->btngTrashQual, QString::number(cobj->quality));
			wdgCensus->pteTrashRemarks->setPlainText(cobj->remarks);
		} else if (shTp == "A" ) { // Anthro Tab
			int index = wdgCensus->cmbAnthroName->findText(cobj->name);
			wdgCensus->cmbAnthroName->setCurrentIndex(index);
			wdgCensus->wdgTabTypes->setCurrentIndex(4);
			selectButtonByString(wdgCensus->btngAnthroQual, QString::number(cobj->quality));
			wdgCensus->cmbAnthroName->findText(cobj->name, Qt::MatchStartsWith);
			wdgCensus->pteAnthroRemarks->setPlainText(cobj->remarks);
		} else { //NoSighting tab
			wdgCensus->wdgTabTypes->setCurrentIndex(2);
			selectButtonByString(wdgCensus->btngNoSightQual, QString::number(cobj->quality));
			wdgCensus->txtNoSightRemarks->setPlainText(cobj->remarks);
		}
}

/*
 * Read results for the selected censor and set the Ui elements respectively
 */
void MainWindow::handleUsrSelect() {
	census * obj;
	if (wdgCensus->cmbUsers->currentIndex() == -1) return;
	obj = db->getRawObjectData(QString::number(curObj->id), wdgCensus->cmbUsers->currentText());
	uiPreSelection(obj);
	delete obj;
}

void MainWindow::handleBrightnessSlider() {
	imgcvs->setRasterBrightness(wdgGraphics->sldBrightness->value());
}

void MainWindow::handleContrastSlider() {
	imgcvs->setRasterContrast(wdgGraphics->sldContrast->value());
}

void MainWindow::handleBrightnessReset() {
	wdgGraphics->sldBrightness->setValue(0);
	handleBrightnessSlider();
}

void MainWindow::handleContrastReset() {
	wdgGraphics->sldContrast->setValue(0);
	handleContrastSlider();
}

void MainWindow::initFilters() {
	wdgObjects->tblFilters->setHorizontalHeaderLabels(
			QStringList() << "ID" << "IMG" << "CAM" << "Typ" << "Bestimmung");
	wdgObjects->tblFilters->horizontalHeader()->setHighlightSections(false);
	wdgObjects->tblFilters->setSelectionMode(QAbstractItemView::NoSelection);

	wdgObjects->tblFilters->setColumnWidth(0, 75);
	wdgObjects->tblFilters->setColumnWidth(1, 80);
	wdgObjects->tblFilters->setColumnWidth(2, 40);
	wdgObjects->tblFilters->setColumnWidth(3, 50);
	wdgObjects->tblFilters->setColumnWidth(4, 80);

	wdgObjects->tblFilters->horizontalHeader()->setStretchLastSection(true);

	wdgObjects->tblFilters->setRowCount(1);
	wdgObjects->tblFilters->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);

	cmbFilterCam = new QComboBox();
	cmbFilterType = new QComboBox();
	cmbFilterCensus = new QComboBox();
	pteFilterImg = new QLineEdit();
	pteFilterId = new QLineEdit();


	wdgObjects->tblFilters->setCellWidget(0,0,pteFilterId);
	wdgObjects->tblFilters->setCellWidget(0,1,pteFilterImg);
	wdgObjects->tblFilters->setCellWidget(0,2,cmbFilterCam);
	wdgObjects->tblFilters->setCellWidget(0,3,cmbFilterType);
	wdgObjects->tblFilters->setCellWidget(0,4,cmbFilterCensus);

	wdgObjects->cmbFilterUserCensor->addItem(trUtf8(""), QVariant(""));
	wdgObjects->cmbFilterUserCensor->addItem(trUtf8("Unbearbeitet"),
			QVariant(" AND tp IS NULL AND (mc<2 OR mc IS NULL)"));
	wdgObjects->cmbFilterUserCensor->addItem(trUtf8("Bearbeitet"),QVariant(" AND tp IS NOT NULL"));

	wdgObjects->cmbFilterCensor->addItem(trUtf8(""), QVariant(""));
	wdgObjects->cmbFilterCensor->addItem(trUtf8("Unbestimmt"),QVariant(" AND (mc=0 OR mc IS NULL)"));
//	wdgObjects->cmbFilterCensor->setItemData( 1, QColor( Qt::white ), Qt::BackgroundRole );
	wdgObjects->cmbFilterCensor->addItem(trUtf8("Erstbestimmt"),QVariant(" AND mc=1 AND cnt=1"));
//	wdgObjects->cmbFilterCensor->setItemData( 2, QColor( Qt::gray ), Qt::BackgroundRole );
	wdgObjects->cmbFilterCensor->addItem(trUtf8("Unstimmigkeiten"),QVariant(" AND mc=1 AND cnt>1"));
//	wdgObjects->cmbFilterCensor->setItemData( 3, QColor( Qt::red ), Qt::BackgroundRole );
	wdgObjects->cmbFilterCensor->addItem(trUtf8("Enbestimmt"),QVariant(" AND mc>1"));
//	wdgObjects->cmbFilterCensor->setItemData( 4, QColor( Qt::green ), Qt::BackgroundRole );

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

	connect(wdgObjects->cmbFilterCensor, SIGNAL(currentIndexChanged(int)),
			this, SLOT(handleCensorFilter()));
	connect(wdgObjects->cmbFilterUserCensor, SIGNAL(currentIndexChanged(int)),
			this, SLOT(handleCensorFilter()));

	wdgObjects->tblFilters->setMinimumSize(
			wdgObjects->tblFilters->width(), wdgObjects->tblFilters->rowHeight(0) +
			wdgObjects->tblFilters->horizontalHeader()->height());
	wdgObjects->tblFilters->setMaximumSize(
			wdgObjects->tblFilters->width(), wdgObjects->tblFilters->rowHeight(0) +
			wdgObjects->tblFilters->horizontalHeader()->height());
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

void MainWindow::handleCensorFilter() {
	filterMap["UserCensor"] = wdgObjects->cmbFilterUserCensor->itemData(
			wdgObjects->cmbFilterUserCensor->currentIndex()).toString();
	filterMap["Censor"] = wdgObjects->cmbFilterCensor->itemData(
			wdgObjects->cmbFilterCensor->currentIndex()).toString();
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
	wdgCensus->btnDelete->setEnabled(false);
	populateObjectTable();
}

bool MainWindow::compareResults(census * curObj, census * cenObj) {
	bool agree = true;
	agree = agree && (curObj->name == cenObj->name);
	agree = agree && (curObj->type == cenObj->type);
	if (curObj->quality == 4 || cenObj->quality == 4)
		agree = agree && (curObj->quality == cenObj->quality);
	agree = agree && (curObj->group == cenObj->group);
	agree = agree && (curObj->stuk4_ass == cenObj->stuk4_ass);
	agree = agree && (curObj->stuk4_beh == cenObj->stuk4_beh);
	return agree;
}

void MainWindow::handleSortingHeader(int section) {
	if (sortSet.contains(section)) {
		wdgObjects->tblObjects->sortByColumn(section, Qt::DescendingOrder);
		sortSet.remove(section);
	} else {
		wdgObjects->tblObjects->sortByColumn(section, Qt::AscendingOrder);
		sortSet.insert(section);
	}
}

void MainWindow::handleBirdSpanMeasurement() {
	conductMeasurement(&curObj->span, wdgCensus->lblBirdSizeSpan);
};

void MainWindow::handleBirdLengthMeasurement() {
	conductMeasurement(&curObj->length, wdgCensus->lblBirdSizeLength);
};

void MainWindow::handleMammalLengthMeasurement() {
	conductMeasurement(&curObj->length, wdgCensus->lblMammalSizeLength);
};

void MainWindow::handleStuk4Selection() {
	if(curObj == 0) return;
	Stuk4Dialog * dlg = new Stuk4Dialog(db, &curObj->stuk4_beh, &curObj->stuk4_ass);
	dlg->exec();
	delete dlg;
	if (wdgCensus->wdgTabTypes->currentWidget()->property("dbvalue").toString() == "BIRD") {
		wdgCensus->lblStuk4BehBird->setText("Verhalten: " + curObj->stuk4_beh.join(", "));
		wdgCensus->lblStuk4AssBird->setText("Assoziationen: " + curObj->stuk4_ass.join(", "));
	} else if (wdgCensus->wdgTabTypes->currentWidget()->property("dbvalue").toString() == "MAMMAL") {
		wdgCensus->lblStuk4BehMammal->setText("Verhalten: " + curObj->stuk4_beh.join(", "));
		wdgCensus->lblStuk4AssMammal->setText("Assoziationen: " + curObj->stuk4_ass.join(", "));
	} else {
		return;
	}

};

void MainWindow::handleGroupSelection() {
	if (curObj == 0) return;
	GroupSelection * dlg = new GroupSelection(db, curObj, this);
	dlg->exec();
	delete dlg;
	if (wdgCensus->wdgTabTypes->currentWidget()->property("dbvalue").toString() == "BIRD") {
		wdgCensus->lblGroupBirdObjects->setText("Gruppe: " + curObj->group.join(", "));
		wdgCensus->lblFamilyBird->setText("Familienv.: " + curObj->family.join(", "));
	} else if (wdgCensus->wdgTabTypes->currentWidget()->property("dbvalue").toString() == "MAMMAL") {
		wdgCensus->lblGroupMammalObjects->setText("Gruppe: " + curObj->group.join(", "));
		wdgCensus->lblFamilyMammal->setText("Familienv.: " + curObj->family.join(", "));
	} else {
		return;
	}
	return;
}

void MainWindow::conductMeasurement(double * length, QLabel * label) {
	measurementWindow->move(size().width()*0.75,size().height()*0.25);
	measurementWindow->startMeasurement(length, label);
}

void MainWindow::handleMiscMeasurement() {
	conductMeasurement(0,0);
}

void MainWindow::handleFlightInfoAction() {
}

void MainWindow::initCollapsibleMenu(){
	// create expandable widget type
		ui->wdgFrameTree->setRootIsDecorated(false);
		ui->wdgFrameTree->setIndentation(0);
		ui->wdgFrameTree->viewport()->setBackgroundRole(QPalette::Background);
		ui->wdgFrameTree->setBackgroundRole(QPalette::Window);
		ui->wdgFrameTree->setAutoFillBackground(true);
		ui->wdgFrameTree->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
		//create widget
			{
			twgSession = new QTreeWidgetItem();
			ui->wdgFrameTree->addTopLevelItem(twgSession);
			cbtSession = new QCategoryCheckButton("Projektauswahl", ui->wdgFrameTree, twgSession);
			ui->wdgFrameTree->setItemWidget(twgSession, 0, cbtSession);

			QTreeWidgetItem * wdgContainer = new QTreeWidgetItem();
			wdgContainer->setDisabled(true);
			twgSession->addChild(wdgContainer);
			QFrame *widget = new QFrame;
			wdgSession = new Ui::wdgSessions;
			wdgSession->setupUi(widget);
			widget->setBackgroundRole(QPalette::Window);
			widget->setAutoFillBackground(true);
			ui->wdgFrameTree->setItemWidget(wdgContainer,0,widget);
			twgSession->setExpanded(true);
		}

		//create widget
		{
			twgObjects = new QTreeWidgetItem();
			ui->wdgFrameTree->addTopLevelItem(twgObjects);
			cbtObjects = new QCategoryCheckButton("Objektauswahl", ui->wdgFrameTree, twgObjects);
			ui->wdgFrameTree->setItemWidget(twgObjects, 0, cbtObjects);

			QTreeWidgetItem * wdgContainer = new QTreeWidgetItem();
			wdgContainer->setDisabled(true);
			twgObjects->addChild(wdgContainer);
			QFrame *widget = new QFrame;
			wdgObjects = new Ui::wdgObjects;
			wdgObjects->setupUi(widget);
			widget->setBackgroundRole(QPalette::Window);
			widget->setAutoFillBackground(true);
			ui->wdgFrameTree->setItemWidget(wdgContainer,0,widget);
			twgObjects->setExpanded(false);
		}

		//create widget
		{
			twgCensus = new QTreeWidgetItem();
			ui->wdgFrameTree->addTopLevelItem(twgCensus);
			cbtCensus = new QCategoryCheckButton("Bestimmungstabellen", ui->wdgFrameTree, twgCensus);
			ui->wdgFrameTree->setItemWidget(twgCensus, 0, cbtCensus);
			QTreeWidgetItem * wdgContainer = new QTreeWidgetItem();
			wdgContainer->setDisabled(true);
			twgCensus->addChild(wdgContainer);
			QFrame *widget = new QFrame;
			wdgCensus = new Ui::wdgCensus;
			wdgCensus->setupUi(widget);
			widget->setBackgroundRole(QPalette::Window);
			widget->setAutoFillBackground(true);
			widget->resize(0,0);
			wdgCensus->wdgTabTypes->setContentsMargins(0,0,0,0);
			wdgCensus->wdgTabTypes->setBackgroundRole(QPalette::Window);
			wdgCensus->wdgTabTypes->setAutoFillBackground(true);

			ui->wdgFrameTree->setItemWidget(wdgContainer,0,widget);
			if (!cbtCensus->isChecked())
				twgCensus->setExpanded(false);

		}

//		//create widget
//		{
//			twgMultiCensus = new QTreeWidgetItem();
//			ui->wdgFrameTree->addTopLevelItem(twgMultiCensus);
//			cbtMultiCensus = new QCategoryCheckButton("Mehrfachbestimmung", ui->wdgFrameTree, twgMultiCensus);
//			ui->wdgFrameTree->setItemWidget(twgMultiCensus, 0, cbtMultiCensus);
//			QTreeWidgetItem * wdgContainer = new QTreeWidgetItem();
//			wdgContainer->setDisabled(true);
//			twgMultiCensus->addChild(wdgContainer);
//			QFrame *widget = new QFrame;
//			wdgMultiCensus = new Ui::wdgMultiCensus;
//			wdgMultiCensus->setupUi(widget);
//			widget->setBackgroundRole(QPalette::Window);
//			widget->setAutoFillBackground(true);
//			widget->resize(0,0);
//
//			wdgMultiCensus->tbvMultiCensus->horizontalHeader()->setStretchLastSection(true);
//			wdgMultiCensus->tbvMultiCensus
//				->verticalHeader()->resizeMode(QHeaderView::ResizeToContents);
//
//			ui->wdgFrameTree->setItemWidget(wdgContainer,0,widget);
//			if (!cbtMultiCensus->isChecked())
//				twgMultiCensus->setExpanded(false);
//
//		}

		//create widget
		{
			twgGraphics = new QTreeWidgetItem();
			ui->wdgFrameTree->addTopLevelItem(twgGraphics);
			cbtGraphics = new QCategoryCheckButton("Bildverarbeitung", ui->wdgFrameTree, twgGraphics);
			ui->wdgFrameTree->setItemWidget(twgGraphics, 0, cbtGraphics);
			QTreeWidgetItem * wdgContainer = new QTreeWidgetItem();
			wdgContainer->setDisabled(true);
			twgGraphics->addChild(wdgContainer);
			QFrame *widget = new QFrame;
			wdgGraphics = new Ui::wdgGraphics;
			wdgGraphics->setupUi(widget);
			widget->setBackgroundRole(QPalette::Window);
			widget->setAutoFillBackground(true);
			ui->wdgFrameTree->setItemWidget(wdgContainer,0,widget);
			twgGraphics->setExpanded(false);
		}

		wdgCensus->btnSave->setEnabled(false);
		wdgCensus->btnDelete->setEnabled(false);
		wdgObjects->tblObjects->setColumnCount(5);
		wdgObjects->tblObjects->setHorizontalHeaderLabels(QStringList() << "ID" << "IMG" << "CAM" << "TP" <<
				"CEN");
		wdgObjects->tblObjects->setColumnWidth(0, 75);
		wdgObjects->tblObjects->setColumnWidth(1, 80);
		wdgObjects->tblObjects->setColumnWidth(2, 40);
		wdgObjects->tblObjects->setColumnWidth(3, 50);
		wdgObjects->tblObjects->setColumnWidth(4, 50);
}

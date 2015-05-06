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
#include "QCategoryButton.h"

MainWindow::MainWindow( ConfigHandler *cfgArg, DatabaseHandler *dbArg, QWidget *parent) :
	QMainWindow(0), ui(new Ui::MainWindow), cfg(cfgArg), db(dbArg)
{
	Q_UNUSED(parent);
	ui->setupUi(this);

	// create expandable widget type
	ui->wdgFrameTree->setRootIsDecorated(false);
	ui->wdgFrameTree->setIndentation(0);

	//create widget
		{
		twgSession = new QTreeWidgetItem();
		ui->wdgFrameTree->addTopLevelItem(twgSession);
		ui->wdgFrameTree->setItemWidget(twgSession, 0,
		new QCategoryButton("Projektauswahl", ui->wdgFrameTree, twgSession));

		QTreeWidgetItem * wdgContainer = new QTreeWidgetItem();
		wdgContainer->setDisabled(true);
		twgSession->addChild(wdgContainer);
		QWidget *widget = new QWidget;
		wdgSession = new Ui::wdgSessions;
		wdgSession->setupUi(widget);
		ui->wdgFrameTree->setItemWidget(wdgContainer,0,widget);
		twgSession->setExpanded(true);
	}

	//create widget
	{
		twgObjects = new QTreeWidgetItem();
		ui->wdgFrameTree->addTopLevelItem(twgObjects);
		ui->wdgFrameTree->setItemWidget(twgObjects, 0,
		new QCategoryButton("Objektauswahl", ui->wdgFrameTree, twgObjects));

		QTreeWidgetItem * wdgContainer = new QTreeWidgetItem();
		wdgContainer->setDisabled(true);
		twgObjects->addChild(wdgContainer);
		QWidget *widget = new QWidget;
		wdgObjects = new Ui::wdgObjects;
		wdgObjects->setupUi(widget);
		ui->wdgFrameTree->setItemWidget(wdgContainer,0,widget);
		twgObjects->setExpanded(false);
	}

	//create widget
	{
		twgCensus = new QTreeWidgetItem();
		ui->wdgFrameTree->addTopLevelItem(twgCensus);
		ui->wdgFrameTree->setItemWidget(twgCensus, 0,
		new QCategoryButton("Bestimmungstabellen", ui->wdgFrameTree, twgCensus));
		QTreeWidgetItem * wdgContainer = new QTreeWidgetItem();
		wdgContainer->setDisabled(true);
		twgCensus->addChild(wdgContainer);
		QWidget *widget = new QWidget;
		wdgCensus = new Ui::wdgCensus;
		wdgCensus->setupUi(widget);
		ui->wdgFrameTree->setItemWidget(wdgContainer,0,widget);
		twgCensus->setExpanded(false);
	}

	//create widget
	{
		twgGraphics = new QTreeWidgetItem();
		ui->wdgFrameTree->addTopLevelItem(twgGraphics);
		ui->wdgFrameTree->setItemWidget(twgGraphics, 0,
		new QCategoryButton("Bildverarbeitung", ui->wdgFrameTree, twgGraphics));
		QTreeWidgetItem * wdgContainer = new QTreeWidgetItem();
		wdgContainer->setDisabled(true);
		twgGraphics->addChild(wdgContainer);
		QWidget *widget = new QWidget;
		wdgGraphics = new Ui::wdgGraphics;
		wdgGraphics->setupUi(widget);
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

    connect( objSelector, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(objectUpdateSelection()));
    connect(wdgSession->btnSession, SIGNAL(released()), this, SLOT(populateObjectTable()));
    connect(btnMapModeImg , SIGNAL(released()), this, SLOT(handleMapToolButton()));
    connect(btnMapModeGeo , SIGNAL(released()), this, SLOT(handleMapToolButton()));
    connect(btnZoomOneOne, SIGNAL(released()), this, SLOT(handleOneToOneZoom()));
    connect(dirDial, SIGNAL(sliderReleased()), this, SLOT(handleDirDial()));
    connect(wdgCensus->btnUserSelect, SIGNAL(released()), this, SLOT(handleUsrSelect()));
    connect(wdgGraphics->sldBrightness, SIGNAL(sliderReleased()), this, SLOT(handleBrightnessSlider()));
    connect(wdgGraphics->sldContrast, SIGNAL(sliderReleased()), this, SLOT(handleContrastSlider()));
    connect(wdgGraphics->btnBrightnessReset, SIGNAL(clicked()), this, SLOT(handleBrightnessReset()));
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
    wdgCensus->btnBirdSizeLength->setEnabled(false);
    wdgCensus->btnBirdSizeSpan->setEnabled(false);
    wdgCensus->btnMammalSizeLength->setEnabled(false);

}

MainWindow::~MainWindow()
{
    delete ui;
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
	twgSession->setExpanded(false);
	twgObjects->setExpanded(true);
}

void MainWindow::objectUpdateSelection() {
	wdgGraphics->sldBrightness->setValue(0);
	wdgGraphics->sldContrast->setValue(0);
	imgcvs->endMeasurement();
	msm_running = false;
	// TODO: Cleanup.
	// TODO: Fix: Crash on empty line
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

	if (!imgcvs->loadObject(curObj, db->getObjectPosition(objId))) {
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
	twgObjects->setExpanded(false);
	twgCensus->setExpanded(true);
	ui->wdgFrameTree->scrollToItem(twgCensus);
}


/*
 * Handle Save Buttons
 * TODO: Put all in one save routine
 * TODO: PUT ALL IN ONE SAVE ROUTINE!
 */

void MainWindow::handleSaveButton() {
	qDebug() << "Trying to save as user: " << curObj->usr;
//	QString objId = wdgObjects->tblObject->item(currentRow, 0)->text();

	curObj->type = wdgCensus->wdgTabTypes->currentWidget()->property("dbvalue").toString();

	if(curObj->type == "BIRD") {
		if ((wdgCensus->btngBirdBeh->checkedButton()->property("dbvalue").toString() == "FLY") && (dialChecked == false)) {
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
		curObj->name = wdgCensus->cmbAnthroName->itemData(wdgCensus->cmbAnthroName->currentIndex()).toString();
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
	msm_running = false;

	// handle those different -- courtesy of the stuk4 table widget
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
			if (cobj->length > 0 ) wdgCensus->lblBirdSizeSpan->setText(QString::number(cobj->span));
			wdgCensus->lblStuk4BehBird->setText("Verhalten: " + cobj->stuk4_beh.join(", "));
			wdgCensus->lblStuk4AssBird->setText("Assoziationen: " + cobj->stuk4_ass.join(", "));

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
			if (cobj->length > 0 )wdgCensus->lblMammalSizeLength->setText(QString::number(cobj->length));
			wdgCensus->lblStuk4BehMammal->setText("Verhalten: " + cobj->stuk4_beh.join(", "));
			wdgCensus->lblStuk4AssMammal->setText("Assoziationen: " + cobj->stuk4_ass.join(", "));

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
//	qDebug() << "Changing Brightness";
//	double scale = wdgGraphics->sldBrightness->value()/100.0/2;
//	int maxval = 65535 - int(scale * 65535.);
//	int minval = 0 + int(scale * 65535.);
//	qDebug() << "Scale: " << scale << "Min. value: " << minval << "Max. value: " << maxval;
//	QgsRasterLayer * imgLayer = imgcvs->getImageLayer();
//	QgsRasterDataProvider * provider = imgLayer->dataProvider();
//    QgsContrastEnhancement* qgsContrastEnhRed = new QgsContrastEnhancement(QGis::UInt16);
//    qgsContrastEnhRed->setMinimumValue(minval);
//    qgsContrastEnhRed->setMaximumValue(maxval);
//    qgsContrastEnhRed->setContrastEnhancementAlgorithm ( QgsContrastEnhancement::StretchToMinimumMaximum);
//
//    QgsContrastEnhancement* qgsContrastEnhGreen = new QgsContrastEnhancement(QGis::UInt16);
//    qgsContrastEnhGreen->setMinimumValue(minval);
//    qgsContrastEnhGreen->setMaximumValue(maxval);
//    qgsContrastEnhGreen->setContrastEnhancementAlgorithm ( QgsContrastEnhancement::StretchToMinimumMaximum);
//
//    QgsContrastEnhancement* qgsContrastEnhBlue = new QgsContrastEnhancement(QGis::UInt16);
//    qgsContrastEnhBlue->setMinimumValue(minval);
//    qgsContrastEnhBlue->setMaximumValue(maxval);
//    qgsContrastEnhBlue->setContrastEnhancementAlgorithm ( QgsContrastEnhancement::StretchToMinimumMaximum);
//
//    QgsMultiBandColorRenderer* renderer = new QgsMultiBandColorRenderer( provider , 1, 2, 3,
//    		qgsContrastEnhRed, qgsContrastEnhGreen, qgsContrastEnhBlue);
//
//    imgLayer->setRenderer(renderer);
//    imgcvs->refresh();
//
//    qDebug() << "Done.";
	imgcvs->setRasterBrightness(wdgGraphics->sldBrightness->value());
}

void MainWindow::handleContrastSlider() {
//	qDebug() << "Changing Contrast";
//	double scale = double()/100.0/2;
//	int minval = 0 + 0.5*pow(double(wdgGraphics->sldContrastMin->value())/100.0,2)*65535;
//	int maxval = 65535 - 0.5*pow(double(wdgGraphics->sldContrastMax->value())/100.0,2)*65535;
//	qDebug() << "Scale: " << scale << "Min. value: " << minval << "Max. value: " << maxval;
//	QgsRasterLayer * imgLayer = imgcvs->getImageLayer();
//	QgsRasterDataProvider * provider = imgLayer->dataProvider();
//    QgsContrastEnhancement* qgsContrastEnhRed = new QgsContrastEnhancement(QGis::UInt16);
//    qgsContrastEnhRed->setMinimumValue(minval);
//    qgsContrastEnhRed->setMaximumValue(maxval);
//    qgsContrastEnhRed->setContrastEnhancementAlgorithm ( QgsContrastEnhancement::StretchToMinimumMaximum);
//
//    QgsContrastEnhancement* qgsContrastEnhGreen = new QgsContrastEnhancement(QGis::UInt16);
//    qgsContrastEnhGreen->setMinimumValue(minval);
//    qgsContrastEnhGreen->setMaximumValue(maxval);
//    qgsContrastEnhGreen->setContrastEnhancementAlgorithm ( QgsContrastEnhancement::StretchToMinimumMaximum);
//
//    QgsContrastEnhancement* qgsContrastEnhBlue = new QgsContrastEnhancement(QGis::UInt16);
//    qgsContrastEnhBlue->setMinimumValue(minval);
//    qgsContrastEnhBlue->setMaximumValue(maxval);
//    qgsContrastEnhBlue->setContrastEnhancementAlgorithm ( QgsContrastEnhancement::StretchToMinimumMaximum);
//
//    QgsMultiBandColorRenderer* renderer = new QgsMultiBandColorRenderer( provider , 1, 2, 3,
//    		qgsContrastEnhRed, qgsContrastEnhGreen, qgsContrastEnhBlue);
//
//    imgLayer->setRenderer(renderer);
//    imgcvs->refresh();
//
//    qDebug() << "Done.";
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
	wdgObjects->tblFilters->setHorizontalHeaderLabels(QStringList() << "ID" << "IMG" << "CAM" << "Typ" <<
				"Bestimmung");
	wdgObjects->tblFilters->horizontalHeader()->setHighlightSections(false);
	wdgObjects->tblFilters->setSelectionMode(QAbstractItemView::NoSelection);

	wdgObjects->tblFilters->setColumnWidth(0, 75);
	wdgObjects->tblFilters->setColumnWidth(1, 80);
	wdgObjects->tblFilters->setColumnWidth(2, 40);
	wdgObjects->tblFilters->setColumnWidth(3, 50);
	wdgObjects->tblFilters->setColumnWidth(4, 80);

	wdgObjects->tblFilters->horizontalHeader()->setStretchLastSection(true);

//	wdgObjects->tblFilters->horizontalHeaderItem(0)->setData()

	wdgObjects->tblFilters->setRowCount(1);

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

	wdgObjects->cmbFilterCensor->addItem(trUtf8(""), QVariant(""));
	wdgObjects->cmbFilterCensor->addItem(trUtf8("Unbearbeitet"),
			QVariant(" AND tp IS NULL AND (mc<2 OR mc IS NULL)"));
	wdgObjects->cmbFilterCensor->addItem(trUtf8("Bearbeitet"),QVariant(" AND tp IS NOT NULL"));
	wdgObjects->cmbFilterCensor->addItem(trUtf8("Endbestimmt"),QVariant(" AND mc>1"));
	wdgObjects->cmbFilterCensor->addItem(trUtf8("Unstimmigkeiten"),QVariant(" AND mc=1 AND cnt>1"));

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
	connect(wdgObjects->cmbFilterCensor, SIGNAL(currentIndexChanged(int)), this, SLOT(handleCensorFilter(int)));
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
	filterMap["Censor"] = wdgObjects->cmbFilterCensor->itemData(index).toString();
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
	if (!msm_running) {
		wdgCensus->btnBirdSizeLength->setEnabled(false);
		wdgCensus->btnMammalSizeLength->setEnabled(false);
		imgcvs->beginMeasurement();
		wdgCensus->lblBirdSizeSpan->setText(trUtf8("Messung läuft."));
		curObj->span = -1.0;
		msm_running = true;
	} else {
		curObj->span = imgcvs->endMeasurement();
		wdgCensus->lblBirdSizeSpan->setText(QString::number(curObj->span, 'f', 2));
		msm_running = false;
		wdgCensus->btnBirdSizeLength->setEnabled(true);
		wdgCensus->btnMammalSizeLength->setEnabled(true);
	}

};

void MainWindow::handleBirdLengthMeasurement() {
	if (!msm_running) {
		wdgCensus->btnBirdSizeSpan->setEnabled(false);
		wdgCensus->btnMammalSizeLength->setEnabled(false);
		imgcvs->beginMeasurement();
		wdgCensus->lblBirdSizeLength->setText(trUtf8("Messung läuft."));
		curObj->length = -1.0;
		msm_running = true;
	} else {
		curObj->length = imgcvs->endMeasurement();
		wdgCensus->lblBirdSizeLength->setText(QString::number(curObj->length, 'f', 2));
		msm_running = false;
		wdgCensus->btnBirdSizeSpan->setEnabled(true);
		wdgCensus->btnMammalSizeLength->setEnabled(true);
	}
};

void MainWindow::handleMammalLengthMeasurement() {
	if (!msm_running) {
		wdgCensus->btnBirdSizeSpan->setEnabled(false);
		wdgCensus->btnBirdSizeLength->setEnabled(false);
		imgcvs->beginMeasurement();
		wdgCensus->lblMammalSizeLength->setText(trUtf8("Messung läuft."));
		curObj->length=-1.0;
		msm_running = true;
	} else {
		curObj->length = imgcvs->endMeasurement();
		wdgCensus->lblMammalSizeLength->setText(QString::number(curObj->length, 'f', 2));
		msm_running = false;
		wdgCensus->btnBirdSizeSpan->setEnabled(false);
		wdgCensus->btnBirdSizeLength->setEnabled(false);
	}
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


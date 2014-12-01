/*
 * SessionDialog.cpp
 *
 *  Created on: Dec 1, 2014
 *      Author: awg
 */

#include "SessionDialog.h"
#include "ui_sessiondialog.h"

SessionDialog::SessionDialog(ConfigHandler * cfg) : cfg(cfg), dlg(new Ui::dlgModeSelect) {
	// TODO Auto-generated constructor stub
	dlg->setupUi(this);
	dlg->cmbSource->addItem("Lokale Daten", "local");
	dlg->cmbSource->addItem("Platform-Z", "pfz");

	QStringList dbList = cfg->getDbList();
	qDebug() << dbList.size() << " databases found.";
	dlg->cmbDatabase->addItems(dbList);

	connect(dlg->btnYes, SIGNAL(released()), this, SLOT(handleYesButton()));
	connect(dlg->btnNo, SIGNAL(released()), this, SLOT(handleNoButton()));

}

SessionDialog::~SessionDialog() {
	delete dlg;
	// TODO Auto-generated destructor stub
}

void SessionDialog::handleYesButton() {
	qDebug() << "Starting application.";
	qDebug() << "Using Database: " << dlg->cmbDatabase->currentText();
	cfg->parseCfgFile(dlg->cmbDatabase->currentText());
	qDebug() << "Using data source: " << dlg->cmbSource->currentText();
	cfg->session_type = dlg->cmbSource->itemData(dlg->cmbSource->currentIndex()).toString();
	this->close();
}

void SessionDialog::handleNoButton() {
	qDebug() << "Quit by user.";
	exit(EXIT_SUCCESS);
}

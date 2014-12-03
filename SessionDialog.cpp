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
//    along with this program.  If not, see <http://www.gnu.org/licenses/>

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

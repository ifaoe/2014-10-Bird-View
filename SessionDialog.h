/*
 * SessionDialog.h
 *
 *  Created on: Dec 1, 2014
 *      Author: awg
 */

#ifndef SESSIONDIALOG_H_
#define SESSIONDIALOG_H_

#include <QDialog>
#include "ConfigHandler.h"

namespace Ui {
	class dlgModeSelect;
}

class SessionDialog : public QDialog{
public:
	SessionDialog(ConfigHandler * cfg);
	virtual ~SessionDialog();
private slots:
	void handleYesButton();
	void handleNoButton();
private:
	Ui::dlgModeSelect *dlg;
	ConfigHandler * cfg;
	QMap<QString, QString> dbMap;
};

#endif /* SESSIONDIALOG_H_ */

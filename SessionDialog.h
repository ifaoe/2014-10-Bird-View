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
	Q_OBJECT;
public:
	SessionDialog(ConfigHandler * cfg);
	virtual ~SessionDialog();
private slots:
	void handleYesButton();
	void handleNoButton();
private:
	ConfigHandler * cfg = 0;
	Ui::dlgModeSelect *dlg = 0;
};

#endif /* SESSIONDIALOG_H_ */

/*
 * FilterDialog.h
 *
 *  Created on: Feb 18, 2015
 *      Author: awg
 */

#ifndef FILTERDIALOG_H_
#define FILTERDIALOG_H_

#include <QDialog>
#include "ui_filterdialog.h"
#include "DatabaseHandler.h"

class FilterDialog: public QDialog {
	Q_OBJECT;
public:
	FilterDialog(DatabaseHandler * aDb, QString Session);
	virtual ~FilterDialog();
	QString getFilter();
public slots:
	void applyFilters();
	void abortFilters();
private:
	Ui::dlgFilterWindow * dlg = 0;
	DatabaseHandler * db = 0;
	QString filter = "";
};

#endif /* FILTERDIALOG_H_ */

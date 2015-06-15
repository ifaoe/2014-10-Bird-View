/*
 * GroupSelection.h
 *
 *  Created on: May 11, 2015
 *      Author: awg
 */

#ifndef GROUPSELECTION_H_
#define GROUPSELECTION_H_

#include <qdialog.h>
#include "DatabaseHandler.h"
#include "ui_dialog_groupid.h"

class GroupSelection: public QDialog {
	Q_OBJECT;
public:
	GroupSelection(DatabaseHandler * db, QWidget * parent = 0);
	virtual ~GroupSelection();
	void loadObject(census * cobj);
private slots:
	void handleSaveButton();
	void handleDiscardButton();
private:
	DatabaseHandler * db;
	census * cobj;
	Ui::dlgGroupSelection * dlg;
	QModelIndex getObjectIndex(QTableView * tbl, QString id);
	QWidget * parent;
protected:
	void resizeEvent(QResizeEvent * e);
};

#endif /* GROUPSELECTION_H_ */

/*
 * FamilySelection.h
 *
 *  Created on: May 11, 2015
 *      Author: awg
 */

#ifndef FamilySelection_H_
#define FamilySelection_H_

#include <qdialog.h>
#include "DatabaseHandler.h"
#include "ui_dialog_groupid.h"

class FamilySelection: public QDialog {
	Q_OBJECT;
public:
	FamilySelection(DatabaseHandler * db, census * cobj, QWidget * parent = 0);
	virtual ~FamilySelection();
private slots:
	void handleSaveButton();
	void handleDiscardButton();
private:
	DatabaseHandler * db;
	census * cobj;
	Ui::dlgGroupSelection * dlg;
	QModelIndex getObjectIndex(QString id);
	QWidget * parent;
protected:
	void resizeEvent(QResizeEvent * e);
};

#endif /* FamilySelection_H_ */

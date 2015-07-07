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

class MainWindow;

class GroupSelection: public QDialog {
    Q_OBJECT;
public:
    GroupSelection(DatabaseHandler * db, QWidget * parent, MainWindow * window);
    virtual ~GroupSelection();
    void loadObject(census * cobj);
private slots:
    void handleSaveButton();
    void handleDiscardButton();
private:
    DatabaseHandler * db;
    census * cobj = 0;
    Ui::dlgGroupSelection * dlg;
    QModelIndex getObjectIndex(QTableView * tbl, QString id);
    QWidget * parent;
    MainWindow * mainwindow;
protected:
    void resizeEvent(QResizeEvent * e);
};

#endif /* GROUPSELECTION_H_ */

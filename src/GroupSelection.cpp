/*
 * GroupSelection.cpp
 *
 *  Created on: May 11, 2015
 *      Author: awg
 */

#include "GroupSelection.h"
#include "census.hpp"
#include "mainwindow.h"

GroupSelection::GroupSelection(DatabaseHandler * aDb, QWidget * parent, MainWindow * window)
    : db(aDb), dlg(new Ui::dlgGroupSelection), parent(parent), mainwindow(window){
    // TODO Auto-generated constructor stub
    dlg->setupUi(this);

    setWindowFlags(windowFlags()|Qt::WindowStaysOnTopHint);
    setWindowModality(Qt::NonModal);
    setModal(false);

    connect(dlg->btbGroupSelection, SIGNAL(accepted()), this, SLOT(handleSaveButton()), Qt::UniqueConnection);
    connect(dlg->btbGroupSelection, SIGNAL(rejected()), this, SLOT(handleDiscardButton()), Qt::UniqueConnection);
}

GroupSelection::~GroupSelection() {
    // TODO Auto-generated destructor stub
}

void GroupSelection::resizeEvent(QResizeEvent * e) {
    QDialog::resizeEvent(e);
    dlg->tbvGroupSelection->resizeRowsToContents();
    dlg->tbvFamilySelection->resizeRowsToContents();
}

void GroupSelection::handleSaveButton() {
    cobj->group.clear();
    QModelIndexList index = dlg->tbvGroupSelection->selectionModel()->selectedRows();
    for (int i=0; i<index.size(); i++) {
        int row = index[i].row();
        cobj->group.push_back(
                dlg->tbvGroupSelection->model()->data(
                        dlg->tbvGroupSelection->model()->index(row,0)).toString());
    }
    qSort(cobj->group);

    cobj->family.clear();
    index.clear();
    index = dlg->tbvFamilySelection->selectionModel()->selectedRows();
    for (int i=0; i<index.size(); i++) {
        int row = index[i].row();
        cobj->family.push_back(
                dlg->tbvFamilySelection->model()->data(
                        dlg->tbvFamilySelection->model()->index(row,0)).toString());
    }
    qSort(cobj->family);
    mainwindow->handleGroupSelection();
}

void GroupSelection::handleDiscardButton() {
    qDebug() << "Reject!";
    setHidden(true);
}

QModelIndex GroupSelection::getObjectIndex(QTableView *tbl, QString obj) {
    for(int i=0; i<tbl->model()->rowCount(); i++) {
        QModelIndex ind = tbl->model()->index(i,0);
        if (tbl->model()->data(ind).toString() == obj)
            return ind;
    }
    return QModelIndex();
}

void GroupSelection::loadObject(census * obj) {
    cobj = obj;

    dlg->tbvGroupSelection->setModel(db->getCloseObjects(cobj));
    dlg->tbvGroupSelection->resizeColumnsToContents();
    dlg->tbvGroupSelection->horizontalHeader()->setStretchLastSection(true);
    dlg->tbvGroupSelection->verticalHeader()->resizeMode(QHeaderView::ResizeToContents);

    dlg->tbvFamilySelection->setModel(db->getCloseObjects(cobj));
    dlg->tbvFamilySelection->resizeColumnsToContents();
    dlg->tbvFamilySelection->horizontalHeader()->setStretchLastSection(true);
    dlg->tbvFamilySelection->verticalHeader()->resizeMode(QHeaderView::ResizeToContents);

    for (int i=0; i<cobj->group.size(); i++) {
        QModelIndex tmpind = getObjectIndex(dlg->tbvGroupSelection,cobj->group[i]);
        if (tmpind.isValid())
            dlg->tbvGroupSelection->selectionModel()->select(tmpind,
                    QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }

    for (int i=0; i<cobj->family.size(); i++) {
        QModelIndex tmpind = getObjectIndex(dlg->tbvFamilySelection, cobj->family[i]);
        if (tmpind.isValid())
            dlg->tbvFamilySelection->selectionModel()->select(tmpind,
                    QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }
}

bool GroupSelection::isHidden() {
	return hidden;
}

void GroupSelection::setHidden(bool hide) {
	hidden = hide;
	if (hide) {
		this->close();
	} else {
		this->show();
	}
}

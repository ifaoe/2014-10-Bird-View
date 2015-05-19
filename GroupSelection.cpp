/*
 * GroupSelection.cpp
 *
 *  Created on: May 11, 2015
 *      Author: awg
 */

#include "GroupSelection.h"
#include "census.hpp"

GroupSelection::GroupSelection(DatabaseHandler * aDb, census * cobj, QWidget * parent)
	: db(aDb), cobj(cobj), dlg(new Ui::dlgGroupSelection), parent(parent){
	// TODO Auto-generated constructor stub
	dlg->setupUi(this);
	dlg->tbvGroupSelection->setModel(db->getCloseObjects(cobj));
	dlg->tbvGroupSelection->resizeColumnsToContents();
	dlg->tbvGroupSelection->horizontalHeader()->setStretchLastSection(true);
	dlg->tbvGroupSelection->verticalHeader()->resizeMode(QHeaderView::ResizeToContents);

	for (int i=0; i<cobj->group.size(); i++) {
		QModelIndex tmpind = getObjectIndex(cobj->group[i]);
		if (tmpind.isValid())
			dlg->tbvGroupSelection->selectionModel()->select(tmpind,
					QItemSelectionModel::Select|QItemSelectionModel::Rows);
	}

	connect(dlg->btbGroupSelection, SIGNAL(accepted()), this, SLOT(handleSaveButton()));
	connect(dlg->btbGroupSelection, SIGNAL(rejected()), this, SLOT(handleDiscardButton()));

	setModal(true);
}

GroupSelection::~GroupSelection() {
	// TODO Auto-generated destructor stub
}

void GroupSelection::resizeEvent(QResizeEvent * e) {
	QDialog::resizeEvent(e);
	dlg->tbvGroupSelection->resizeRowsToContents();
	dlg->tbvGroupSelection->resizeRowsToContents();
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
	this->close();
}

void GroupSelection::handleDiscardButton() {
	qDebug() << "Reject!";
	this->close();
}

QModelIndex GroupSelection::getObjectIndex(QString obj) {
	for(int i=0; i<dlg->tbvGroupSelection->model()->rowCount(); i++) {
		QModelIndex ind = dlg->tbvGroupSelection->model()->index(i,0);
		if (dlg->tbvGroupSelection->model()->data(ind).toString() == obj)
			return ind;
	}
	return QModelIndex();
}

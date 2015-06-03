/*
 * GroupSelection.cpp
 *
 *  Created on: May 11, 2015
 *      Author: awg
 */

#include "FamilySelection.h"
#include "census.hpp"

FamilySelection::FamilySelection(DatabaseHandler * aDb, census * cobj, QWidget * parent)
	: db(aDb), cobj(cobj), dlg(new Ui::dlgGroupSelection), parent(parent){
	// TODO Auto-generated constructor stub
	dlg->setupUi(this);
	dlg->tbvGroupSelection->setModel(db->getCloseObjects(cobj));
	dlg->tbvGroupSelection->resizeColumnsToContents();
	dlg->tbvGroupSelection->horizontalHeader()->setStretchLastSection(true);
	dlg->tbvGroupSelection->verticalHeader()->resizeMode(QHeaderView::ResizeToContents);
	this->setWindowTitle("Auswahl - Familienverband");

	for (int i=0; i<cobj->family.size(); i++) {
		QModelIndex tmpind = getObjectIndex(cobj->family[i]);
		if (tmpind.isValid())
			dlg->tbvGroupSelection->selectionModel()->select(tmpind,
					QItemSelectionModel::Select|QItemSelectionModel::Rows);
	}

	connect(dlg->btbGroupSelection, SIGNAL(accepted()), this, SLOT(handleSaveButton()));
	connect(dlg->btbGroupSelection, SIGNAL(rejected()), this, SLOT(handleDiscardButton()));

	setModal(true);
}

FamilySelection::~FamilySelection() {
	// TODO Auto-generated destructor stub
}

void FamilySelection::resizeEvent(QResizeEvent * e) {
	QDialog::resizeEvent(e);
	dlg->tbvGroupSelection->resizeRowsToContents();
	dlg->tbvGroupSelection->resizeRowsToContents();
}

void FamilySelection::handleSaveButton() {
	cobj->family.clear();
	QModelIndexList index = dlg->tbvGroupSelection->selectionModel()->selectedRows();
	for (int i=0; i<index.size(); i++) {
		int row = index[i].row();
		cobj->family.push_back(
				dlg->tbvGroupSelection->model()->data(
						dlg->tbvGroupSelection->model()->index(row,0)).toString());
	}
	qSort(cobj->family);
	this->close();
}

void FamilySelection::handleDiscardButton() {
	qDebug() << "Reject!";
	this->close();
}

QModelIndex FamilySelection::getObjectIndex(QString obj) {
	for(int i=0; i<dlg->tbvGroupSelection->model()->rowCount(); i++) {
		QModelIndex ind = dlg->tbvGroupSelection->model()->index(i,0);
		if (dlg->tbvGroupSelection->model()->data(ind).toString() == obj)
			return ind;
	}
	return QModelIndex();
}

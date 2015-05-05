/*
 * Stuk4Dialog.cpp
 *
 *  Created on: May 4, 2015
 *      Author: awg
 */

#include "Stuk4Dialog.h"

Stuk4Dialog::Stuk4Dialog(DatabaseHandler * aDb, QStringList * s4beh, QStringList * s4ass)
	: db(aDb), stuk4_beh(s4beh), stuk4_ass(s4ass), dlg(new Ui::dlgStuk4Codes) {

	// TODO Auto-generated constructor stub
	dlg->setupUi(this);

	dlg->tbvStuk4Beh->setModel(db->getStuk4Behaviour());
	dlg->tbvStuk4Beh->resizeColumnsToContents();
	dlg->tbvStuk4Beh->horizontalHeader()->setStretchLastSection(true);
//	dlg->tbvStuk4Beh->resizeRowsToContents();
	dlg->tbvStuk4Beh->verticalHeader()->resizeMode(QHeaderView::ResizeToContents);

	dlg->tbvStuk4Ass->setModel(db->getStuk4Associations());
	dlg->tbvStuk4Ass->resizeColumnsToContents();
	dlg->tbvStuk4Ass->horizontalHeader()->setStretchLastSection(true);
//	dlg->tbvStuk4Ass->resizeRowsToContents();
	dlg->tbvStuk4Ass->verticalHeader()->resizeMode(QHeaderView::ResizeToContents);


	for(int i=0; i<s4beh->size(); i++) {
		QModelIndex tmpind = getBehaviourIndex(s4beh->at(i));
		if (tmpind.isValid())
			dlg->tbvStuk4Beh->selectionModel()->select(tmpind,
					QItemSelectionModel::Select|QItemSelectionModel::Rows);
	}

	for(int i=0; i<s4ass->size(); i++) {
		QModelIndex tmpind = getAssociationsIndex(s4ass->at(i));
		if (tmpind.isValid())
			dlg->tbvStuk4Ass->selectionModel()->select(tmpind,
					QItemSelectionModel::Select|QItemSelectionModel::Rows);
	}

	connect(dlg->btbStuk4Codes, SIGNAL(accepted()), this, SLOT(handleSaveButton()));
	connect(dlg->btbStuk4Codes, SIGNAL(rejected()), this, SLOT(handleDiscardButton()));
}

Stuk4Dialog::~Stuk4Dialog() {
	// TODO Auto-generated destructor stub
}

void Stuk4Dialog::handleSaveButton() {
	stuk4_beh->clear();
	QModelIndexList beh_index = dlg->tbvStuk4Beh->selectionModel()->selectedRows();
	for(int i=0; i<beh_index.size(); i++){
		int row = beh_index[i].row();
		stuk4_beh->push_back(
			dlg->tbvStuk4Beh->model()->data(dlg->tbvStuk4Beh->model()->index(row, 0)).toString()
			);
	}
	stuk4_ass->clear();
	QModelIndexList ass_index = dlg->tbvStuk4Ass->selectionModel()->selectedRows();
	for(int i=0; i<ass_index.size(); i++){
		int row = ass_index[i].row();
		stuk4_ass->push_back(
			dlg->tbvStuk4Ass->model()->data(dlg->tbvStuk4Ass->model()->index(row, 0)).toString());
	}
	this->close();
}

void Stuk4Dialog::handleDiscardButton() {
	qDebug() << "Reject!";
	this->close();
}

void Stuk4Dialog::resizeEvent(QResizeEvent * e) {
	QDialog::resizeEvent(e);
//	dlg->tbvStuk4Beh->resizeColumnsToContents();
	dlg->tbvStuk4Beh->resizeRowsToContents();
//	dlg->tbvStuk4Ass->resizeColumnsToContents();
	dlg->tbvStuk4Ass->resizeRowsToContents();
}

QModelIndex Stuk4Dialog::getBehaviourIndex(QString code){
	for(int i=0; i<dlg->tbvStuk4Beh->model()->rowCount(); i++) {
		QModelIndex ind = dlg->tbvStuk4Beh->model()->index(i,0);
		if (dlg->tbvStuk4Beh->model()->data(ind).toString() == code)
			return ind;
	}
	return QModelIndex();
}

QModelIndex Stuk4Dialog::getAssociationsIndex(QString code){
	for(int i=0; i<dlg->tbvStuk4Ass->model()->rowCount(); i++) {
		QModelIndex ind = dlg->tbvStuk4Ass->model()->index(i,0);
		if (dlg->tbvStuk4Ass->model()->data(ind).toString() == code)
			return ind;
	}
	return QModelIndex();
}

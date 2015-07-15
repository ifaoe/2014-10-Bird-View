/*
 * IdSelectionDialog.cpp
 *
 *  Created on: Jul 8, 2015
 *      Author: awg
 */

#include "IdSelectionDialog.h"

IdSelectionDialog::IdSelectionDialog(QLabel * label, QWidget * parent)
	: parent_(parent), info_label_(label), dlg_(new Ui::dlg_idselection) {
	// TODO Auto-generated constructor stub
	dlg_->setupUi(this);

	setWindowFlags(windowFlags()|Qt::WindowStaysOnTopHint);
	setWindowModality(Qt::NonModal);
	setModal(false);

	connect(dlg_->btb_idselection, SIGNAL(accepted()), this, SLOT(HandleSaveButton()), Qt::UniqueConnection);
	connect(dlg_->btb_idselection, SIGNAL(rejected()), this, SLOT(HandleDiscardButton()), Qt::UniqueConnection);
}

IdSelectionDialog::~IdSelectionDialog() {
	// TODO Auto-generated destructor stub
}

QModelIndex IdSelectionDialog::GetDataRow(QString obj) {
    for(int i=0; i<dlg_->tbv_idselection->model()->rowCount(); i++) {
        QModelIndex ind = dlg_->tbv_idselection->model()->index(i,0);
        if (dlg_->tbv_idselection->model()->data(ind).toString() == obj)
            return ind;
    }
    return QModelIndex();
}

void IdSelectionDialog::resizeEvent(QResizeEvent * e) {
    QDialog::resizeEvent(e);
    dlg_->tbv_idselection->resizeRowsToContents();
    dlg_->tbv_idselection->resizeRowsToContents();
}


void IdSelectionDialog::HandleSaveButton() {
    id_list_->clear();
    QModelIndexList index = dlg_->tbv_idselection->selectionModel()->selectedRows();
    for (int i=0; i<index.size(); i++) {
        int row = index[i].row();
        id_list_->push_back(
                dlg_->tbv_idselection->model()->data(
                        dlg_->tbv_idselection->model()->index(row,0)).toString());
    }
    qSort(*id_list_);
    UpdateInfoLabel();
	this->close();
	hidden_ = true;
}

void IdSelectionDialog::HandleDiscardButton() {
	this->close();
	hidden_ = true;
}

void IdSelectionDialog::ToggleHidden() {
	hidden_ = !hidden_;
	if (hidden_)
		this->close();
	else
		this->show();
}

void IdSelectionDialog::set_data_model(QSqlQueryModel * model) {
	data_model_ = model;
	dlg_->tbv_idselection->setModel(model);
	dlg_->tbv_idselection->resizeColumnsToContents();
	dlg_->tbv_idselection->horizontalHeader()->setStretchLastSection(true);
	dlg_->tbv_idselection->verticalHeader()->resizeMode(QHeaderView::ResizeToContents);
}

void IdSelectionDialog::UpdateSelection() {
    for (int i=0; i<id_list_->size(); i++) {
        QModelIndex tmpind = GetDataRow(id_list_->at(i));
        if (tmpind.isValid())
            dlg_->tbv_idselection->selectionModel()->select(tmpind,
                    QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }
}

void IdSelectionDialog::UpdateInfoLabel() {
	info_label_->setText(id_list_->join(", "));
}

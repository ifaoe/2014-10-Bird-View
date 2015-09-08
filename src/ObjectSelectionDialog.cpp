/*
 * IdSelectionDialog.cpp
 *
 *  Created on: Jul 8, 2015
 *      Author: awg
 */

#include "ObjectSelectionDialog.h"

ObjectSelectionDialog::ObjectSelectionDialog(QLabel * label, QWidget * parent)
	: info_label_(label), parent_(parent), dialog(new Ui::object_selection) {
	// TODO Auto-generated constructor stub
	dialog->setupUi(this);

	setWindowFlags(windowFlags()|Qt::WindowStaysOnTopHint);
	setWindowModality(Qt::NonModal);
	setModal(false);

	connect(dialog->buttonbox, SIGNAL(accepted()), this, SLOT(HandleSaveButton()), Qt::UniqueConnection);
	connect(dialog->buttonbox, SIGNAL(rejected()), this, SLOT(HandleDiscardButton()), Qt::UniqueConnection);
}

ObjectSelectionDialog::~ObjectSelectionDialog() {
	// TODO Auto-generated destructor stub
}

QModelIndex ObjectSelectionDialog::GetDataRow(QString obj) {
    for(int i=0; i<dialog->selection_view->model()->rowCount(); i++) {
        QModelIndex ind = dialog->selection_view->model()->index(i,0);
        if (dialog->selection_view->model()->data(ind).toString() == obj)
            return ind;
    }
    return QModelIndex();
}

void ObjectSelectionDialog::resizeEvent(QResizeEvent * e) {
    QDialog::resizeEvent(e);
    dialog->selection_view->resizeRowsToContents();
}


void ObjectSelectionDialog::HandleSaveButton() {
    id_list_->clear();
    QModelIndexList index = dialog->selection_view->selectionModel()->selectedRows();
    for (int i=0; i<index.size(); i++) {
        int row = index[i].row();
        id_list_->push_back(
                dialog->selection_view->model()->data(
                        dialog->selection_view->model()->index(row,0)).toString());
    }
    qSort(*id_list_);
    UpdateInfoLabel();
	this->close();
	hidden_ = true;
}

void ObjectSelectionDialog::HandleDiscardButton() {
	this->close();
	hidden_ = true;
}

void ObjectSelectionDialog::ToggleHidden() {
	hidden_ = !hidden_;
	if (hidden_)
		this->close();
	else
		this->show();
}

void ObjectSelectionDialog::setDataModel(QSqlQueryModel * model) {
	data_model = model;
	dialog->selection_view->setModel(model);
	dialog->selection_view->resizeColumnsToContents();
	dialog->selection_view->horizontalHeader()->setStretchLastSection(true);
	dialog->selection_view->verticalHeader()->resizeMode(QHeaderView::ResizeToContents);
}

void ObjectSelectionDialog::UpdateSelection() {
	dialog->selection_view->clearSelection();
    for (int i=0; i<id_list_->size(); i++) {
        QModelIndex tmpind = GetDataRow(id_list_->at(i));
        if (tmpind.isValid())
            dialog->selection_view->selectionModel()->select(tmpind,
                    QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }
}

void ObjectSelectionDialog::UpdateInfoLabel() {
	info_label_->setText(id_list_->join(", "));
}

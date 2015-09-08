/*
 * IdSelectionDialog.h
 *
 *  Created on: Jul 8, 2015
 *      Author: awg
 */

#ifndef IDSELECTIONDIALOG_H_
#define IDSELECTIONDIALOG_H_

#include <QDialog>
#include <QLabel>
#include <QSqlQueryModel>
#include "ui_dialog_object_selection.h"

class ObjectSelectionDialog: public QDialog {
	Q_OBJECT
public:
	ObjectSelectionDialog(QLabel * label, QWidget * parent = 0);
	virtual ~ObjectSelectionDialog();
	void setDataModel(QSqlQueryModel * model);
	QAbstractItemModel * dataModel() { return dialog->selection_view->model();}
	void set_id_column(int column) { id_column = column; }
	int idColumn() {return id_column;}
	void set_id_list(QStringList * id_list) {id_list_ = id_list; UpdateSelection(); UpdateInfoLabel();}
	void set_info_label(QLabel * label) {info_label_ = label;}
	void UpdateSelection();
	void ToggleHidden();
	void UpdateInfoLabel();
private:
    QLabel * info_label_;
	QStringList * id_list_;
	QWidget * parent_;
	Ui::object_selection * dialog;
	QSqlQueryModel * data_model;
	int id_column = 0;
	bool hidden_ = true;
	QModelIndex GetDataRow(QString id);
private slots:
	void HandleSaveButton();
	void HandleDiscardButton();
protected:
	void resizeEvent(QResizeEvent * e);
};

#endif /* IDSELECTIONDIALOG_H_ */

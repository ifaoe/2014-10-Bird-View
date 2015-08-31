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
#include "ui_dialog_idselection.h"

class IdSelectionDialog: public QDialog {
	Q_OBJECT
public:
	IdSelectionDialog(QLabel * label, QWidget * parent = 0);
	virtual ~IdSelectionDialog();
	void setDataModel(QSqlQueryModel * model);
	QAbstractItemModel * dataModel() { return dlg_->tbv_idselection->model();}
	void set_id_column(int column) { id_column_ = column; }
	int id_column() {return id_column_;}
	void set_id_list(QStringList * id_list) {id_list_ = id_list; UpdateSelection(); UpdateInfoLabel();}
	void set_info_label(QLabel * label) {info_label_ = label;}
	void UpdateSelection();
	void ToggleHidden();
	void UpdateInfoLabel();
private:
    QLabel * info_label_;
	QStringList * id_list_;
	QWidget * parent_;
	Ui::dlg_idselection * dlg_;
	QSqlQueryModel * data_model_;
	int id_column_ = 0;
	bool hidden_ = true;
	QModelIndex GetDataRow(QString id);
private slots:
	void HandleSaveButton();
	void HandleDiscardButton();
protected:
	void resizeEvent(QResizeEvent * e);
};

#endif /* IDSELECTIONDIALOG_H_ */

/*
 * FilterDialog.cpp
 *
 *  Created on: Feb 18, 2015
 *      Author: awg
 */

#include "FilterDialog.h"

FilterDialog::FilterDialog(DatabaseHandler * aDb, QString session) : dlg(new Ui::dlgFilterWindow), db(aDb){
	// TODO Auto-generated constructor stub
	dlg->setupUi(this);
	dlg->cmbCensusFilter->addItem(trUtf8(""), QVariant(""));
	dlg->cmbCensusFilter->addItem(trUtf8("Unbestimmt"),QVariant(" AND (mc IS NULL OR mc=0)"));
	dlg->cmbCensusFilter->addItem(trUtf8("Vorbestimmt"),QVariant(" AND mc=1"));
	dlg->cmbCensusFilter->addItem(trUtf8("Endbestimmt"),QVariant(" AND mc>1"));
	dlg->cmbCamFilter->addItem(trUtf8(""), QVariant(""));
	dlg->cmbCamFilter->addItem(trUtf8("1"), QVariant(" AND cam='1'"));
	dlg->cmbCamFilter->addItem(trUtf8("2"), QVariant(" AND cam='2'"));
	dlg->cmbTypeFilter->addItem(trUtf8(""), QVariant(""));
	dlg->cmbTypeFilter->addItems(db->getTypeList(session));

	connect(dlg->btbFilter, SIGNAL(accepted()), this, SLOT(applyFilters()));
	connect(dlg->btbFilter, SIGNAL(rejected()), this, SLOT(abortFilters()));
}

FilterDialog::~FilterDialog() {
	// TODO Auto-generated destructor stub
}

void FilterDialog::applyFilters() {
	filter.append(" WHERE TRUE ");
	filter.append(dlg->cmbCensusFilter->itemData(dlg->cmbCensusFilter->currentIndex()).toString());
	filter.append(dlg->cmbCamFilter->itemData(dlg->cmbCamFilter->currentIndex()).toString());
	if (dlg->cmbTypeFilter->currentIndex() > 0)
		filter.append(" AND tp='" + dlg->cmbTypeFilter->currentText() + "'");
	filter.append(" AND cast(rcns_id as text) LIKE '%" + dlg->pteIdFilter->toPlainText() + "%'");
	filter.append(" AND img LIKE '%" + dlg->pteImgFilter->toPlainText() + "%'");
}

void FilterDialog::abortFilters() {
	this->close();
}

QString FilterDialog::getFilter() {
	return filter;
}

/*
 * MeasurementDialog.cpp
 *
 *  Created on: Jun 9, 2015
 *      Author: awg
 */

#include "MeasurementDialog.h"
#include <QPushButton>

MeasurementDialog::MeasurementDialog(ImgCanvas * cvs)
: cvs(cvs), dlg(new Ui::dlgMeasurement) {
	dlg->setupUi(this);
	connect(dlg->btbMeasurement,SIGNAL(accepted()), this, SLOT(handleAccept()));
	connect(dlg->btbMeasurement,SIGNAL(rejected()), this, SLOT(handleReject()));
	setWindowFlags(windowFlags()|Qt::WindowStaysOnTopHint);
	setWindowModality(Qt::NonModal);
	setModal(false);
}

MeasurementDialog::~MeasurementDialog() {
	// TODO Auto-generated destructor stub
}

void MeasurementDialog::updateStatusMessage(QString text) {
	dlg->lblMeasurementStatus->setText(text);
}

void MeasurementDialog::updateInfoMessage(QString text) {
	dlg->lblMeasurementInfo->setText(text);
}

void MeasurementDialog::handleAccept() {
	running=false;
	close();
	if (value == 0 || label == 0) {
		cvs->endMeasurement();
		return;
	}
	*value = cvs->endMeasurement();
	if (*value > 0) {
		label->setText(QString::number(*value) + " m");
	} else {
		*value=-1.0;
		label->clear();
	}
}

void MeasurementDialog::handleReject() {
	running=false;
	cvs->endMeasurement();
	close();
	if (value != 0 && label != 0) {
		label->clear();
		*value = -1.0;
	}
}

void MeasurementDialog::startMeasurement(double * val, QLabel * lbl) {
	if (running) return;
	running = true;
	updateStatusMessage(QString::fromUtf8("Messung läuft."));
	updateInfoMessage(QString::fromUtf8("Bitte Messpunkte setzen."));
	value = val;
	label = lbl;
	cvs->beginMeasurement(this);

	if (value == 0 || label == 0)
		dlg->btbMeasurement->button(QDialogButtonBox::Save)->hide();
	else
		dlg->btbMeasurement->button(QDialogButtonBox::Save)->show();
	show();
}

void MeasurementDialog::closeEvent(QCloseEvent * e) {
//	running=false;
//	cvs->endMeasurement();
//	close();
//	if (value != 0 && label != 0) {
//		label->clear();
//		*value = -1.0;
//	}
	e->ignore();
}

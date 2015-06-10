/*
 * MeasurementDialog.cpp
 *
 *  Created on: Jun 9, 2015
 *      Author: awg
 */

#include "MeasurementDialog.h"

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
	if (value == 0 || label == 0) this->close();
	*value = cvs->endMeasurement();
	if (*value > 0) {
		label->setText(QString::number(*value) + " m");
	} else {
		*value=-1.0;
		label->clear();
	}

	this->close();
}

void MeasurementDialog::handleReject() {
	if (value == 0 || label == 0) this->close();
	cvs->endMeasurement();
	label->clear();
	*value = -1.0;
	this->close();
}

void MeasurementDialog::startMeasurement(double * val, QLabel * lbl) {
	updateStatusMessage(QString::fromUtf8("Messung läuft."));
	updateInfoMessage(QString::fromUtf8("Bitte Messpunkte setzen."));
	value = val;
	label = lbl;
	cvs->beginMeasurement(this);
}

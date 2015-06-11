/*
 * MeasurementDialog.h
 *
 *  Created on: Jun 9, 2015
 *      Author: awg
 */

#ifndef MEASUREMENTDIALOG_H_
#define MEASUREMENTDIALOG_H_
#include "ui_dialog_measurement.h"
#include "ImgCanvas.h"
#include "census.hpp"

class ImgCanvas;


class MeasurementDialog : public QDialog {
	Q_OBJECT;
public:
	MeasurementDialog(ImgCanvas * cvs);
	virtual ~MeasurementDialog();
	void updateStatusMessage(QString text);
	void updateInfoMessage(QString text);
	void startMeasurement(double * value, QLabel * label);
private:
	void closeEvent(QCloseEvent *e);
	ImgCanvas * cvs = 0;
	double * value = 0;
	QLabel * label = 0;
	bool running = false;
	Ui::dlgMeasurement * dlg = 0;
private slots:
	void handleAccept();
	void handleReject();
};

#endif /* MEASUREMENTDIALOG_H_ */

//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef IMGCANVAS_H_
#define IMGCANVAS_H_

#include <QDebug>
#include <QObject>
#include <QWidget>
#include <QNetworkReply>
#include <qgis.h>
#include <qgsmapcanvas.h>
#include <qgsrasterlayer.h>
#include <qgsvectorlayer.h>
#include <qgsrasterdataprovider.h>
#include <qgsmaplayerregistry.h>
#include <qgsmaptoolemitpoint.h>
#include <qgsmaptoolpan.h>
#include <qgsgeometry.h>
#include "QgsLayerStack.h"
#include "ConfigHandler.h"
#include "DatabaseHandler.h"
#include "ui_mainwindow.h"
#include "census.hpp"
#include "MeasurementDialog.h"
#include <qgslabel.h>
#include <qgslabelattributes.h>
#include <qgsfield.h>

class MeasurementDialog;

class ImgCanvas: public QgsMapCanvas {
	Q_OBJECT
public:
	ImgCanvas(QWidget *parent, Ui::MainWindow *mUi, ConfigHandler * cfg, DatabaseHandler *db);
	virtual ~ImgCanvas();
	bool loadObject(census * obj);
	void centerOnPixelPosition(int x, int y, double scale);
	void centerOnWorldPosition(double ux, double uy, double scale);
	QgsPoint calcWorldPosition(int x, int y);
	void calcPixelPosition(QgsPoint pos);
	QgsRasterLayer * getImageLayer();
	void setRasterBrightness(int value);
	void setRasterContrast(int value);
	void beginMeasurement(MeasurementDialog * msmWindow);
	double getCurrentMeasurement();
	double endMeasurement();
private slots:
	void handleCanvasClicked(const QgsPoint & point);
	void handleHideObjectMarkers();
private:
	QgsLayerStack * layerStack = 0;
	Ui::MainWindow *ui = 0;
	ConfigHandler *cfg = 0;
	DatabaseHandler *db = 0;
	QgsMapLayerRegistry *lyrRegistry = 0;
	QgsRasterLayer * imgLayer = 0;
	QgsVectorLayer * msmLayer = 0;
	QgsRasterDataProvider* imgProvider = 0;

	QgsMapToolEmitPoint *qgsEmitPointTool = 0;
	QgsMapToolPan * qgsMapPanTool = 0;
//	std::vector<QgsPoint> msmList;
	QgsPolyline msmList;
	double msmValue=-1;

	QNetworkAccessManager* networkManager = 0;

	MeasurementDialog * msmWindow = 0;

	QSqlQueryModel * objModel = 0;
	QgsVectorLayer * objLayer = 0;
	QgsLabel * objLabels = 0;
	QgsLabelAttributes * objLabelAttributes = 0;


	void populateObjectLayer(census * obj);
};

#endif /* IMGCANVAS_H_ */

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

#include "ImgCanvas.h"
#include <qgsmultibandcolorrenderer.h>
#include <QFileDialog>
#include <QNetworkAccessManager>
#include <math.h>
#include <QMessageBox>
#include <qgsgeometry.h>
#include <qgsvectordataprovider.h>
#include <QSqlRecord>

ImgCanvas::ImgCanvas(QWidget *parent, Ui::MainWindow *mUi, ConfigHandler *cfg, DatabaseHandler *db)
	: QgsMapCanvas(parent),ui(mUi), cfg(cfg), db(db) {
	// TODO Auto-generated constructor stub

    enableAntiAliasing(true);
    setParallelRenderingEnabled( true );
    setCanvasColor(QColor(0, 0, 0));
    freeze(false);
    setCachingEnabled(true);
    setCacheMode(QgsMapCanvas::CacheBackground);

    setMapUnits(QGis::Meters);
	layerStack = new QgsLayerStack(this);

	qgsEmitPointTool = new QgsMapToolEmitPoint(this);
	qgsMapPanTool = new QgsMapToolPan(this);
	networkManager = new QNetworkAccessManager(this);

	setMapTool(qgsMapPanTool);

//    connect(ui->btnObjectMarkers, SIGNAL(clicked()), this, SLOT(handleHideObjectMarkers()));
    connect(ui->actionMarkierungen, SIGNAL(triggered()), this, SLOT(handleHideObjectMarkers()));
}

ImgCanvas::~ImgCanvas() {
	// TODO Auto-generated destructor stub
	delete qgsEmitPointTool;
	delete layerStack;
}

bool ImgCanvas::loadObject(census * obj) {
	msmValue = -1.0;

	// check if still same image
	if (curSession == obj->session && curCam == obj->camera && curImg == obj->image) {
		centerOnWorldPosition(obj->ux, obj->uy, 1.0);
		return true;
	}

	if(imgLayer) {
		layerStack->removeMapLayer("image");
	}
	this->refresh();

	curSession = obj->session;
	curCam = obj->camera;
	curImg = obj->image;

	QString file = cfg->image_path + "/cam" + obj->camera + "/geo/" + obj->image + ".tif";
	qDebug() << "Loading file " << file;

	QFileInfo info(file);
    if ( !info.isFile() || !info.isReadable() ) {
       	qDebug() << "Error: Invalid Filepath: " << file;
    } else {
    	qDebug() << "Success.";
    }
    QString basePath = info.filePath();
    QString baseName = info.fileName();

    imgLayer = new QgsRasterLayer(basePath,baseName);
    imgProvider = imgLayer->dataProvider();

    QgsContrastEnhancement* qgsContrastEnhRed = new QgsContrastEnhancement(QGis::UInt16);
    QgsContrastEnhancement* qgsContrastEnhGreen = new QgsContrastEnhancement(QGis::UInt16);
    QgsContrastEnhancement* qgsContrastEnhBlue = new QgsContrastEnhancement(QGis::UInt16);

    QgsMultiBandColorRenderer* renderer = new QgsMultiBandColorRenderer( imgProvider, 1, 2, 3,
                qgsContrastEnhRed, qgsContrastEnhGreen, qgsContrastEnhBlue);

    imgLayer->setRenderer( renderer );

    layerStack->addMapLayer("image", imgLayer, 100);
    setExtent(fullExtent());
    centerOnWorldPosition(obj->ux, obj->uy, 1.0);
    if ( !imgLayer->isValid() ) {
    	qDebug() << "Warning: Imagelayer is invalid!";

    	QMessageBox *imgerror= new QMessageBox();
    	imgerror->setText("Fehler beim Laden des Bildes.");
		imgerror->addButton(trUtf8("Abbrechen"), QMessageBox::NoRole);
		imgerror->exec();
		delete imgerror;
		return false;
    }

    objModel = db->getImageObjects(obj);

    for (uint i=0; i<objMarkers.size(); i++)
    {
    	delete objMarkers[i];
    }
    objMarkers.clear();


    for (int i=0; i<objModel->rowCount(); i++) {
    	int id = objModel->record(i).value(0).toInt();
    	double ux = objModel->record(i).value(2).toDouble();
    	double uy = objModel->record(i).value(3).toDouble();
    	int mcen = objModel->record(i).value(4).toInt();
    	int ccen = objModel->record(i).value(5).toInt();

    	QgsMapMarker * marker = new QgsMapMarker(this);
    	marker->setCenter(QgsPoint(ux,uy));
    	marker->setIconType(QgsMapMarker::ICON_CIRCLE);
    	marker->setIconSize(7);
    	marker->setFill(true);
    	marker->setText(QString::number(id));
    	marker->setTextColor(Qt::red);
    	marker->setTextWidth(2);
    	marker->setTextOffset(15,15);

    	qDebug() << id;
    	if (mcen == 2) {
    		marker->setColor(Qt::green);
    	} else if (mcen == 1) {
    		if (ccen > 1)
    			marker->setColor(Qt::red);
    		else
    			marker->setColor(Qt::gray);
    	} else {
    		marker->setColor(Qt::white);
    	}

    	objMarkers.push_back(marker);

    	marker = new QgsMapMarker(this);
		marker->setCenter(QgsPoint(ux,uy));
		marker->setIconType(QgsMapMarker::ICON_CIRCLE);
		marker->setPenWidth(1);
		marker->setIconSize(8);
		marker->setColor(Qt::black);

		objMarkers.push_back(marker);
    }

    handleHideObjectMarkers();

    return true;
}

void ImgCanvas::centerOnPixelPosition(int x, int y, double scale) {
	double w = scale*this->width()/2;
	double h = scale*this->height()/2;
	QgsPoint min = calcWorldPosition(x-w, y+h);
	QgsPoint max = calcWorldPosition(x+w, y-h);
	QgsRectangle view(min, max);
	this->setExtent(view);
	this->refresh();
}

void ImgCanvas::centerOnWorldPosition(double ux, double uy, double scale) {
	if (!imgLayer->isValid()) return;
	double w = scale*this->width()/2*imgLayer->rasterUnitsPerPixelX();
	double h = scale*this->height()/2*imgLayer->rasterUnitsPerPixelY();
	QgsPoint min(ux-w, uy-h);
	QgsPoint max(ux+w, uy+h);
	QgsRectangle view(min, max);
	this->setExtent(view);
	this->refresh();
}

QgsPoint ImgCanvas::calcWorldPosition(int x, int y) {
	qDebug() << "Calculating world position for (" << x << y << ")";
	double wx,wy;
	wx = this->fullExtent().xMinimum() + double(x)*imgLayer->rasterUnitsPerPixelX();
	wy = this->fullExtent().yMaximum() - double(y)*imgLayer->rasterUnitsPerPixelY();
	return QgsPoint(wx,wy);
}

void ImgCanvas::calcPixelPosition(QgsPoint pos) {
	double w = this->width()/2;
	double h = this->height()/2;
	QgsRectangle view(pos.x()-w, pos.y()-h, pos.x()+w, pos.y()+h);
	this->setExtent(view);
}

/*
 * Handle clicks on the map canvas with regards to the measurement tool
 */
void ImgCanvas::handleCanvasClicked(const QgsPoint & point) {
	msmList.push_back(point);


	QgsMapMarker * vmarker = new QgsMapMarker(this);
	vmarker->setIconType(QgsMapMarker::ICON_CROSS);
	vmarker->setCenter(point);
	vmarker->setIconSize(9);
	vmarker->setPenWidth(2);
	msmMarkers.push_back( vmarker );

	if (msmList.size() > 1) {
		msmValue = QgsGeometry::fromPolyline(msmList)->length();
		msmWindow->updateInfoMessage(
			QString::fromUtf8("Momentane Länge: ") + QString::number(msmValue));
	}
}

QgsRasterLayer * ImgCanvas::getImageLayer() { return imgLayer; }

void ImgCanvas::beginMeasurement(MeasurementDialog * msmDialog) {
	msmList.clear();
	msmValue = -1.0;
	msmWindow = msmDialog;
	setMapTool(qgsEmitPointTool);
    connect(qgsEmitPointTool, SIGNAL( canvasClicked(const QgsPoint &, Qt::MouseButton) ),
    		this, SLOT( handleCanvasClicked(const QgsPoint &)));

}

double ImgCanvas::endMeasurement() {
	setMapTool(qgsMapPanTool);
	disconnect(qgsEmitPointTool, SIGNAL( canvasClicked(const QgsPoint &, Qt::MouseButton) ),
    		this, SLOT( handleCanvasClicked(const QgsPoint &)));
	msmList.clear();
	for (uint i=0; i<msmMarkers.size(); i++)
		delete msmMarkers[i];
	msmMarkers.clear();
	return msmValue;
}

void ImgCanvas::setRasterBrightness(int value) {
	QgsRasterLayer * rlyr = static_cast<QgsRasterLayer*>(layerStack->getMapLayer("image"));
	rlyr->brightnessFilter()->setBrightness(value);
	refresh();
}

void ImgCanvas::setRasterContrast(int value) {
	QgsRasterLayer * rlyr = static_cast<QgsRasterLayer*>(layerStack->getMapLayer("image"));
	rlyr->brightnessFilter()->setContrast(value);
	refresh();
}

double ImgCanvas::getCurrentMeasurement() {
	return msmValue;
}

void ImgCanvas::handleHideObjectMarkers() {
//	if (ui->btnObjectMarkers->isChecked()){
	if (ui->actionMarkierungen->isChecked()){
		for (uint i=0; i<objMarkers.size(); i++)
			objMarkers[i]->show();
	} else {
		for (uint i=0; i<objMarkers.size(); i++)
			objMarkers[i]->hide();
	}
	refresh();
}

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

ImgCanvas::ImgCanvas(QWidget *parent, Ui::MainWindow *mUi, ConfigHandler *cfg) : QgsMapCanvas(parent),ui(mUi), cfg(cfg) {
	// TODO Auto-generated constructor stub

    enableAntiAliasing(true);
    setParallelRenderingEnabled( true );
    setCanvasColor(QColor(0, 0, 0));
    freeze(false);

    imgLayer    = 0;
    imgProvider = 0;

    setMapUnits(QGis::Meters);
	layerStack = new QgsLayerStack(this);

	qgsEmitPointTool = new QgsMapToolEmitPoint(this);
	networkManager = new QNetworkAccessManager(this);
    setMapTool(qgsEmitPointTool);

    // TODO: Variable UTM sector
    QString props = QString("Point?")+
                       QString("crs=epsg:32632");

    msmLayer = new QgsVectorLayer(props, "msm", "memory");
    layerStack->addMapLayer("msm",msmLayer, 10);

}

ImgCanvas::~ImgCanvas() {
	// TODO Auto-generated destructor stub
	delete qgsEmitPointTool;
	delete layerStack;
}

bool ImgCanvas::loadObject(census * obj, double * pos) {
	if(imgLayer) {
		layerStack->removeMapLayer("image");
//		delete imgLayer;
//		imgLayer = new QgsRasterLayer();
//		imgProvider = imgLayer->dataProvider();
	}
	this->refresh();
	QString file;
	if (cfg->getSessionType() == "local") {

		file = cfg->image_path + "/cam" + QString::number(obj->camera) + "/geo/" + obj->image + ".tif";
		qDebug() << "Loading file " << file;
	} else if (cfg->getSessionType() == "pfz") {
		QString img = QString::number(obj->id) + "-" + obj->image + ".tif";
		QUrl url("http://platform-z.ifaoe.de/daisi/" + obj->session + "/cam" + QString::number(obj->camera) + "/crop/" + img);
		qDebug() << "Getting cropped image from" << url.toString();
		QEventLoop eventloop;
		QNetworkReply* reply = networkManager->get(QNetworkRequest(url));
	    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
	            &eventloop, SLOT(quit()));
	    eventloop.exec(QEventLoop::ExcludeUserInputEvents);
		QFile imgfile("/tmp/birdview-tmp" + QString::number(obj->id) + ".tif");
		imgfile.open(QIODevice::WriteOnly);
		imgfile.write(reply->readAll());
		imgfile.close();
		file = "/tmp/birdview-tmp" + QString::number(obj->id) + ".tif";
	} else {
		return false;
	}

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
    centerOnWorldPosition(pos[0], pos[1], 1.0);
    if ( !imgLayer->isValid() ) {
    	qDebug() << "Warning: Imagelayer is invalid!";

    	QMessageBox *imgerror= new QMessageBox();
    	imgerror->setText("Fehler beim Laden des Bildes.");
		imgerror->addButton(trUtf8("Abbrechen"), QMessageBox::NoRole);
		imgerror->exec();
		delete imgerror;
		return false;
    }
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
	QgsFeature fet;
	fet.setGeometry(QgsGeometry::fromPoint(point));
	msmLayer->startEditing();
	msmLayer->addFeature(fet);
	msmLayer->commitChanges();
}

QgsRasterLayer * ImgCanvas::getImageLayer() { return imgLayer; }

void ImgCanvas::beginMeasurement() {
	msmList.clear();
    connect(qgsEmitPointTool, SIGNAL( canvasClicked(const QgsPoint &, Qt::MouseButton) ),
    		this, SLOT( handleCanvasClicked(const QgsPoint &)));
}

double ImgCanvas::endMeasurement() {
	double dist=0.0;
	if (msmList.size() < 2) return 0.0;
	for (uint i=0; i<msmList.size()-1; i++) {
		dist += sqrt(msmList[i].sqrDist(msmList[i+1]));
	}
	disconnect(qgsEmitPointTool, SIGNAL( canvasClicked(const QgsPoint &, Qt::MouseButton) ),
    		this, SLOT( handleCanvasClicked(const QgsPoint &)));
	msmList.clear();
	QgsFeatureIds ids;
	QgsFeatureIterator fit = msmLayer->dataProvider()->getFeatures();
	QgsFeature fet;
	while(fit.nextFeature(fet)) {
		ids.insert(fet.id());
	}
	msmLayer->startEditing();
	msmLayer->dataProvider()->deleteFeatures(ids);
	msmLayer->commitChanges();
	return dist;
}

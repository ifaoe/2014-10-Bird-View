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
    connect(qgsEmitPointTool, SIGNAL( canvasClicked(const QgsPoint &, Qt::MouseButton) ),
    		this, SLOT( handleCanvasClicked(const QgsPoint &)));
    setMapTool(qgsEmitPointTool);
}

ImgCanvas::~ImgCanvas() {
	// TODO Auto-generated destructor stub
	delete qgsEmitPointTool;
	delete layerStack;
}

bool ImgCanvas::loadObject(census * obj, double * pos) {
	QString file;
	if (cfg->session_type == "local") {

		file = cfg->image_path + "/cam" + QString::number(obj->camera) + "/geo/" + obj->image + ".tif";

	} else if (cfg->session_type == "pfz") {
		QString img = QString::number(obj->id) + "-" + obj->image + ".tif";
		QUrl url("http://platform-z.ifaoe.de/daisi/" + obj->session + "/cam" + QString::number(obj->camera) + "/crop/" + img);
		qDebug() << "Getting cropped image from" << url.toString();
		QEventLoop eventloop;
		QNetworkReply* reply = networkManager->get(QNetworkRequest(url));
	    connect(networkManager, SIGNAL(finished(QNetworkReply*)),
	            &eventloop, SLOT(quit()));
	    eventloop.exec();
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
        return false;
    }

	if(imgLayer) {
		layerStack->removeMapLayer("image");
		imgLayer = 0;
	}
    QString basePath = info.filePath();
    QString baseName = info.fileName();

    imgLayer = new QgsRasterLayer(basePath,baseName);


    if ( !imgLayer->isValid() ) {
    	qDebug() << "Warning: Imagelayer is invalid!";
    	return false;
    }

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
	if (msmList.size() == 1) {
		msmList.append(point);
		QgsPoint p0 = msmList.at(0);
		QgsPoint p1 = msmList.at(1);
		double distance = sqrt(pow(p0.x()-p1.x(),2) + pow(p0.y() - p1.y(),2) );
		ui->lblMsmTool->setText("Zweiter Punkt gesetzt. Berechnete Distanz :" + QString::number(distance) + "m");


	} else {
		msmList.clear();
		msmList.append(point);
		ui->lblMsmTool->setText("Erster Punkt gesetzt. Bitte 2. Punkt setzen.");
	}
}

void ImgCanvas::paintEvent(QPaintEvent * event) {
	QgsMapCanvas::paintEvent(event);
// TODO: paintEvent
//	if (msmList.size() == 2) {
//		const QgsMapToPixel * mapper = this->getCoordinateTransform();
//		QgsPoint p0 = msmList.at(0);
//		QgsPoint p1 = msmList.at(1);
//		QPen pen;
//		pen.setColor(Qt::red);
//		pen.setWidth(4);
//		msmLine = new QPainter(this);
//		msmLine->setPen(pen);
//		msmLine->begin(this);
//		msmLine->drawLine(mapper->transform(p0).x(), mapper->transform(p0).y(), mapper->transform(p1).x(), mapper->transform(p1).y());
//		msmLine->end();
//	}
}

QgsRasterLayer * ImgCanvas::getImageLayer() { return imgLayer; }

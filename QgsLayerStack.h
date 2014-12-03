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

#ifndef QGSLAYERSTACK_H_
#define QGSLAYERSTACK_H_

#include <QObject>
#include <qgsmapcanvas.h>
#include <qgsmaplayer.h>
#include <qgsmaplayerregistry.h>

class QgsLayerStack: public QObject {
public:
	QgsLayerStack(QgsMapCanvas * mapCanvas);
	virtual ~QgsLayerStack();

	bool addMapLayer(QString layerName, QgsMapLayer *mapLayer, int priority);
	bool removeMapLayer(QString layerName);
	bool setLayerPriority(QString layerName, int priority);
	QgsMapLayer * getMapLayer(QString layerName);
	QList<QString> getLayerNames();

private:
	void registerMapLayers();
	void refreshLayerSet();
	QgsMapLayerRegistry* qgsLyrRegistry = 0;
	QgsMapCanvas *mapCanvas;
	QMap<int,QgsMapLayer*> lyrMap;	// Sorted in ascending order by key: c++ standard
	QMap<QString,int> priMap;
};

#endif /* QGSLAYERSTACK_H_ */

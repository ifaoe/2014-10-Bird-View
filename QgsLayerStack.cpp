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

#include "QgsLayerStack.h"

QgsLayerStack::QgsLayerStack(QgsMapCanvas * canvas) : mapCanvas(canvas) {
	// TODO Auto-generated constructor stub
	qgsLyrRegistry = QgsMapLayerRegistry::instance();
}

QgsLayerStack::~QgsLayerStack() {
	// TODO Auto-generated destructor stub
}

bool QgsLayerStack::addMapLayer(QString layerName, QgsMapLayer * mapLayer, int priority) {
	if(lyrMap.contains(priority)) {
		qWarning("Warning addMapLayer: Layer with priority %d already exists.", priority);
		return false;
	} else if (priMap.contains(layerName)){
		qWarning("Warning addMapLayer: Layer with name %s already exists.", layerName.toStdString().c_str());
		return false;
	} else {
		lyrMap[priority] = mapLayer;
		priMap[layerName]= priority;
		qgsLyrRegistry->addMapLayer(mapLayer);
		qgsLyrRegistry->reloadAllLayers();
		refreshLayerSet();
		return true;
	}
}

bool QgsLayerStack::removeMapLayer(QString layerName) {
	if(!priMap.contains(layerName)) {
		qWarning("Warning removeMapLayer: No layer with name %s found.", layerName.toStdString().c_str());
		return false;
	} else {
		int priority = priMap[layerName];
		priMap.remove(layerName);
		lyrMap.remove(priority);
		refreshLayerSet();
		return true;
	}
}

QgsMapLayer * QgsLayerStack::getMapLayer(QString layerName) {
	if (!priMap.contains(layerName)) {
		return NULL;
		qWarning("Warning getMapLayer: No layer named: %s found.", layerName.toStdString().c_str());
	}
	int priority = priMap[layerName];
	return lyrMap[priority];
}

void QgsLayerStack::registerMapLayers() {
	QMap<int,QgsMapLayer*>::iterator i;
	for(i = lyrMap.begin(); i != lyrMap.end(); ++i) {
		qgsLyrRegistry->addMapLayer(i.value());
	}
	qgsLyrRegistry->reloadAllLayers();
}

bool QgsLayerStack::setLayerPriority(QString layerName, int priority){
	if(lyrMap.contains(priority)) {
		qWarning("Warning setLayerPriority: Layer with priority %d already exists.", priority);
		return false;
	} else if (!priMap.contains(layerName)) {
		qWarning("Warning setLayerPriority: No layer with name %s found.", layerName.toStdString().c_str());
		return false;
	} else {
		// delete old mapping and create new one
		int oldPri = priMap[layerName];
		QgsMapLayer * temp = lyrMap[oldPri];
		priMap[layerName] = priority;
		lyrMap.remove(oldPri);
		lyrMap[priority] = temp;
		return true;
	}
}

void QgsLayerStack::refreshLayerSet() {
	QMap<int,QgsMapLayer*>::iterator i;
	QList<QgsMapCanvasLayer> layerSet;
	if (lyrMap.size() == 0) return;
	for(i = lyrMap.begin(); i != lyrMap.end(); ++i) {
		layerSet.append(QgsMapCanvasLayer(i.value()));
	}
	mapCanvas->setLayerSet(layerSet);
}

QList<QString> QgsLayerStack::getLayerNames() {
	return priMap.keys();
}

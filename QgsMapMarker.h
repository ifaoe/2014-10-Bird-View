/*
 * QgsMapMarker.h
 *
 *  Created on: Jun 18, 2015
 *      Author: awg
 */

#ifndef QGSMAPMARKER_H_
#define QGSMAPMARKER_H_

#include <qgsvertexmarker.h>
#include <QPainter>

class QgsMapMarker: public QgsVertexMarker {
public:
	QgsMapMarker(QgsMapCanvas * canvas);
	virtual ~QgsMapMarker();
	void paint( QPainter * p);

    enum IconType
    {
      ICON_NONE,
      ICON_CROSS,
      ICON_X,
      ICON_BOX,
	  ICON_CIRCLE
    };
private:

};

#endif /* QGSMAPMARKER_H_ */

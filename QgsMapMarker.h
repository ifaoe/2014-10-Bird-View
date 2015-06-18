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
	void setFill(bool filled = false);
	void setText(QString label = "");
	void setTextWidth(int width);
	void setTextColor(QColor color);
	void setTextOffset(double x, double y);

    enum IconType
    {
      ICON_NONE,
      ICON_CROSS,
      ICON_X,
      ICON_BOX,
	  ICON_CIRCLE
    };

private:
    bool mFilled = false;
    QString text = "";
    int mTextWidth = 1;
    QColor mTextColor = Qt::red;
    double mTextOffsetX = 0.0;
    double mTextOffsetY = 0.0;
};

#endif /* QGSMAPMARKER_H_ */

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
    void setFillColor(QColor color) {mFillColor = color;}
    void setDrawWidth(int width) {mPenWidth = width;}
    void setText(QString label = "");
    void setTextWidth(int width);
    void setTextColor(QColor color);
    void setTextOffset(double x, double y);
    void setIconColor(QColor color) {mColor = color;}

    enum IconType
    {
      ICON_NONE,
      ICON_CROSS,
      ICON_X,
      ICON_BOX,
      ICON_CIRCLE,
	  ICON_TRIANGLE,
	  ICON_DIAMOND,
      ICON_CROSS_BOX,
      ICON_CROSS_CIRCLE,
	  ICON_CROSS_TRIANGLE,
	  ICON_CROSS_DIAMOND,
	  ICON_X_BOX,
	  ICON_X_CIRCLE,
	  ICON_X_DIAMOND,
    };

private:
    QColor mColor = Qt::red;
    bool mFilled = false;
    QColor mFillColor =Qt::yellow;
    int mPenWidth = 1;
    QString text = "";
    int mTextWidth = 1;
    QColor mTextColor = Qt::red;
    double mTextOffsetX = 0.0;
    double mTextOffsetY = 0.0;
};

#endif /* QGSMAPMARKER_H_ */

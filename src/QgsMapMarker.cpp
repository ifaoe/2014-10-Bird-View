/*
 * QgsMapMarker.cpp
 *
 *  Created on: Jun 18, 2015
 *      Author: awg
 */

#include "QgsMapMarker.h"

QgsMapMarker::QgsMapMarker(QgsMapCanvas * canvas) : QgsVertexMarker(canvas) {
    // TODO Auto-generated constructor stub
}

QgsMapMarker::~QgsMapMarker() {
    // TODO Auto-generated destructor stub
}

void QgsMapMarker::setFill(bool filled) {
    mFilled = filled;
}

void QgsMapMarker::paint(QPainter * p) {
    qreal s = mIconSize;
    p->setRenderHint(QPainter::Antialiasing);
    QPen pen( mColor );
    pen.setWidth( mPenWidth );
    p->setPen( pen );
    if (mFilled)
        p->setBrush( QBrush(mFillColor) );
    else
        p->setBrush(Qt::NoBrush);
    switch ( mIconType )
    {
        case ICON_NONE:
        {
            break;
        }
        case ICON_CROSS:
        {
            p->drawLine( QLineF( -s, 0, s, 0 ) );
            p->drawLine( QLineF( 0, -s, 0, s ) );
            break;
        }
        case ICON_X:
        {
            p->drawLine( QLineF( -s, -s, s, s ) );
            p->drawLine( QLineF( -s, s, s, -s ) );
            break;
        }
        case ICON_BOX:
        {
        	QPoint points[] = {
        			QPoint(-s,-s),
					QPoint(s,-s),
					QPoint(s,s),
					QPoint(-s,s)
        	};
        	p->drawPolygon(points,4);
            break;
        }
        case ICON_CIRCLE:
        {
            p->drawEllipse(-s, -s, 2*s, 2*s);
            break;
        }
        case ICON_TRIANGLE:
        {
            QPoint points[] = {
            		QPoint(-s,-s),
					QPoint(s,-s),
					QPoint(0,s)
            };
            p->drawPolygon(points,3);
            break;
        }
        case ICON_DIAMOND:
        {
        	QPoint points[] = {
        			QPoint(-s,0),
					QPoint(0,s),
					QPoint(s,0),
					QPoint(0,-s)
        	};
        	p->drawPolygon(points,4);
        	break;
        }
        case ICON_CROSS_BOX:
        {
        	//box
        	QPoint points[] = {
        			QPoint(-s,-s),
					QPoint(s,-s),
					QPoint(s,s),
					QPoint(-s,s)
        	};
        	p->drawPolygon(points,4);
            //cross
            pen.setWidth( ceil(mPenWidth/2) );
            p->setPen(pen);
            p->drawLine( QLineF( -s, 0, s, 0 ) );
            p->drawLine( QLineF( 0, -s, 0, s ) );
            break;
        }
        case ICON_X_BOX:
        {
        	//box
        	QPoint points[] = {
        			QPoint(-s,-s),
					QPoint(s,-s),
					QPoint(s,s),
					QPoint(-s,s)
        	};
        	p->drawPolygon(points,4);
            //X
            pen.setWidth( ceil(mPenWidth/2) );
            p->setPen(pen);
            p->drawLine( QLineF( -s, -s, s, s ) );
            p->drawLine( QLineF( -s, s, s, -s ) );
            break;
        }
        case ICON_CROSS_TRIANGLE:
        {
            //triangle
            QPoint points[] = {
            		QPoint(-s,-s),
					QPoint(s,-s),
					QPoint(0,s)
            };
            p->drawPolygon(points,3);
        	//cross
            pen.setWidth( ceil(mPenWidth/2) );
            p->setPen(pen);
            p->drawLine( QLineF( -s/2, 0, s/2, 0 ) );
            p->drawLine( QLineF( 0, -s, 0, s ) );
        	break;
        }
        case ICON_CROSS_CIRCLE:
        {
            //circle
            p->drawEllipse(-s, -s, 2*s, 2*s);
        	//cross
            pen.setWidth( ceil(mPenWidth/2) );
            p->setPen(pen);
            p->drawLine( QLineF( -s, 0, s, 0 ) );
            p->drawLine( QLineF( 0, -s, 0, s ) );
            break;
        }
        case ICON_X_CIRCLE:
        {
            //circle
            p->drawEllipse(-s, -s, 2*s, 2*s);
        	//cross
            pen.setWidth( ceil(mPenWidth/2) );
            p->setPen(pen);
            double s_x = s/sqrt(2);
            p->drawLine( QLineF( -s_x, -s_x, s_x, s_x ) );
            p->drawLine( QLineF( -s_x, s_x, s_x, -s_x ) );
            break;
        }
        case ICON_CROSS_DIAMOND:
        {
            //diamond
        	QPoint points[] = {
        			QPoint(-s,0),
					QPoint(0,s),
					QPoint(s,0),
					QPoint(0,-s)
        	};
        	p->drawPolygon(points,4);
        	//cross
            pen.setWidth( ceil(mPenWidth/2) );
            p->setPen(pen);
            p->drawLine( QLineF( -s, 0, s, 0 ) );
            p->drawLine( QLineF( 0, -s, 0, s ) );

        	break;
        }
        case ICON_X_DIAMOND:
        {
            //diamond
        	QPoint points[] = {
        			QPoint(-s,0),
					QPoint(0,s),
					QPoint(s,0),
					QPoint(0,-s)
        	};
        	p->drawPolygon(points,4);
        	//cross
            pen.setWidth( ceil(mPenWidth/2) );
            p->setPen(pen);
        	s = s/2;
            p->drawLine( QLineF( -s, -s, s, s ) );
            p->drawLine( QLineF( -s, s, s, -s ) );

        	break;
        }
    }
    if (text != "") {
        QPen textPen(mTextColor);
        textPen.setWidth(mTextWidth);
        textPen.setColor(mTextColor);
        p->setPen(textPen);
        p->drawText(QPointF(mTextOffsetX,mTextOffsetY), text);
    }
}

void QgsMapMarker::setText(QString label) {
    text = label;
}

void QgsMapMarker::setTextWidth(int width) {
    mTextWidth = width;
}

void QgsMapMarker::setTextColor(QColor color) {
    mTextColor = color;
}

void QgsMapMarker::setTextOffset(double x, double y) {
    mTextOffsetX = x;
    mTextOffsetY = y;
}

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
	qreal s = ( mIconSize - 1 ) / 2.0;

	QPen pen( mColor );
	pen.setWidth( mPenWidth );
	p->setPen( pen );
	if (mFilled)
		p->setBrush( QBrush(mColor) );
	else
		p->setBrush(Qt::NoBrush);
	switch ( mIconType )
	{
		case ICON_NONE:
			break;

		case ICON_CROSS:
			p->drawLine( QLineF( -s, 0, s, 0 ) );
			p->drawLine( QLineF( 0, -s, 0, s ) );
			break;

		case ICON_X:
			p->drawLine( QLineF( -s, -s, s, s ) );
			p->drawLine( QLineF( -s, s, s, -s ) );
			break;

		case ICON_BOX:
			p->drawLine( QLineF( -s, -s, s, -s ) );
			p->drawLine( QLineF( s, -s, s, s ) );
			p->drawLine( QLineF( s, s, -s, s ) );
			p->drawLine( QLineF( -s, s, -s, -s ) );
			break;
		case ICON_CIRCLE:
			p->drawEllipse(-s, -s, 2*s, 2*s);
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

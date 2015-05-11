/*
 * QCategoryCheckButton.cpp
 *
 *  Created on: May 6, 2015
 *      Author: awg
 */

#include "QCategoryCheckButton.h"

#include <QHBoxLayout>

QCategoryCheckButton::QCategoryCheckButton( const QString& a_Text,
        QTreeWidget* a_pParent, QTreeWidgetItem* a_pItem ) : QFrame(a_pParent)
    , m_pItem(a_pItem)
{
	setFrameStyle(QFrame::Raised);
	btnCategory = new QPushButton(a_Text, this);
	chbCategory = new QPushButton("", this);
	chbCategory->setCheckable(true);
	chbCategory->setChecked(false);
	chbCategory->setFixedSize(btnCategory->height(),btnCategory->height());

	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(chbCategory);
	layout->addWidget(btnCategory);
	layout->setSpacing(0);
	layout->setContentsMargins(0,0,0,0);
	setLayout(layout);
	setContentsMargins(0,0,0,0);

	setBackgroundRole(QPalette::Window);
	setAutoFillBackground(true);

    connect(btnCategory, SIGNAL(pressed()), this, SLOT(ButtonPressed()));
    connect(chbCategory, SIGNAL(clicked()), this, SLOT(CheckBoxSwitched()));
}

void QCategoryCheckButton::ButtonPressed()
{
    m_pItem->setExpanded( !m_pItem->isExpanded() );
}

void QCategoryCheckButton::CheckBoxSwitched() {
	if (!chbCategory->isChecked() ) {
		btnCategory->setChecked(false);
		btnCategory->setEnabled(true);
		m_pItem->setExpanded(false);
	} else {
		btnCategory->setChecked(true);
		btnCategory->setEnabled(false);
		m_pItem->setExpanded(true);
	}
}

bool QCategoryCheckButton::isChecked() {
	return chbCategory->isChecked();
}

//QCategoryCheckButton::~QCategoryCheckButton() {
//	// TODO Auto-generated destructor stub
//}


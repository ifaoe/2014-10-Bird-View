/*
 * QCategoryButton.cpp
 *
 *  Created on: May 6, 2015
 *      Author: awg
 */

#include "QCategoryButton.h"
#include <QHBoxLayout>

QCategoryButton::QCategoryButton( const QString& a_Text,
        QTreeWidget* a_pParent, QTreeWidgetItem* a_pItem ) : QWidget(a_pParent)
    , m_pItem(a_pItem)
{
	btnCategory = new QPushButton(a_Text, this);
	chbCategory = new QCheckBox;
	chbCategory->setChecked(false);
	;
	QHBoxLayout *layout = new QHBoxLayout;
	layout->addWidget(chbCategory);
	chbCategory->setFixedSize(30,30);
	layout->addWidget(btnCategory);
	this->setLayout(layout);

	setBackgroundRole(QPalette::Window);
	setAutoFillBackground(true);

    connect(btnCategory, SIGNAL(pressed()), this, SLOT(ButtonPressed()));
    connect(chbCategory, SIGNAL(stateChanged(int)), this, SLOT(CheckBoxSwitched(int)));
}

void QCategoryButton::ButtonPressed()
{
    m_pItem->setExpanded( !m_pItem->isExpanded() );
}

void QCategoryButton::CheckBoxSwitched(int state) {
	if (state == Qt::Unchecked ) {
		btnCategory->setChecked(false);
		btnCategory->setEnabled(true);
		m_pItem->setExpanded(false);
	} else if (state == Qt::Checked) {
		btnCategory->setChecked(true);
		btnCategory->setEnabled(false);
		m_pItem->setExpanded(true);
	} else {
		return;
	}

}

bool QCategoryButton::isChecked() {
	return chbCategory->isChecked();
}

//QCategoryButton::~QCategoryButton() {
//	// TODO Auto-generated destructor stub
//}


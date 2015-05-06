/*
 * QCategoryButton.cpp
 *
 *  Created on: May 6, 2015
 *      Author: awg
 */

#include "QCategoryButton.h"

QCategoryButton::QCategoryButton( const QString& a_Text,
        QTreeWidget* a_pParent, QTreeWidgetItem* a_pItem )
    : QPushButton(a_Text, a_pParent)
    , m_pItem(a_pItem)
{
    connect(this, SIGNAL(pressed()), this, SLOT(ButtonPressed()));
}

void QCategoryButton::ButtonPressed()
{
    m_pItem->setExpanded( !m_pItem->isExpanded() );
}

//QCategoryButton::~QCategoryButton() {
//	// TODO Auto-generated destructor stub
//}


/*
 * QCategoryButton.cpp
 *
 *  Created on: May 6, 2015
 *      Author: awg
 */

#include "QCategoryButton.h"
#include <QVBoxLayout>

QCategoryButton::QCategoryButton( const QString& a_Text, QTreeWidget* a_pParent,
        QTreeWidgetItem* a_pItem ) : QFrame(a_pParent), m_pItem(a_pItem)
{
    setBackgroundRole(QPalette::Window);
    setAutoFillBackground(true);

    btnCategory = new QPushButton(a_Text, this);
    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(btnCategory);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

    setContentsMargins(0,0,0,0);

    connect(btnCategory, SIGNAL(pressed()), this, SLOT(ButtonPressed()));
}

void QCategoryButton::ButtonPressed()
{
    m_pItem->setExpanded( !m_pItem->isExpanded() );
}

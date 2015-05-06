/*
 * QCategoryButton.h
 *
 *  Created on: May 6, 2015
 *      Author: awg
 */

#ifndef QCATEGORYBUTTON_H_
#define QCATEGORYBUTTON_H_

#include <qpushbutton.h>
#include <QTreeWidget>

class QCategoryButton: public QPushButton {
	Q_OBJECT
public:
    QCategoryButton(const QString& a_Text, QTreeWidget* a_pParent,
        QTreeWidgetItem* a_pItem);

private slots:
    void ButtonPressed();

private:
    QTreeWidgetItem* m_pItem;
};

#endif /* QCATEGORYBUTTON_H_ */

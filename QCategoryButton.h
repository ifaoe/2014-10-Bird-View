/*
 * QCategoryButton.h
 *
 *  Created on: May 6, 2015
 *      Author: awg
 */

#ifndef QCATEGORYBUTTON_H_
#define QCATEGORYBUTTON_H_

#include <QPushButton>
#include <QCheckBox>
#include <QTreeWidget>

class QCategoryButton: public QWidget {
	Q_OBJECT
public:
    QCategoryButton(const QString& a_Text, QTreeWidget* a_pParent,
        QTreeWidgetItem* a_pItem);
    bool isChecked();

private slots:
    void ButtonPressed();
    void CheckBoxSwitched(int state);

private:
    QTreeWidgetItem* m_pItem;
    QPushButton * btnCategory;
    QCheckBox * chbCategory;
};

#endif /* QCATEGORYBUTTON_H_ */

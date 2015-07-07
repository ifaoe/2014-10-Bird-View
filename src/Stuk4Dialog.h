/*
 * Stuk4Dialog.h
 *
 *  Created on: May 4, 2015
 *      Author: awg
 */

#ifndef STUK4DIALOG_H_
#define STUK4DIALOG_H_

#include <qdialog.h>
#include "DatabaseHandler.h"
#include "ui_stuk4codes.h"

class Stuk4Dialog: public QDialog {
    Q_OBJECT;
public:
    Stuk4Dialog(DatabaseHandler * db, QStringList * s4beh, QStringList * s4ass);
    virtual ~Stuk4Dialog();
private slots:
    void handleSaveButton();
    void handleDiscardButton();
private:
    DatabaseHandler * db;
    QStringList * stuk4_beh;
    QStringList * stuk4_ass;
    Ui::dlgStuk4Codes * dlg;
    QModelIndex getBehaviourIndex(QString code);
    QModelIndex getAssociationsIndex(QString code);
protected:
    void resizeEvent(QResizeEvent * e);
};

#endif /* STUK4DIALOG_H_ */

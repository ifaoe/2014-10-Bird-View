//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>

#ifndef SESSIONDIALOG_H_
#define SESSIONDIALOG_H_

#include <QDialog>
#include "ConfigHandler.h"
#include "ui_sessiondialog.h"

class SessionDialog : public QDialog{
    Q_OBJECT;
public:
    SessionDialog(ConfigHandler * cfg);
    virtual ~SessionDialog();
private slots:
    void handleYesButton();
    void handleNoButton();
private:
    ConfigHandler * cfg = 0;
    Ui::dlgModeSelect *dlg = 0;
};

#endif /* SESSIONDIALOG_H_ */
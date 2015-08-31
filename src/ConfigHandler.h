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
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CONFIGHANDLER_H_
#define CONFIGHANDLER_H_

#include <QSettings>
#include <QDebug>

struct DatabaseInfo {
	QString id = "local";
	QString host = "localhost";
	int port = 5432;
	QString name = "daisi";
	QString password = "18ifaoe184";
	QString user = "daisi";
};

class ConfigHandler : public QSettings{
public:
	void InitSettings();
	void AddDatabase(const QString & id, const QString & host, int port, const QString & name,const QString & user,
			const QString & password);
	QString user() {return user_;}
	void setAppPosition(QPoint pos);
	QPoint getAppPosition();
	void setAppSize(QSize size);
	QSize getAppSize();
	void setAppMaximized(bool max);
	bool getAppMaximized();
	void setPreferredDatabase(const QString & database);
	QString getPreferredDatabase();
	void setPreferredSession(const QString & session);
	QString getPreferredSession();
	QStringList getDatabaseList();
	DatabaseInfo getDatabaseInfo(const QString & id);
private:
    QString user_;
};

#endif /* CONFIGHANDLER_H_ */

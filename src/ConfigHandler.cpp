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

#include "ConfigHandler.h"
#include <QFile>
#include <QSize>
#include <QPoint>
#include <QStringList>

void ConfigHandler::InitSettings() {
	user_ = QString(getenv("USER"));
	if (!QFile::exists(fileName())) {
		AddDatabase("Rostock", "192.168.118.35", 5432, "daisi", "daisi","18ifaoe184");
		AddDatabase("Hamburg", "192.168.200.35", 5432, "daisi", "daisi","18ifaoe184");
	}
}

void ConfigHandler::AddDatabase(const QString & id, const QString & host, int port, const QString & name,const QString & user,
		const QString & password) {
beginGroup("Database");
{
	beginGroup(id);
	setValue("host",QString(host));
	setValue("port", port);
	setValue("name", name);
	setValue("password", password);
	setValue("user",user);
	endGroup();
}
endGroup();
}

void ConfigHandler::setPreferredDatabase(const QString & database) {
	setValue("Database/preferred", database);
}

QString ConfigHandler::getPreferredDatabase() {
	return value("Database/preferred","").toString();
}

void ConfigHandler::setSessionName(const QString & session) {
	setValue("session", session);
}

QString ConfigHandler::getPreferredSession() {
	return value("session","").toString();
}

void ConfigHandler::setAppSize(QSize size) {
	setValue("MainWindow/size",size);
}

QSize ConfigHandler::getAppSize() {
	return value("MainWindow/size",QSize(800,600)).toSize();
}

void ConfigHandler::setAppPosition(QPoint pos) {
	setValue("MainWindow/position",pos);
}

QPoint ConfigHandler::getAppPosition() {
	return value("MainWindow/position",QPoint(50,50)).toPoint();
}

void ConfigHandler::setAppMaximized(bool max) {
	setValue("MainWindow/maximized",max);
}

bool ConfigHandler::getAppMaximized() {
	return value("MainWindow/maximized",false).toBool();
}

QStringList ConfigHandler::getDatabaseList() {
	beginGroup("Database");
	QStringList children = childGroups();
	endGroup();
	return children;
}

DatabaseInfo ConfigHandler::getDatabaseInfo(const QString & id) {
	DatabaseInfo info;
	beginGroup("Database");
	if (!childGroups().contains(id)) {
		return info;
		endGroup();
	}
		beginGroup(id);
		info.id = id;
		info.host = value("host",QString("localhost")).toString();
		info.port = value("port", 5432).toInt();
		info.name = value("name", "daisi").toString();
		info.password = value("password", "18ifaoe184").toString();
		info.user = value("user","daisi").toString();
		endGroup();
	endGroup();
	return info;
}

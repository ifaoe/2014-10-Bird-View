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

#include <string>
#include <QFile>
#include <QDebug>
#include <QStringList>
#include <boost/property_tree/ptree.hpp>

class ConfigHandler {
public:
	ConfigHandler(int argc, char *argv[]);
	virtual ~ConfigHandler();

	QString dbHost = "localhost";
	QString dbName = "daisi";
	QString dbUser = "daisi";
	QString dbPass = "18ifaoe184";
	QString dbPort = "5432";
	QString user();
	QString image_path = "/net";
	void parseCfgFile(QString database);
	QStringList getDbList();
	QString getSessionType();
	void setSessionType(QString type);
private:
	QString session_type = "local";
	QString usr;
	QFile *cfgFile;
	QStringList databaseList;
	boost::property_tree::ptree cfg;
};

#endif /* CONFIGHANDLER_H_ */

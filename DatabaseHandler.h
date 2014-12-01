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

#ifndef DATABASEHANDLER_H_
#define DATABASEHANDLER_H_

#include <QSqlDatabase>
#include <QStringList>
#include <QComboBox>
#include "ConfigHandler.h"
#include "census.h"

//struct census;

class DatabaseHandler {
public:
	DatabaseHandler(ConfigHandler *cfg);
	virtual ~DatabaseHandler();
	QStringList getSessionList();
	QSqlQuery * getObjectResult(QString session);
	census * getRawObjectData(QString objId, QString usr);
	double * getObjectPosition(QString objId);
	QStringList getBirdTypeList();
	QStringList getUserList(QString objId);
	QMap<int, int> getObjectDone(QString usr, QString session);
	QMap<int, int> getObjectFinal(QString session);
	census * getCensusData(QString objId);
	void writeCensus(census * obj);
	QString getProjectPath(QString session);
private:
	QSqlDatabase *db;
	ConfigHandler *cfg;
	void setRecordTable(QSqlRecord * record, census * obj);
};

//// C Struct which resembles the database record structure
//struct census {
//	int id = -1;
//	QString session = "";
//	QString image = "";
//	int camera = -1;
//	QString usr = "";
//	QString epsg = "";
//	int px = -1;
//	int py = -1;
//	double ux = -0.0;
//	double uy = -0.0;
//	double lx = -0.0;
//	double ly = -0.0;
//	QString type = "";
//	QString name = "";
//	int quality = -1;
//	QString behavior = "";
//	QString gender = "";
//	QString age = "";
//	int direction = -1;
//	QString remarks = "";
//	int censor = 0;
//	int imageQuality = 0;
//};

#endif /* DATABASEHANDLER_H_ */

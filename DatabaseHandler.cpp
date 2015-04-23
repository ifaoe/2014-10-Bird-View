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

#include "DatabaseHandler.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QSqlTableModel>
#include <qsql_psql.h>

DatabaseHandler::DatabaseHandler(ConfigHandler *cfgArg) :cfg(cfgArg) {
	// TODO Auto-generated constructor stub
	db = new QSqlDatabase();
	*db = QSqlDatabase::addDatabase("QPSQL");

	if (!db->isValid()) {
		qFatal("Database invalid: QPSQL");
	}
	db->setHostName(cfg->dbHost);
	db->setDatabaseName(cfg->dbName);
	db->setPort(cfg->dbPort.toInt());
	db->setUserName(cfg->dbUser);
	db->setPassword(cfg->dbPass);
	qDebug() << "Opening Database " + cfg->dbName + " on Host " + cfg->dbHost + ".";
	if (!db->open()) {
		qFatal("Could not open Database");
	} else {
		qDebug() << "Database opened.";
	}
}

DatabaseHandler::~DatabaseHandler() {
	// TODO Auto-generated destructor stub
	db->close();
	delete db;
}

QStringList DatabaseHandler::getSessionList() {
	qDebug() << "Getting session list from database.";
	QStringList sessionList;
	QString qstr = "SELECT distinct(session) FROM raw_census ORDER BY session";
	qDebug() << qstr;
	QSqlQuery query(qstr);
	while(query.next()) {
		sessionList.append(query.value(0).toString());
	}
	if (sessionList.empty()) {
		qFatal("Session list is empty.");
	}
	return sessionList;
}

QStringList DatabaseHandler::getBirdTypeList() {
	qDebug() << "Getting bird type list from database.";
	QStringList birdList;
	QString qstr = "SELECT tx_name_de FROM taxa_bird";
	qDebug() << qstr;
	QSqlQuery query("SELECT tx_name_de FROM taxa_bird");
	while(query.next()) {
		birdList.append(query.value(0).toString());
	}
	if (birdList.empty()) {
		qFatal("Bird type list is empty.");
	}
	return birdList;
}

QStringList DatabaseHandler::getMammalTypeList() {
	qDebug() << "Getting mammal type list from database.";
	QStringList birdList;
	QString qstr = "SELECT tx_name_de FROM taxa_mammal";
	qDebug() << qstr;
	QSqlQuery query(qstr);
	while(query.next()) {
		birdList.append(query.value(0).toString());
	}
	if (birdList.empty()) {
		qFatal("Bird type list is empty.");
	}
	return birdList;
}

QStringList DatabaseHandler::getUserList(QString objId) {
	qDebug() << "Getting user list from database.";
	QStringList userList;
//	userList.append(cfg->user());
	QString qstr = "SELECT usr FROM census WHERE rcns_id=" + objId + " ORDER BY censor, fcns_id";
	qDebug() << qstr;
	QSqlQuery query(qstr);
	QString user;
	if (query.size() == -1) return userList;
	while(query.next()) {
		user = query.value(0).toString();
		if (!userList.contains(user))
			userList.append(user);
	}
	return userList;
}

QString DatabaseHandler::getProjectPath(QString session) {
	qDebug() << "Getting session path list from database.";
	QSqlQuery query("SELECT path FROM projects WHERE project_id='" + session + "'");
	if (query.size() == -1) return "/net";
	while(query.next()) {
		return query.value(0).toString();
	}
	return "/net";
}

QSqlQuery * DatabaseHandler::getObjectResult(QString session, QString user, QString filter, QString order) {
	// get object data for population of object list
	qDebug() << "Gettings object data for session: " + session;
	QString otbl = "SELECT rc.rcns_id, rc.tp as pre_tp, rc.cam, rc.img, max(c.censor) as mc,"
			" count(*) as cnt, string_agg(c.tp, ', ' ORDER BY c.fcns_id) as otp FROM raw_census as rc LEFT JOIN census"
			" as c ON rc.rcns_id=c.rcns_id WHERE (censor>0 OR censor IS NULL) AND rc.session='" + session +
			"' GROUP BY rc.rcns_id, rc.tp, rc.cam, rc.img";
	QString utbl = "SELECT rcns_id, tp, censor FROM census where usr='"+user+"'";
	QString qstr = "SELECT * FROM (" + otbl + ") as ot LEFT JOIN (" + utbl + ") as ut ON " +
			"ot.rcns_id=ut.rcns_id " + filter + order;
	qDebug() << qstr;
	QSqlQuery * query = new QSqlQuery(qstr);
	return query;
}

double * DatabaseHandler::getObjectPosition(QString objId) {
	double *pos = new double[2];
	QString qstr = "SELECT ux, uy, px, py FROM raw_census WHERE rcns_id=" + objId;
	qDebug() << qstr;
	QSqlQuery * query = new QSqlQuery(qstr);
	if (query->next()) {
		pos[0] = query->value(0).toDouble();
		pos[1] = query->value(1).toDouble();
	} else {
		qDebug() << "Couldn't get position data for object id " << objId;
	}
	delete query;
	return pos;
}

census * DatabaseHandler::getRawObjectData(QString objId, QString usr) {
	qDebug() << "Getting raw object data for object ID: " << objId;
	QString qstr = "SELECT rcns_id, session, epsg, cam, img, tp, px, py, ux, uy, lx, ly FROM raw_census WHERE rcns_id=" + objId;
	qDebug() << qstr;
	QSqlQuery * query = new QSqlQuery(qstr);
	census *obj = new census;
	if(!query->next()) {
		qDebug() << "No data found for object ID: " << objId;
		return obj;
	}
	obj->id = query->value(0).toInt();
	obj->session = query->value(1).toString();
	obj->epsg = query->value(2).toString();
	obj->camera = query->value(3).toInt();
	obj->image = query->value(4).toString();
	obj->px = query->value(6).toInt();
	obj->py = query->value(7).toInt();
	obj->ux = query->value(8).toDouble();
	obj->uy = query->value(9).toDouble();
	obj->lx = query->value(10).toDouble();
	obj->ly = query->value(11).toDouble();
	obj->usr = usr;
	delete query;
	qDebug() << "Getting object specific data for ID: " << objId;
	qstr = "SELECT tp, name, qual, beh, age, gen, dir, rem, censor, imgqual FROM census WHERE rcns_id=" + objId
			+ " AND usr='" + usr + "'";
	qDebug() << qstr;
	// if there is already an entry in census db-table,
	// initialize census structure with these values
	query = new QSqlQuery(qstr);
	if (query->next()) {
		obj->type = query->value(0).toString();
		obj->name = query->value(1).toString();
		obj->quality = query->value(2).toInt();
		obj->behavior = query->value(3).toString();
		obj->age = query->value(4).toString();
		obj->gender = query->value(5).toString();
		obj->direction = query->value(6).toInt();
		obj->remarks = query->value(7).toString();
		obj->censor = query->value(8).toInt();
		obj->imageQuality = query->value(9).toInt();
	}
	delete query;
//	qstr = "SELECT max(censor) FROM census WHERE rcns_id=" + objId;
//	query = new QSqlQuery(qstr);
//	if (query->next()) {
//		obj->censor = query->value(0).toInt();
//	}
//	delete query;
	return obj;
}

bool DatabaseHandler::writeCensus(census * obj) {
	qDebug() << "Writing object data to database.";
	QSqlTableModel table;
	table.setTable("census");
	table.setFilter("rcns_id=" + QString::number(obj->id) + " AND usr='" + obj->usr + "'");
	table.select();
	// get record structure from db
	QSqlRecord record(table.record());
	// initialize record with census-structure values
	setRecordTable(&record, obj);

	// insert or update records in db
	if (table.rowCount() == 0) { //INSERT
		qDebug() << "Insert!";
		// remove first entry of record
		// auto increment of id is handled by postgres
		record.remove(0);
		return table.insertRecord(-1,record);
	} else { //UPDATE
		qDebug() << "Update!";
		record.setValue("fcns_id",table.record(0).value(0).toInt());
		bool check = true;
		check = check && table.setRecord(0, record);
		check = check && table.submitAll();
		return check;
	}
	return true;
}

/*
 * translate values from census struct into a QSqlRecord which can be written
 * directly to DB
 */
void DatabaseHandler::setRecordTable(QSqlRecord * record, census * obj) {
	record->setValue("rcns_id",obj->id);
	record->setValue("age",obj->age);
	record->setValue("beh",obj->behavior);
	record->setValue("gen",obj->gender);
	record->setValue("name",obj->name);
	record->setValue("tp",obj->type);
	record->setValue("qual",obj->quality);
	record->setValue("rem",obj->remarks.replace('"', " "));
	record->setValue("usr",obj->usr);
	if (obj->direction >= 0) record->setValue("dir", obj->direction);
	record->setValue("censor", obj->censor);
	record->setValue("imgqual", obj->imageQuality);
}

/*
 * get map of viewed objects in census table for a specific user
 */
QMap<int, int> DatabaseHandler::getObjectDone(QString usr, QString session) {
	qDebug() << "Getting viewed object list from database for user" << usr << " and session " << session;
	QMap <int, int> objMap;
	QString keys = "census.rcns_id, census.censor";
	QString qstr = "SELECT " + keys + " FROM census JOIN raw_census on " +
			"census.rcns_id=raw_census.rcns_id WHERE raw_census.session='" + session +
			"' AND census.usr='" + usr + "'";
	QSqlQuery query(qstr);
	if (query.size() == -1) return objMap;
	while(query.next()) {
		objMap[query.value(0).toInt()] = query.value(1).toInt();
	}
	return objMap;
}

/*
 * get map of completed objects for all users
 */
QMap<int, int> DatabaseHandler::getObjectFinal(QString session) {
	qDebug() << "Getting viewed object list from database.";
	QMap <int, int> objMap;
	QSqlQuery query("SELECT census.rcns_id, census.censor FROM census JOIN raw_census ON census.rcns_id=raw_census.rcns_id WHERE raw_census.session='" + session + "' AND census.censor=2");
	if (query.size() == -1) return objMap;
	while(query.next()) {
		objMap[query.value(0).toInt()] = query.value(1).toInt();
	}
	return objMap;
}

census * DatabaseHandler::getCensusData(QString objId) {
	qDebug() << "Getting object specific query for ID: " << objId;
	QString qstr = "SELECT tp, name, qual, beh, age, gen, dir, rem, censor, imgqual FROM census WHERE rcns_id=" + objId +
			" AND usr!='" + cfg->user() + "' AND censor=1";
	qDebug() << qstr;
	// if there is already an entry in census db-table,
	// initialize census structure with these values
	QSqlQuery * query = new QSqlQuery(qstr);
	if (query->size() != 1) {
		delete query;
		return 0;
	}
	census * obj = new census;
	if (query->next()) {
		obj->type = query->value(0).toString();
		obj->name = query->value(1).toString();
		obj->quality = query->value(2).toInt();
		obj->behavior = query->value(3).toString();
		obj->age = query->value(4).toString();
		obj->gender = query->value(5).toString();
		obj->direction = query->value(6).toInt();
		obj->remarks = query->value(7).toString();
		obj->censor = query->value(8).toInt();
		obj->imageQuality = query->value(9).toInt();
	}
	delete query;
	return obj;
}

int DatabaseHandler::getMaxCensor(QString objId) {
	QString qstr = "SELECT max(censor) FROM census WHERE rcns_id=" + objId;
//	qDebug() << qstr;
	QSqlQuery * query = new QSqlQuery(qstr);
	if (query->next()) {
			return query->value(0).toInt();
	}
	return 0;
}

int DatabaseHandler::getMaxCensor(QString objId, QString usr) {
	QString qstr = "SELECT max(censor) FROM census WHERE rcns_id=" + objId + " AND usr!='" + usr +"'";
//	qDebug() << qstr;
	QSqlQuery * query = new QSqlQuery(qstr);
	if (query->next()) {
			return query->value(0).toInt();
	}
	return 0;
}

int DatabaseHandler::getCensorCount(QString objId, QString censor) {
	QString qstr = "SELECT count(usr) FROM census WHERE rcns_id=" + objId + " AND censor=" + censor;
//	qDebug() << qstr;
	QSqlQuery * query = new QSqlQuery(qstr);
	if (query->next()) {
			return query->value(0).toInt();
	}
	return 0;
}

int DatabaseHandler::getCensorCount(QString objId, QString censor, QString usr) {
	QString qstr = "SELECT count(usr) FROM census WHERE rcns_id=" + objId + " AND censor=" + censor
			+ " AND usr!='" + usr + "'";
//	qDebug() << qstr;
	QSqlQuery * query = new QSqlQuery(qstr);
	if (query->next()) {
			return query->value(0).toInt();
	}
	return 0;
}

QMap<int, QString> DatabaseHandler::getUserCensus(QString usr, QString session) {
	qDebug() << "Getting map of user census." << endl;
	QMap<int, QString> cmap;
	QString keys = "c.rcns_id, c.tp";
	QString qstr = "SELECT "+ keys +" FROM census as c JOIN raw_census as rc " +
			"on c.rcns_id=rc.rcns_id WHERE c.usr='" + usr + "' AND rc.session='" + session + "'";
	qDebug() << qstr;
	QSqlQuery query(qstr);
	while(query.next()) {
		cmap[query.value(0).toInt()] = query.value(1).toString();
	}
	return cmap;
}

QMap<int, QString> DatabaseHandler::getFinalCensus(QString session) {
	qDebug() << "Getting map of user census." << endl;
	QMap<int, QString> cmap;
	QString keys = "c.rcns_id, c.tp";
	QString qstr = "SELECT "+ keys +" FROM census as c JOIN raw_census as rc " +
			"on c.rcns_id=rc.rcns_id WHERE rc.session='" + session + "' AND c.censor=2";
	qDebug() << qstr;
	QSqlQuery query(qstr);
	while(query.next()) {
		cmap[query.value(0).toInt()] = query.value(1).toString();
	}
	return cmap;
}

QStringList DatabaseHandler::getTypeList() {
	qDebug() << "Getting type list from DB";
	QStringList list;
	QString qstr = "SELECT DISTINCT tp FROM raw_census";
	qDebug() << qstr;
	QSqlQuery query(qstr);
	while(query.next()) {
		list.append(query.value(0).toString());
	}
	return list;
}

QStringList DatabaseHandler::getCensusList() {
	qDebug() << "Getting type list from DB";
	QStringList list;
	QString qstr = "SELECT DISTINCT tp FROM census";
	qDebug() << qstr;
	QSqlQuery query(qstr);
	while(query.next()) {
		list.append(query.value(0).toString());
	}
	return list;
}

void DatabaseHandler::deleteCensusData(QString objId, QString usr) {
	qDebug() << "Delete data from user: " + usr + " ID: " + objId + "." << endl;
	QString qstr = "DELETE FROM census WHERE rcns_id=" + objId + " AND usr='" + usr + "'";
	qDebug() << qstr;
	QSqlQuery query(qstr);
	query.exec();
}

bool DatabaseHandler::getSessionActive(QString session) {
	QString qstr = "SELECT active FROM projects WHERE project_id='" + session + "'";
	QSqlQuery query(qstr);
	if(query.next()) {
		if (query.value(0).toInt() > 0)
			return true;
		else
			return false;
	} else {
		return false;
	}
}

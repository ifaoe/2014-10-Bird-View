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
	QSqlQuery query("SELECT distinct(session) FROM raw_census");
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
	QSqlQuery query("SELECT tx_name_de FROM taxa");
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
	userList.append(QString::fromStdString(getenv("USER")));
	QSqlQuery query("SELECT usr FROM census WHERE rcns_id=" + objId);
	QString user;
	if (query.size() == -1) return userList;
	while(query.next()) {
		user = query.value(0).toString();
		if (!userList.contains(user))
			userList.append(user);
	}
	qDebug() << "Done";
	return userList;
}

QSqlQuery * DatabaseHandler::getObjectResult(QString session) {
	// get object data for population of object list
	QString qstr = "SELECT raw_census.rcns_id, raw_census.tp, raw_census.cam, raw_census.img, census.usr, census.censor, census.tp FROM raw_census"
			" LEFT JOIN census on raw_census.rcns_id=census.rcns_id WHERE raw_census.session='"
			+ session + "' ORDER BY cam,img,rcns_id,censor desc";
	QSqlQuery * query = new QSqlQuery(qstr);
	return query;
}

double * DatabaseHandler::getObjectPosition(QString objId) {
	double *pos = new double[2];
	QString qstr = "SELECT ux, uy, px, py FROM raw_census WHERE rcns_id=" + objId;
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
	qDebug() << "Done.";
	qDebug() << "Getting object specific data for ID: " << objId;
	qstr = "SELECT tp, name, qual, beh, age, gen, dir, rem, censor, imgqual FROM census WHERE rcns_id=" + objId
			+ " AND usr='" + usr + "'";
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
	qDebug() << "Done";
	return obj;
}

void DatabaseHandler::writeCensus(census * obj) {
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
		table.insertRecord(-1,record);
	} else { //UPDATE
		qDebug() << "Update!";
		record.setValue("fcns_id",table.record(0).value(0).toInt());
		table.setRecord(0, record);
		table.submitAll();
	}
}

void DatabaseHandler::setRecordTable(QSqlRecord * record, census * obj) {
	// write values from census structure into db record
	record->setValue("age",obj->age);
	record->setValue("beh",obj->behavior);
	record->setValue("cam",obj->camera);
	record->setValue("epsg",obj->epsg);
	record->setValue("gen",obj->gender);
	record->setValue("rcns_id",obj->id);
	record->setValue("img",obj->image);
	record->setValue("lx",obj->lx);
	record->setValue("ly",obj->ly);
	record->setValue("ux",obj->ux);
	record->setValue("uy",obj->uy);
	record->setValue("px",obj->px);
	record->setValue("py",obj->py);
	record->setValue("name",obj->name);
	record->setValue("tp",obj->type);
	record->setValue("qual",obj->quality);
	record->setValue("rem",obj->remarks);
	record->setValue("session",obj->session);
	record->setValue("usr",obj->usr);
	if (obj->direction >= 0) record->setValue("dir", obj->direction);
	record->setValue("censor", obj->censor);
	record->setValue("imgqual", obj->imageQuality);
}

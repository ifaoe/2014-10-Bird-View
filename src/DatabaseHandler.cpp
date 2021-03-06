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
#include <QTableView>
#include <QHeaderView>

DatabaseHandler::DatabaseHandler(ConfigHandler *cfgArg) :config(cfgArg) {
    // TODO Auto-generated constructor stub
    db = new QSqlDatabase();
    *db = QSqlDatabase::addDatabase("QPSQL");

    if (!db->isValid()) {
        qFatal("Database invalid: QPSQL");
    }
}

DatabaseHandler::~DatabaseHandler() {
    // TODO Auto-generated destructor stub
    db->close();
    delete db;
}

bool DatabaseHandler::OpenDatabase() {
	if (config->getPreferredDatabase().isEmpty())
		return false;

	if (db->isOpen()) {
		db->close();
	}

	DatabaseInfo info = config->getDatabaseInfo(config->getPreferredDatabase());
	if (info.id.isEmpty())
		return false;
    db->setHostName(info.host);
    db->setDatabaseName(info.name);
    db->setPort(info.port);
    db->setUserName(info.user);
    db->setPassword(info.password);
    qDebug() << "Opening Database " + db->databaseName()+ " on Host " + db->hostName() + ".";
    if (!db->open()) {
        qFatal("Could not open Database");
    }
    return true;
}

QStringList DatabaseHandler::getSessionList() {
    qDebug() << "Getting session list from database.";
    QStringList sessionList;
    QString qstr = "SELECT project_id FROM projects WHERE active>0 ORDER BY project_id";
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

bool DatabaseHandler::getSpeciesList(QString type, QComboBox * cmb_box) {
	qDebug() << "Populating species list for " << type;
    QString qstr = "SELECT name_de, name_lat, euring_id, length FROM taxa LEFT JOIN "
    		"(SELECT id_code, to_char(avg(length), 'FM99.99') as length FROM census WHERE tp='%1' GROUP BY id_code) as lt ON taxa.euring_id = lt.id_code "
    		"WHERE type='%1' ORDER BY seaflag DESC, name_de";
	QSqlQueryModel * model = new QSqlQueryModel;
	model->setQuery(qstr.arg(type));
	qDebug() << qstr.arg(type);
	model->setHeaderData(0, Qt::Horizontal, "Deutscher Name");
	model->setHeaderData(1, Qt::Horizontal, "Wissenschaftlicher Name");
	model->setHeaderData(2, Qt::Horizontal, "EURING Code");
	model->setHeaderData(3, Qt::Horizontal, QString::fromUtf8("Länge"));
	cmb_box->setModel(model);
	QTableView * view = new QTableView;
	cmb_box->setView(view);
	view->verticalHeader()->hide();
	view->hideColumn(2);
	view->resizeColumnsToContents();
	view->setSelectionMode(QAbstractItemView::SingleSelection);
	view->setSelectionBehavior(QAbstractItemView::SelectRows);
	view->setMinimumWidth(view->horizontalHeader()->length());

	return true;
}

bool DatabaseHandler::GetAnthroObjectList(QComboBox * combo_box) {
    qDebug() << "Gettings list of anthropogenic objects from database.";
    combo_box->addItem("",-1);
    QString qstr = "SELECT code, description, remarks FROM stuk4_codes "
    		"WHERE type='ANTHRO' AND code!='0' ORDER BY description";
    qDebug() << qstr;
	QSqlQuery query(qstr);
	while (query.next()) {
		combo_box->addItem(query.value(1).toString(), query.value(0));
	}
	return true;
}

QStringList DatabaseHandler::getUserList(QString objId) {
    qDebug() << "Getting user list from database.";
    QStringList userList;
//    userList.append(cfg->user());
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
    QString otbl = QString("SELECT rc.rcns_id, rc.tp as pre_tp, rc.cam, rc.img, max(c.censor) as mc,"
            " count(*) as cnt, string_agg(c.tp, ', ' ORDER BY c.fcns_id) as otp,"
            " max(case when c.usr='%1' then 1 else 0 end) FROM raw_census as rc LEFT JOIN census"
            " as c ON rc.rcns_id=c.rcns_id WHERE (censor>0 OR censor IS NULL) AND rc.session='%2'"
            " GROUP BY rc.rcns_id, rc.tp, rc.cam, rc.img").arg(user, session);
    QString utbl = QString("SELECT rcns_id, tp, censor FROM census where usr='%1'").arg(user);
    QString qstr = QString("SELECT * FROM (%1) as ot LEFT JOIN (%2) as ut ON ot.rcns_id=ut.rcns_id WHERE %3 %4")
    		.arg(otbl).arg(utbl).arg(filter).arg(order);
    qDebug() << qstr;
    QSqlQuery * query = new QSqlQuery(qstr);
    return query;
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
    obj->camera = query->value(3).toString();
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
    qstr =    "SELECT tp, name, confidence, beh, age, gen, dir, rem, censor, imgqual, length, width"
            ", stuk4_beh, stuk4_ass, group_objects, family_group,id_code,age_year,plumage"
            " FROM census WHERE rcns_id=" + objId + " AND usr='" + usr + "'";
    qDebug() << qstr;
    // if there is already an entry in census db-table,
    // initialize census structure with these values
    query = new QSqlQuery(qstr);
    if (query->next()) {
        obj->type = query->value(0).toString();
        obj->name = query->value(1).toString();
        obj->confidence = query->value(2).toInt();
        obj->behavior = query->value(3).toString();
        obj->age = query->value(4).toString();
        obj->gender = query->value(5).toString();
        if (!query->value(6).isNull())
        	obj->direction = query->value(6).toInt();
        obj->remarks = query->value(7).toString();
        obj->censor = query->value(8).toInt();
        obj->imageQuality = query->value(9).toInt();
        obj->length = query->value(10).toDouble();
        obj->span = query->value(11).toDouble();
        obj->stuk4_beh = query->value(12).toString().remove(QRegExp("[{}]")).split(",");
        obj->stuk4_ass = query->value(13).toString().remove(QRegExp("[{}]")).split(",");
        obj->group = query->value(14).toString().remove(QRegExp("[{}]")).split(",");
        obj->family = query->value(15).toString().remove(QRegExp("[{}]")).split(",");
        obj->code = query->value(16).toString();
        if (!query->value(17).isNull())
        	obj->age_year = query->value(17).toInt();
        if (!query->value(18).toString().isEmpty())
        	obj->plumage = query->value(18).toString();
    }
    delete query;
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
        bool done = table.insertRecord(-1,record);
        qDebug() << table.lastError();
        return done;
    } else { //UPDATE
        qDebug() << "Update!";
        record.setValue("fcns_id",table.record(0).value(0).toInt());
        bool check = true;
        check = check && table.setRecord(0, record);
        check = check && table.submitAll();
        qDebug() << table.lastError();
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

    if (obj->age.isNull()) record->setNull("age");
    else record->setValue("age",obj->age);

    if (obj->age_year>0) record->setValue("age_year",obj->age_year);
    else record->setNull("age_years");

    if (obj->behavior.isEmpty()) record->setNull("beh");
    else record->setValue("beh",obj->behavior);

    if (obj->gender.isEmpty()) record->setNull("gen");
    else record->setValue("gen",obj->gender);

    if (obj->name.isEmpty()) record->setNull("name");
    else record->setValue("name",obj->name);

    record->setValue("id_code", obj->code);
    record->setValue("tp",obj->type);
    record->setValue("confidence",obj->confidence);

    if (obj->remarks.isEmpty()) record->setNull("rem");
    else record->setValue("rem",obj->remarks.replace('"', " "));

    record->setValue("usr",obj->usr);

    if (obj->direction >= 0) record->setValue("dir", obj->direction);
    else record->setNull("dir");

    record->setValue("censor", obj->censor);

    record->setValue("imgqual", obj->imageQuality);

    if (obj->length >0) record->setValue("length", obj->length);
    else record->setNull("length");

    if (obj->span > 0) record->setValue("width", obj->span);
    else record->setNull("width");

    if (obj->plumage.isEmpty()) record->setNull("plumage");
    else record->setValue("plumage",obj->plumage);

    record->setValue("stuk4_beh", "{"+obj->stuk4_beh.join(",")+"}");
    record->setValue("stuk4_ass", "{"+obj->stuk4_ass.join(",")+"}");
    record->setValue("group_objects", "{"+obj->group.join(",")+"}");
    record->setValue("family_group", "{"+obj->family.join(",")+"}");
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
    QString qstr = "SELECT tp, name, confidence, beh, age, gen, dir, rem, censor, imgqual FROM census WHERE rcns_id=" + objId +
            " AND usr!='" + config->user() + "' AND censor=1";
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
        obj->confidence = query->value(2).toInt();
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
//    qDebug() << qstr;
    QSqlQuery * query = new QSqlQuery(qstr);
    if (query->next()) {
            return query->value(0).toInt();
    }
    return 0;
}

int DatabaseHandler::getMaxCensor(QString objId, QString usr) {
    QString qstr = "SELECT max(censor) FROM census WHERE rcns_id=" + objId + " AND usr!='" + usr +"'";
//    qDebug() << qstr;
    QSqlQuery * query = new QSqlQuery(qstr);
    if (query->next()) {
            return query->value(0).toInt();
    }
    return 0;
}

int DatabaseHandler::getCensorCount(QString objId, QString censor) {
    QString qstr = "SELECT count(usr) FROM census WHERE rcns_id=" + objId + " AND censor=" + censor;
//    qDebug() << qstr;
    QSqlQuery * query = new QSqlQuery(qstr);
    if (query->next()) {
            return query->value(0).toInt();
    }
    return 0;
}

int DatabaseHandler::getCensorCount(QString objId, QString censor, QString usr) {
    QString qstr = "SELECT count(usr) FROM census WHERE rcns_id=" + objId + " AND censor=" + censor
            + " AND usr!='" + usr + "'";
//    qDebug() << qstr;
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

QStringList DatabaseHandler::getRawTypeList() {
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

QStringList DatabaseHandler::getCamList() {
    qDebug() << "Getting camera list from DB";
    QStringList list;
    QString qstr = "SELECT DISTINCT cam FROM raw_census";
    qDebug() << qstr;
    QSqlQuery query(qstr);
    while(query.next()) {
        list.append(query.value(0).toString());
    }
    return list;
}


void DatabaseHandler::GetBirdAgeClasses(QComboBox * cmb_box) {
	cmb_box->clear();
	cmb_box->addItem("",-1);
	qDebug() << "Getting bird age classes from database";
	QStringList list;
	QString qstr = "SELECT code, description FROM stuk4_codes WHERE type='AGE_YEARS' ORDER BY code";
	qDebug() << qstr;
	QSqlQuery query(qstr);
	while (query.next()) {
		cmb_box->addItem(query.value(1).toString(), query.value(0));
	}
}

void DatabaseHandler::GetMiscObjects(QComboBox * cmb_box) {
	cmb_box->clear();
	cmb_box->addItem("","0");
	qDebug() << "Getting miscellanous objects from database";
	QStringList list;
	QString qstr = "SELECT code, description FROM stuk4_codes WHERE type='MISC' AND code!='0' ORDER BY description";
	qDebug() << qstr;
	QSqlQuery query(qstr);
	while (query.next()) {
		cmb_box->addItem(query.value(1).toString(), query.value(0));
	}
}

void DatabaseHandler::GetBirdPlumageClasses(QComboBox * cmb_box) {
	cmb_box->clear();
	cmb_box->addItem("",QVariant());
	qDebug() << "Getting bird age classes from database";
	QStringList list;
	QString qstr = "SELECT code, description FROM stuk4_codes WHERE type='PLUMAGE' ORDER BY code";
	qDebug() << qstr;
	QSqlQuery query(qstr);
	while (query.next()) {
		cmb_box->addItem(query.value(1).toString(), query.value(0));
	}
}

void DatabaseHandler::deleteCensusData(QString objId, QString usr) {
    qDebug() << "Delete data from user: " + usr + " ID: " + objId + "." << endl;
    QString qstr = "DELETE FROM census WHERE rcns_id=" + objId + " AND usr='" + usr + "'";
    qDebug() << qstr;
    QSqlQuery query(qstr);
    query.exec();
}

bool DatabaseHandler::getSessionActive(const QString & session) {
    QString qstr = "SELECT active FROM projects WHERE project_id='" + session + "'";
    QSqlQuery query(qstr);
    if(query.next()) {
        if ((query.value(0).toInt() > 0) && (query.value(0).toInt() < 3))
            return true;
        else
            return false;
    } else {
        return false;
    }
}

QSqlQueryModel * DatabaseHandler::getStuk4Behaviour() {
    QSqlQueryModel * model = new QSqlQueryModel;
    model->setQuery("SELECT code, category, description FROM stuk4_codes "
    		"where type='BEH' AND code!='0' ORDER BY cast(code as integer)");
    model->setHeaderData(0, Qt::Horizontal, "Code");
    model->setHeaderData(1, Qt::Horizontal, "Kategorie");
    model->setHeaderData(2, Qt::Horizontal, "Beschreibung");
    return model;
}

QSqlQueryModel * DatabaseHandler::getStuk4Associations() {
    QSqlQueryModel * model = new QSqlQueryModel;
    model->setQuery("SELECT code, category, description FROM stuk4_codes "
    		"where type='ASS' AND code!='0' ORDER BY cast(code as integer)");
    model->setHeaderData(0, Qt::Horizontal, "Code");
    model->setHeaderData(1, Qt::Horizontal, "Kategorie");
    model->setHeaderData(2, Qt::Horizontal, "Beschreibung");
    return model;
 }

QSqlQueryModel * DatabaseHandler::getCloseObjects(census * obj) {
    QSqlQueryModel * model = new QSqlQueryModel;
    QString qstr = "SELECT sync_id FROM sync_utm32 WHERE cam" + obj->camera + "_id='"
            + obj->image + "'";
    QSqlQuery query(qstr);
    int sync_id;
    if (query.next())
        sync_id = query.value(0).toInt();
    else {
        qDebug() << qstr;
        return model;
    }



    query.clear();

    QStringList sidList = QStringList() << QString::number(sync_id-1) << QString::number(sync_id)
                                        << QString::number(sync_id+1);
    qstr = "SELECT cam, img FROM image_properties WHERE sync_id IN (" +sidList.join(",")+ ")";
    query.exec(qstr);

    QStringList condList;
    while(query.next()) {
        condList.append("(cam='" + query.value(0).toString()
                    + "' AND img='" +query.value(1).toString()+ "')");
    }
    /*
     * TODO: Add distance comparison
     */
    qstr = "SELECT rcns_id, cam, img, ux, uy, tp, "
            "ST_Distance( (SELECT ST_SetSRID(ST_Point(ux,uy),32632) FROM raw_census "
            "where rcns_id=" + QString::number(obj->id) + ") "
            ", ST_SetSRID(ST_Point(ux,uy),32632) ) as dist FROM raw_census "
            "WHERE (" + condList.join(" OR ") + ") AND rcns_id!=" + QString::number(obj->id) +
            " AND session='" + obj->session + "' ORDER BY dist";
    qDebug() << qstr;
    model->setQuery(qstr);
    model->setHeaderData(0, Qt::Horizontal, "Object Id");
    model->setHeaderData(1, Qt::Horizontal, "Kamera");
    model->setHeaderData(2, Qt::Horizontal, "Bildnummer");
    model->setHeaderData(3, Qt::Horizontal, "UTM X");
    model->setHeaderData(4, Qt::Horizontal, "UTM Y");
    model->setHeaderData(5, Qt::Horizontal, "Vorsortierung");
    model->setHeaderData(6, Qt::Horizontal, "Entfernung in m");
    return model;
}

QSqlQueryModel * DatabaseHandler::getImageObjects(census * obj) {
    QSqlQueryModel * model = new QSqlQueryModel;
    QString qstr = "SELECT rcns_id, tp, ux, uy, max(censor), count(*) FROM view_census WHERE cam='" +
            obj->camera + "' AND img='" + obj->image + "' AND session='"
            + obj->session + "' AND (censor>0 OR censor IS NULL) GROUP BY rcns_id, tp, ux, uy ORDER BY rcns_id";
    qDebug() << qstr;
    model->setQuery(qstr);
    model->setHeaderData(0, Qt::Horizontal, "Objekt Id");
    model->setHeaderData(1, Qt::Horizontal, "Typ");
    model->setHeaderData(2, Qt::Horizontal, "UTM X");
    model->setHeaderData(3, Qt::Horizontal, "UTM Y");
    model->setHeaderData(4, Qt::Horizontal, "Highest censor");
    model->setHeaderData(5, Qt::Horizontal, "Censor count");
    return model;
}

int DatabaseHandler::getCensusCount(const QString & session, const QString & user,
		const QString & having_filter, const QString & where_filter) {
	QString query_string = "SELECT count(*) FROM "
			"( SELECT max(censor) as mc, count(censor) as cnt FROM view_census "
			"WHERE session='%1' AND (usr like '%2' OR usr IS NULL) AND %3"
			" GROUP BY rcns_id) as tmp where %4";
	QSqlQuery query(query_string.arg(session).arg(user).arg(where_filter).arg(having_filter));
	qDebug() << query_string.arg(session).arg(user).arg(where_filter).arg(having_filter);
	if (query.next())
		return query.value(0).toInt();
	return 0;
}

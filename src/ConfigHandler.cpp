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
#include <boost/program_options.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <iostream>
#include <cstring>
#include <QDir>

namespace po = boost::program_options;
namespace pt = boost::property_tree;
using namespace std;

ConfigHandler::ConfigHandler(int argc, char *argv[]) {
    // TODO Auto-generated constructor stub
    usr = QString::fromStdString(getenv("USER"));

    po::options_description desc("Options");
    desc.add_options()
            ("help,h", "Show this help message.")
            ("config,c", po::value<string>(), "Path to config-file.");
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << "\n";
        exit(0);
    }
    if (vm.count("config")) {
        string tmp = vm["config"].as<string>();
        cfgFile = new QFile(tmp.data());
    } else {
        cfgFile = new QFile("/usr/local/ifaoe/Settings/Daisi/birdview/main.cfg");
    }
    if (cfgFile->exists()) {
        qDebug() << "Using config-file: " << cfgFile->fileName();
    } else {
        qFatal( "Fatal: Config file %s does not exist or is not readable. Aborting.", cfgFile->fileName().toStdString().c_str() );
    }

    boost::property_tree::ini_parser::read_ini(cfgFile->fileName().toStdString(), cfg);
    databaseList = QString().fromStdString( cfg.get<std::string>("main.dblist")).split(",");
}

ConfigHandler::~ConfigHandler() {
    // TODO Auto-generated destructor stub
    delete cfgFile;
}

QString ConfigHandler::user() { return usr; }

void ConfigHandler::parseCfgFile(QString database, QString filedb) {
//    boost::property_tree::ini_parser::read_ini(cfgFile->fileName().toStdString(), cfg);
    dbHost = QString::fromStdString( cfg.get<std::string>(database.toStdString() + ".host") );
    dbFile = QString::fromStdString( cfg.get<std::string>(filedb.toStdString() + ".host") );
    dbName = QString::fromStdString( cfg.get<std::string>(database.toStdString() + ".name") );
    dbUser = QString::fromStdString( cfg.get<std::string>(database.toStdString() + ".user") );
    dbPass = QString::fromStdString( cfg.get<std::string>(database.toStdString() + ".pass") );
    dbPort = QString::fromStdString( cfg.get<std::string>(database.toStdString() + ".port") );
}

QStringList ConfigHandler::getDbList() { return databaseList; }

QString ConfigHandler::getSessionType() {return session_type; }

void ConfigHandler::setSessionType(QString type) { session_type = type; }

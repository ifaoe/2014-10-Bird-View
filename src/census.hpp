//    Copyright (C) 2014, 2015 Axel Wegener
//
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

#ifndef CENSUS_H_
#define CENSUS_H_


// C Struct which resembles the database record structure
struct census {
    int id = -1;
    QString session = "";
    QString image = "";
    QString camera = "";
    QString usr = "";
    QString epsg = "";
    int px = -1;
    int py = -1;
    double ux = -0.0;
    double uy = -0.0;
    double lx = -0.0;
    double ly = -0.0;
    QString type = "";
    QString name = "";
    QString code = "";
    int confidence = -1;
    QString behavior = "";
    QString gender = "";
    QString age = "";
    int age_years = -1;
    int direction = -1;
    QString remarks = "";
    int censor = -1;
    int imageQuality = 0;
    double span = -1;
    double length = -1;
    QStringList stuk4_beh;
    QStringList stuk4_ass;
    QStringList group;
    QStringList family;
};


#endif /* CENSUS_HPP_ */

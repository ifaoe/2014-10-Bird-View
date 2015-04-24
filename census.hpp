/*
 * census.hpp
 *
 *  Created on: Apr 24, 2015
 *      Author: awg
 */

#ifndef CENSUS_HPP_
#define CENSUS_HPP_


// C Struct which resembles the database record structure
struct census {
	int id = -1;
	QString session = "";
	QString image = "";
	int camera = -1;
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
	int quality = -1;
	QString behavior = "";
	QString gender = "";
	QString age = "";
	int direction = -1;
	QString remarks = "";
	int censor = -1;
	int imageQuality = 0;
	double span = -1;
	double length = -1;
};


#endif /* CENSUS_HPP_ */

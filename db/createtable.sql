CREATE TABLE census(
	fcns_id SERIAL not null primary key,
	rcns_id int,
	session text,
	cam	int,
	img text,
	usr text,
	px int,
	py int,
	epsg text,
	ux float8,
	uy float8,
	lx float8,
	ly float8,
	tp text,
	name text,
	qual text,
	beh text,
	age text,
	gen text,
	dir int,
	rem text,
	censor int,
	imgqual int
);

CREATE TABLE taxa_bird(
	tx_euring text not null primary key,
	tx_sea_flag text,
	tx_abv_de text,
	tx_abv_dk text,
	tx_name_lat text,
	tx_name_de text,
	tx_name_en text,
	tx_name_dk text
);

# ----------------------------------------------------------------
# Aufbau der exakt georeferenzierten Biddatenbank 2010 Wismarbucht
# ----------------------------------------------------------------
# (c) - 2013 IFAOE.DE; ALexander Weidauer; weidauer@ifaoe.de
# ----------------------------------------------------------------
VERSION    = 1.0
PRJ_PATH   = /home/census/hwi-2010/2013-09-rebuild
PRJ_BIN	   = $(PRJ_PATH)/bin
DB_NAME	   = 2013_bildflug_digital 
DB_ACCESS  = psql -q -d $(DB_NAME) 

all: 
	$(DB_ACCESS) -f $(PRJ_BIN)/create-incoming-v1.sql
	$(PRJ_BIN)/import-exif.pl
	$(PRJ_BIN)/import-gps.pl
	$(PRJ_BIN)/interpolate-gps.pl
	$(PRJ_BIN)/mk-image-log.pl
	$(PRJ_BIN)/mk-geo-ref.pl
	$(PRJ_BIN)/mk-trfm-ref.pl

# ----------------------------------------------------------------
# EOF
# ----------------------------------------------------------------

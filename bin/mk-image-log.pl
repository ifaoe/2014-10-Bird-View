#!/usr/bin/perl -w
# mk-image-log.pl ---
# ========================================================
# Verknuepfung der Bild und der Flugdaten und Berechnung der
# Referenzierungspunkte
# (c) 2013 IFAOE.DE,  A. Weidauer 
# ========================================================
use strict;
use lib "/home/census/tools/Perl";
use Config::BildflugWismar2010;
use Data::Dumper;
use Math::Round qw/round/;
use Math::Complex;
use Math::Trig;
use UserUtils;

# ===================================================================
# Standarteinstellungen 
# ===================================================================
my %PRJ_ENV=BildflugWismar2010::projectEnviron();
my $SESSION='2010-03-31';
my $DB_CLEAN=1;    
my $DB_NAME=$PRJ_ENV{"DBNAME"};
my $DATE=UserUtils::currentDate();
my $EPSG=32632;
my $DEBUG=0;
my $CAM="AIC.P45";
my $USE_TRACK_ANGLE = 1;


# ===================================================================
# SQL Zugriffe 
# ===================================================================

# SQL Query Eintraege der Session loeschen 
my $SQL_CLR_IMG="DELETE FROM incoming.image_log WHERE session =\'$SESSION\';";

# SQL Eintraege der Session aktualisieren 
my $SQL_UPD_IMG="INSERT INTO incoming.image_log  (\\\
 session, trc_num, utm_x, utm_y,utm_z,\\\
 utm_px, utm_py, utm_pz, dx, dy, dz, dr, dt, tm_sample,\\\
 tm_expose, img_file, trc_rad, trc_grd, trc_ngrd,\\\
 pln_rad, pln_grd, pln_ngrd)\\\
 SELECT\\\
 session, trc_num, utm_x, utm_y, utm_z,\\\
 utm_px, utm_py, utm_pz, dx, dy, dz, dr, dt, tm_sample,\\\
 tm_expose, img_file,\\\
 trc_rad, trc_grd, -trc_grd,\\\
 pln_rad, pln_grd, -pln_grd\\\
 FROM incoming.image_dpos_merge;
";

# Abfrage Kameradaten
my $SQL_QRY_CAM="select cam_type, cam_cols, cam_rows, cam_dx, cam_dy, cam_cs\\\
 from incoming.cam where cam_type=\'$CAM\';";

# Geopositionen Bild aktualisieren
my $SQL_UPD_IPOS="update incoming.image_log \\\
 set utm_ipos=st_setsrid(st_makepoint(utm_x,utm_y,utm_z),$EPSG)\\\
 where session = \'$SESSION\';";

# Geopositionen Bildvorgaenger aktualisieren
my $SQL_UPD_PPOS="update incoming.image_log \\\
 set utm_ppos=st_setsrid(st_makepoint(utm_px,utm_py,utm_pz),$EPSG)\\\
 where session = \'$SESSION\';";

# Geoposition Richtungsvektor aktualisieren
my $SQL_UPD_DIR="update incoming.image_log\\\
 set utm_dir=geo_mk_segment($EPSG,utm_x,utm_y,utm_z,utm_px,utm_py,utm_pz)
 where session = \'$SESSION\';";

my $SQL_QRY_IMG =();
if ( $USE_TRACK_ANGLE  ) {
 $SQL_QRY_IMG="SELECT
  ilg_pk, tm_expose, utm_x, utm_y, utm_z, dr/dt as v, trc_rad\\\
  from incoming.image_log where dt>0 and dr>0 and session=\'$SESSION\';";
} else {
 $SQL_QRY_IMG="SELECT
   ilg_pk, tm_expose, utm_x, utm_y, utm_z, dr/dt as v, pln_rad\\\
   from incoming.image_log where dt>0 and dr>0 and session=\'$SESSION\';";   
}

my $SQL_UPD_TRC1="update incoming.image_log set utm_ipos=null, utm_ppos=null, utm_dir=null, utm_roi=null, utm_ll=null, utm_ul=null, utm_ur=null, utm_lr=null where trc_num in (select trc_num from  (select trc_num, count(img_file) from incoming.image_log group by trc_num) as c where count=1) and session =\'$SESSION\'";

# ===================================================================
# Hauptabschnitt
# ===================================================================

# ------------------------------------------------------
# Greetings
# ------------------------------------------------------
print "Zusammenstellung von Bild- und Transektdaten:\n";
print "-----------------------\n\n";
print "Parameter:\n";
print "----------\n";
print "Nutzer:   ",$PRJ_ENV{"USER"},"\n";
print "Zeit:     ",$DATE,"\n";
print "SESSION:  ",$SESSION,"\n";
print "DB-CLEAN: ",$DB_CLEAN,"\n";
print "EPSG:     ",$EPSG,"\n";
print "KAMERA:   ",$CAM,"\n";

# ------------------------------------------------------
# Datenbank aufraeumen 
# ------------------------------------------------------
if ($DB_CLEAN) {
    print "\n..zuruecksetzen der Transekt-/ Bilddaten fuer Session \'$SESSION\' ";
    my $cmd="psql -d $DB_NAME -c \"$SQL_CLR_IMG\"";
    my @res =`$cmd`; print ".entfernt ", @res;

    print "\n..erneutes Einlesen der Basisdaten ";
    $cmd="psql -d $DB_NAME -c \"$SQL_UPD_IMG\"";
    @res =`$cmd`; print ".schreiben ", @res;
    
    print "\n..erneutes Einlesen der Position des Bildes ";
    $cmd="psql -d $DB_NAME -c \"$SQL_UPD_IPOS\"";
    @res =`$cmd`; print ".schreiben ", @res;

    print "\n..erneutes Einlesen der Ansteuerung des Bildes ";
    $cmd="psql -d $DB_NAME -c \"$SQL_UPD_PPOS\"";
    @res =`$cmd`; print ".schreiben ", @res;

    print "\n..erneutes Einlesen des Richtunsvektors ";
    $cmd="psql -d $DB_NAME -c \"$SQL_UPD_DIR\"";
    @res =`$cmd`; print ".schreiben ", @res;
    
}

print "\n..einlesen der Kameradatendaten ";
my $cmd="psql -A -t -d $DB_NAME -c \"$SQL_QRY_CAM\""; my @res =`$cmd`;
die "Es wurden keine Daten fuer Kamera $CAM gefunden!\n" if $#res < 0; 
my ($cam_id, $cam_cols, $cam_rows, $cam_dx, $cam_dy, $cam_cs) =
    split(/\|/,$res[0]); chomp($cam_cs);

print ".OK\n\n";
print "Kameradaten:\n";
print "------------\n";
print " Kenner der Kamera: $cam_id \n";
print " Spaltenaufloesung: $cam_dx [m]\n";
print " ZeilenaufloesungY: $cam_dy [m]\n";
print " Spalten Sensor X:  $cam_cols \n";
print " Zeilen Sensor Y:   $cam_rows \n";
print " Kamerakonstatnte:  $cam_cs \n";




print "\n..einlesen der Flug- und Bilddaten ";
$cmd="psql -A -t -d $DB_NAME -c \"$SQL_QRY_IMG\""; my @data =`$cmd`;
die "Es wurden keine Bild und Flugdaten gefunden!\n" if $#data <0;
my $numd = $#data+1; my $cntd =0;
print ".lesen $numd Eintraege gefunden!\n";

my @fields = qw(S:cam_type img_width img_height res_x res_y
                world_x world_y delta_v
                utm_ul utm_ll utm_ur utm_lr utm_roi);
             

print "\n..Georeferenz der Flug- und Bilddaten \n";
for my $line (@data) {
    $cntd++;
    chomp($line);   
    my ($pk, $tx, $x, $y, $z, $v, $phi) =  split(/\|/,$line);
    my @calc = &calcImageData($EPSG, $tx, $cam_id, $x, $y, $z, $v, $phi,
                   $cam_cols, $cam_rows, $cam_dx, $cam_dy, $cam_cs);
    
    my $qry =  UserUtils::listDataAsSqlSet("incoming.image_log",
                                           \@fields,\@calc," ilg_pk = $pk ");
    # print $qry,"\n";
    
    my $cmd="psql -A -t -d $DB_NAME -c \"$qry\""; my @res =`$cmd`;
    die "Fehler beim Berechnen igl_pk=$pk! \n" if ! $res[0] =~/UPDATE/;
    print "..Referenz $cntd von $numd \n";
          
}

print "\n..Konsistenzcheck der Flug- und Bilddaten \n";
my $cmd="psql -A -t -d $DB_NAME -c \"$SQL_UPD_TRC1\""; my @res =`$cmd`;
print "..Loesche Transekte mit nur einem Bild  ", @res;

# ===================================================================
# Serviceroutinen
# ===================================================================

# -------------------------------------------------------------------
# Definition eines Vierecks
# srid - EPSG, x1,y1... Koordinaten
# -------------------------------------------------------------------
sub mkGeoRect {
    my ($srid, $x1, $y1, $x2, $y2, $x3, $y3, $x4, $y4) =@_;
    my $txt = "SRID=$srid ;POLYGON((".
        "$x1 $y1, $x3 $y3, $x2 $y2, $x4 $y4, $x1 $y1))";
    return " GeomFromEWKT(\'$txt\') ";
}

# -------------------------------------------------------------------
# Definition eines Punktes
# srid - EPSG, x, y - Koordinaten
# -------------------------------------------------------------------
sub mkGeoPoint {
    my ($srid, $x, $y) =@_;
    my $txt = "SRID=$srid ;POINT($x $y)";
    return " GeomFromEWKT(\'$txt\') ";
}

# -------------------------------------------------------------------
# Relevante Bilddaten berechnen
# srid   - EPSG
# cam_id - Kameratyp
# tx     - Belichtungszeit
# x,y,z  - Position
# tphi   - Kurs der Plattform
# $ncol  - Anzahl der Bildspalten
# $nrow  - Anzahl der Bildzeilen
# $dx    - Groesse Pixelelement in x
# $dy    - Groesse Pixelelement in y
# $cc    - Kamerakonstante 
# -------------------------------------------------------------------
sub calcImageData {
    my ($srid, $tx, $cam_id, $x, $y, $z, $v, $phi, $ncol, $nrow, $dx, $dy, $cc) = @_;
    my $phi90 = $phi + pi/2;      # der um 90 Grad gedehte Kurswinkel
    my $wx = $z * $dx / $cc;      # Zellgroesse ueber Grund in Flugrichtung [m]
    my $wy = $z * $dy / $cc;      # Zellgroesse ueber Grund querab [m]
    my $sx = $wx * $ncol;         # Bildgrroesse ueber Grund in Flugrichtung [m]
    my $sy = $wy * $nrow;         # Bildgrroesse ueber Grund querab [m]
    my $vz = $v * $tx;            # Verzeichnung 
    my $px = cos($phi)*$sy/2;     # Offset querab X Koord [m]
    my $py = sin($phi)*$sy/2;     # Offset querab Y Koord [m]
    my $p90x = cos($phi90)*$sx/2; # Offset in Flugrichtung X Koord [m]
    my $p90y = sin($phi90)*$sx/2; # Offset in Flugrichtung Y Koord [m]
    my $x1 = $x+$px; # Position querab X Koord. Backboard [m]
    my $y1 = $y+$py; # Position querab Y Koord. Backboard [m]
    my $x2 = $x-$px; # Position querab X Koord. Steuerboard [m]
    my $y2 = $y-$py; # Position querab Y Koord. Steuerboard [m]
    my $x3 = $x+$p90x; # Position in Flugrichtung X Koord. voraus [m]
    my $y3 = $y+$p90y; # Position in Flugrichtung Y Koord. voraus [m]
    my $x4 = $x-$p90x; # Position in Flugrichtung X Koord. achtern [m]
    my $y4 = $y-$p90y; # Position in Flugrichtung Y Koord. achtern [m]
    # Eckpolygon (Bildrahmen)
    my $poly = mkGeoRect($srid,
                         $x1+$p90x,$y1+$p90y,
                         $x2-$p90x,$y2-$p90y,
                         $x3-$px,  $y3-$py,
                         $x4+$px,  $y4+$py);
    # Ecken (Bildrahmen)  
    my $ul = mkGeoPoint($srid, $x1+$p90x, $y1+$p90y); # upper left
    my $lr = mkGeoPoint($srid, $x2-$p90x, $y2-$p90y); # lower left
    my $ll = mkGeoPoint($srid, $x3-$px, $y3-$py);     # lower right
    my $ur = mkGeoPoint($srid, $x4+$px, $y4+$py);     # upper right
    return ($cam_id, $ncol, $nrow, $wx, $wy, $sx, $sy, $vz, $ul, $ll, $ur, $lr, $poly);
}
# ===================================================================
# EOF
# ===================================================================

#!/usr/bin/perl -w
# mk-geo-ref.pl ---
# ========================================================
# Verknuepfung der Bid und der Flugdaten und Berechnung der
# Referenzierungspunkte
# (c) 2013 IFAOE.DE,  A. Weidauer 
# ========================================================
use strict;
use lib "/home/census/tools/Perl";
use Config::BildflugWismar2010;
use Data::Dumper;
use List::Util qw[min max];
use Math::Round qw/round/;
use Math::Complex;
use Math::Trig;
use UserUtils;

# ===================================================================
# Standarteinstellungen 
# ===================================================================
# Achtung ggf. Namen normalisieren
# for f in `ls *.TIF`; do n=$(echo $f | sed -e 's/_entw//g'); mv $f $n; done

my %PRJ_ENV      = BildflugWismar2010::projectEnviron();
my $USER         = $PRJ_ENV{"USER"},"\n";
my $DATE         = UserUtils::currentDate();
my $DATA_PATH    = $PRJ_ENV{"DATA"};
my $DB_NAME      = $PRJ_ENV{"DBNAME"};
my $SESSION      = '2010-03-31';
my $DEBUG        = 0;
my $IMG_CLEAN    = 1;    
my $EPSG         = 32632;
my $SRC_IMG_PATH = $DATA_PATH."/img-tif-raw";
my $DST_IMG_PATH = $DATA_PATH."/img-tif-geo";
my $TMP_PATH     = $DATA_PATH."/temp";
my $BLOCKX       = 512;
my $BLOCKY       = 512;
my $OFORMAT      = "GTiff";
my $COMPRESS     = "LZW";
my $TARGET_RES   = "0.02";

# ----------------------------------------------------------------------------------
# near         - nearest neighbour resampling 
# bilinear     - bilinear resampling. 
# cubic        - cubic resampling. 
# cubicspline  - cubic spline resampling. 
# lanczos      - Lanczos windowed sinc resampling. 
# average      - average resampling, computes the average of all non-NODATA 
# ----------------------------------------------------------------------------------
my $RESAMPLE     = "cubicspline";
my $PYRAMIDE     = " 2 4 8 16";
my $GDALWARP     = "/usr/local/bin/gdalwarp";
my $GDALTRANS    = "/usr/local/bin/gdal_translate";
my $GDALADDO     = "/usr/local/bin/gdaladdo";
my $GCONF        = "--config GDAL_CACHEMAX 2000 -wm 2000 -wo NUM_THREADS=6";

# ===================================================================
# SQL Zugriffe 
# ===================================================================

my $SQL_PASS_PNT = "select img_file, st_x(utm_ul) as ulx, st_y(utm_ul) as uly, st_x(utm_ll) as llx, st_y(utm_ll) as lly, st_x(utm_ur) as urx, st_y(utm_ur) as ury, st_x(utm_lr) as lrx, st_y(utm_lr) as lry, img_width, img_height from incoming.image_log where  utm_ll is not null and utm_ul is not null and  utm_lr is not null and utm_ur is not null and session =\'$SESSION\' order by img_file;";
# ------------------------------------------------------
# Greetings
# ------------------------------------------------------
print "Georeferenzierung von digitalen Flugbildern:\n";
print "--------------------------------------------\n\n";
print "Parameter:\n";
print "----------\n";
print "Nutzer:         ",$USER,"\n";
print "Zeit:           ",$DATE,"\n";
print "SESSION:        ",$SESSION,"\n";
print "DB-NAME:        ",$DB_NAME,"\n";
print "IMG-CLEAN:      ",$IMG_CLEAN,"\n";
print "EPSG:           ",$EPSG,"\n";
print "Bildquellen:    ",$SRC_IMG_PATH,"\n";
print "Bildablage:     ",$DST_IMG_PATH,"/",$SESSION,"\n";
print "Bildformat:     ",$OFORMAT,"\n";
print "Bloecke X:      ",$BLOCKX,"\n";
print "Bloecke Y:      ",$BLOCKY,"\n";
print "Kompression:    ",$RESAMPLE,"\n";
print "Algorithmus:    ",$COMPRESS,"\n";
print "Bildkaskade:    ",$PYRAMIDE,"\n";
print "Zielaufloesung: ",$TARGET_RES," [m]\n";

# Testen und absolut machen 
die "Unbekanntes Verzeichnis fuer Bildquellen!" if ! -e $SRC_IMG_PATH;
my @res=`readlink -f $SRC_IMG_PATH`;$SRC_IMG_PATH=$res[0]; chomp($SRC_IMG_PATH);



print "\n..Anlegen des des Zielverzeichnisses $DST_IMG_PATH/$SESSION ";
if ( ! -e $DST_IMG_PATH ) {
    system("mkdir $DST_IMG_PATH")  == 0 or die
        "Verzeichnis $DST_IMG_PATH konnte nicht angelegt werden!\n".
        " Details: $? !\n";
}
my $DST_PATH= $DST_IMG_PATH."/".$SESSION ;

if ( ! -e $DST_PATH) {
    system("mkdir $DST_PATH")  == 0 or die
        "Verzeichnis $DST_PATH konnte nicht angelegt werden!\n".
        " Details: $? !\n";
}
print ".OK\n";

die "Unbekanntes Verzeichnis fuer Bildquellen!" if ! -e $DST_PATH;
@res=`readlink -f $DST_PATH`;$DST_PATH=$res[0]; chomp($DST_PATH);
    
if (  $DST_PATH eq $SRC_IMG_PATH) {
    die "Quell- und Zielverzeichnis stimmen ueberein!".
        " Es besteht die Gefahr des Ueberschreibens!"
}

if ( $IMG_CLEAN && -e $DST_PATH ) {
    print "\n..Leeren des Zielverzeichnisses $DST_PATH ";
    system("rm -vf $DST_PATH/*");
    print ".OK\n";  
}

print "\n..einlesen der Passpunktdaten ";
my $cmd="psql -A -t -d $DB_NAME -c \"$SQL_PASS_PNT\""; my @files =`$cmd`;
die "Es wurden keine Passpunkte gefunden!\n" if $#files < 0;
my $nump =$#files+1;my $cntp = 0;
print ".$nump referenzierte Bilddatensaetze .OK!\n"; 

for my $line (@files) {
    chomp($line); $cntp++;
    my ($img_file, $ulx, $uly, $llx, $lly, $urx, $ury, $lrx, $lry, $wd, $hg) =    
        split (/\|/, $line);  $wd++; $hg++;
    print "\n..Referenziere Bild $cntp von $nump ";
    my $ifile = $SRC_IMG_PATH."/".$img_file;
    my $ofile = $DST_PATH."/".$img_file;
    my $tfile = $TMP_PATH."/".$img_file;    

    if ( -e $ofile) { system ("rm -f $ofile "); }
    if ( -e $tfile) { system ("rm -f $tfile "); }

    my $cwd = $wd/2; my $chg = $hg/2;
    my $cpx = ( $llx + $lrx+ $urx + $ulx) / 4.0;
    my $cpy = ( $lly + $lry+ $ury + $uly) / 4.0;
    
    print "..Vermessen der Quelldatei\n" ;
    my $cmd = "$GDALTRANS -of GTiff\\\n".
	" -gcp   0     0 $ulx   $uly \\\n".
	" -gcp   0   $hg $llx   $lly \\\n".
	" -gcp $cwd $chg $cpx   $cpy \\\n".
	" -gcp $wd     0 $urx   $ury \\\n".
	" -gcp $wd   $hg $lrx   $lry \\\n".
	" -a_srs epsg:$EPSG\\\n $ifile\\\n $tfile";

    # print "\n$cmd\n";

    system($cmd) == 0 or die "Abbruch: Operation fehlgeschlagen!\n";

    print "..Interpolation achsenparalleles Bild \n" ;
    $cmd = "$GDALWARP $GCONF -tr $TARGET_RES $TARGET_RES \\\n". #-co COMPRESS=$COMPRESS
	" -co TILED=YES -co BLOCKXSIZE=$BLOCKX -co BLOCKYSIZE=$BLOCKY\\\n".
	" -of $OFORMAT -r $RESAMPLE -dstnodata \'0 0 0\'\\\n".
	" -t_srs epsg:$EPSG\\\n $tfile \\\n $ofile";
   #  print "$cmd  \n";
    
    system($cmd)==0 or die "Abbruch: Operation fehlgeschlagen!";
    system("rm $tfile")==0 or
    die "Abbruch: Temporaerdatei konnte nicht geloescht werden!";

    print "..Bildpyramide\n" ;
    $cmd="$GDALADDO -r gauss ".$ofile." ".$PYRAMIDE;
    system($cmd) ==0 or die "Abbruch: Operation fehlgeschlagen!\n";
    
}
print ".OK";

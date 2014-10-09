#!/usr/bin/perl -w
# mk-img-samples.pl ---
# ========================================================
# Auslesen der Bildumgebung um die Voegel und speichern
# 
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
my $USER         = $PRJ_ENV{"USER"};
my $DATE         = UserUtils::currentDate();
my $DATA_PATH    = $PRJ_ENV{"DATA"};
my $DB_NAME      = $PRJ_ENV{"DBNAME"};
my $SESSION      = '2010-03-31';
my $DEBUG        = 0;
my $IMG_CLEAN    = 1;    
my $EPSG         = 32632;
my $SRC_IMG_PATH = $DATA_PATH."/img-tif-geo/".$SESSION;
my $DST_IMG_PATH = $DATA_PATH."/img-cns-smpl/".$SESSION;

my $SQL_QRY_IMG = "select distinct img_file from incoming.image_log where utm_roi is not null order by img_file;";
my $SQL_QRY_CNS = "select cns_pk, taxon, gender, behavior, quality, round(utm_x::numeric,2) as utmx, round(utm_y::numeric,2) as utm_y, num from incoming.census where img_file=\'%s\';";
my $SQL_CLN_IMG = "delete from incoming.image_samples where session=\'$SESSION\';";

if ($IMG_CLEAN) {
    my $cmd="psql -d $DB_NAME -c \"$SQL_CLN_IMG\""; my @res =`$cmd`;
    print "..loesche Datenbankeintraege ",@res;
    print "..loesche Bilddateien:\n";
    system("rm -fv $DST_IMG_PATH/*.TIF");
}

# Woertebucher fuer die Uabersetzung der Dateinamen
my %TX_DICT = qw(Eiderente EDE Eisente EIE Gründelente GRE Heringsmöwe HRM Trauerente TRE);
my %BH_DICT = qw(fliegend FL schwimmend SW);
my $SIZE    = 64;
my @fields  = qw(cns_ref S:session S:img_file size);

my $cmd="psql -A -t -d $DB_NAME -c \"$SQL_QRY_IMG\""; my @images =`$cmd`;
for my $image (@images) {
    chomp($image);
    my $ikey = $image; $ikey =~ s/\.TIF//;
    my $qry = sprintf($SQL_QRY_CNS, $image);
    $cmd="psql -A -t -d $DB_NAME -c \"$qry\""; my @birds =`$cmd`;
    next if $#birds<0;
    my @points = (); 
    for my $bird (@birds) {
	chomp($bird);
	my ($pk, $tax, $gnd, $bhv, $qly, $utmx, $utmy, $num) =  split (/\|/, $bird);
	$points[++$#points]=$pk; $points[++$#points]=$utmx; $points[++$#points]=$utmy;
    }
    my $ifile=$SRC_IMG_PATH."/".$image; my $ofile=$DST_IMG_PATH."/".$ikey;
    print "..Bildschnitt fuer $ikey (ID X Y ...): ".join(" ",@points)."\n";
    $cmd="geo-cut $ifile $ofile .TIF $SIZE ".join(" ",@points);
    my @add=`$cmd | grep ADD`;
    for my $line (@add) {
	chomp($line);
	my ($tk, $pk, $fn) = split(/ /,$line);
	if ($tk=~/ADD/) {
	    print "..registriere davon $ikey ID: $pk\n";
	    my @data = ($pk, $SESSION, $image, $SIZE);
	    $qry= UserUtils::listDataAsSqlInsert("incoming.image_samples",\@fields,\@data);
	    $cmd="psql -A -t -d $DB_NAME -c \"$qry\""; my @res =`$cmd`;
	    die "Fehler beim Erstellen der Samples $image! \n" if ! $res[0] =~/INSERT/; 
        }
    }
    # system($cmd)==0 or die "Abbruch: Operation fehlgeschlagen!\n";
}

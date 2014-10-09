#!/usr/bin/perl -w
# ========================================================
# Import der GPS Dateien in eine Datenbank
# (c) 2013 IFAOE.DE,  A. Weidauer 
# ========================================================

use lib "/home/census/tools/Perl";
use Config::BildflugWismar2010;
use Data::Dumper;
use UserUtils;

# ========================================================
my %PRJ_ENV=BildflugWismar2010::projectEnviron();
my $SESSION='2010-03-31';
my $SRC_PATTERN=$PRJ_ENV{"DATA"}."/gps-tracks/*.txt";
my $DB_CLEAN=1;    
my $DB_TABLE="incoming.tracks";
my $DB_NAME=$PRJ_ENV{"DBNAME"};
my $DATE=UserUtils::currentDate();
my $EPSG=32632;
my $DEBUG=0;

# ========================================================
my $track_cnt =0;

# ------------------------------------------------------
# Greetings
# ------------------------------------------------------
print "IMPORT Transektdateien:\n";
print "-----------------------\n\n";
print "Parameter:\n";
print "----------\n";
print "Nutzer:   ",$PRJ_ENV{"USER"},"\n";
print "Zeit:     ",$DATE,"\n";
print "SESSION:  ",$SESSION,"\n";
print "Dateien:  ",$SRC_PATTERN,"\n";
print "DB-CLEAN: ",$DB_CLEAN,"\n";
print "TABELLE:  ",$DB_TABLE,"\n";

# ------------------------------------------------------
# Datenbank aufraeumen 
# ------------------------------------------------------
if ($DB_CLEAN) {
    my $qry="delete from $DB_TABLE where session = \'$SESSION\';\n";
    print "\n..zuruecksetzen der Transekte fuer Session \'$SESSION\'\n";
    my $cmd="psql -d $DB_NAME -c \"$qry\"";
    my @res =`$cmd`; print @res if $DEBUG;
    my @num = split(/ /,$res[0]); my $num = $num[1]; chomp($num);
    print "  $num Eintraege geloescht .OK\n";
    
}

# ------------------------------------------------------
# EXIF Dateien oeffnen
# ------------------------------------------------------
print "\n..lese Dateien aus ",$SRC_PATTERN,"\n";
@files=`ls $SRC_PATTERN`;
print "FILES: ", join(" ",@files),"\n" if $DEBUG;
my $numf = $#files+1; my $cntf =0; my $cnt =0;
my %was = ();
print "  $numf Eintraege gefunden\n";

my $trc_cnt=0;
# ------------------------------------------------------
# Daten importieren 
# ------------------------------------------------------

for my $file (@files) {
    $cntf++;
    print "\n..importiere Datei $cntf von $numf\n>", $file;
    my @lines=&openGPS($file);
    my $numl = $#lines+1; my $cntl =0;    
    for $line (@lines) {
	$cnt++;
	print $line,"\n" if $DEBUG;
	my ($date, $time, $lat, $lon, $x, $zone, $y,
	    $hgt, $hd, $rel, $cnt, $image, $speed, $laps)
	    = split (/\|/, $line);
	$time =~ s/\-/:/g;
        my $key = $time."|".$x."|".$y."|".$hgt;
        if ( ! $was{$key} ) {
            $cntl++;
            my @hash=(
                   "N:trc_num"      ,$trc_cnt,
		   "N:trc_srid",     $EPSG,
		   "S:session",  $SESSION,
		   "T:trc_time", $date." ".$time,
		   "N:trc_lat",   $lat, 
		   "N:trc_lon",   $lon, 
		   "N:trc_x" ,    $x, 
		   "N:trc_y",     $y, 
		   "N:trc_z" ,    $hgt, 
		   "N:trc_head",  $hd, 
		   "N:trc_irel",  $rel, 
		   "N:trc_inum",  $image, 
		   "N:trc_icnt",  $cnt, 
		   "N:trc_speed", $speed, 
		   "N:trc_laps",  $laps);
            print UserUtils::listHash(%hash) if $DEBUG;
            my  $qry=UserUtils::listHashAsSql($DB_TABLE,@hash);
            my $cmd   = "psql -d $DB_NAME -c \"$qry\"";
            my @res =`$cmd`; print "..schreibe Koordiante $cntl von $numl ",@res;
            @hash = undef; @res = undef;
            $was{$key} = 1;
        }
        exit 1 if $DEBUG;       
    }
    @lines = undef;
    
}

my $qry="update incoming.tracks set utm_pos=st_setsrid(st_makepoint(trc_x,trc_y,trc_z),trc_srid);\n";
my $cmd   = "psql -d $DB_NAME -c \"$qry\"";
my @res =`$cmd`; print "\n..erzeuge UTM Layer ",@res;
my $qry = "update incoming.tracks set ll_pos=st_setsrid(st_makepoint(trc_lon,trc_lat,trc_z),4326);\n";
my $cmd   = "psql -d $DB_NAME -c \"$qry\"";
my @res =`$cmd`; print "..erzeuge LL Layer ",@res;
print "\nStatistik:\n";
print "-----------\n";
@keys = keys %was;
print " Koordinaten: ", $#keys,"\n";
print " Doubletten:  ", $cnt-$#keys,"\n";
print "\n.OK\n";

# == EOF MAIN ============================================


# ========================================================
# Service Routines
# ========================================================

# ------------------------------------------------------
# XMP Datei einlesen und in Array umwandeln
# ------------------------------------------------------
sub openGPS($) {
    my $fname = shift;
    open(FH,"<$fname")  || die "Datei $fname kann nicht gelesen werden!\n";
    while (my $line = <FH>) {
	next if $line =~ /^#/;
	next if $line =~ /^\s+$/;
	next if $line =~ /Grid/;
	$line =~ s/\t+/ /g;
	$line =~ s/\s+/|/g;
	chomp($line);
	$lines[++$#lines] = $line; 
    }
    close(FH);
    return @lines;
}

# ------------------------------------------------------
# EOF
# ------------------------------------------------------

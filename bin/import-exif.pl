#!/usr/bin/perl -w
# ========================================================
# Import der Exif Dateien in eine Datenbank
# (c) 2013 IFAOE.DE,  A. Weidauer 
# ========================================================

use lib "/home/census/tools/Perl";
use Config::BildflugWismar2010;
use Data::Dumper;
use UserUtils;

# ========================================================
# Projektparameter laden 
# ========================================================
my %PRJ_ENV=BildflugWismar2010::projectEnviron();

# ========================================================
# Rahmenparameter
# @todo in Programmparameter umwandels
# ========================================================
my $SESSION='2010-03-31';
my $SRC_PATTERN=$PRJ_ENV{"DATA"}."/exif-tif-raw/*.xmp";
my $DB_CLEAN=1;    
my $DB_TABLE="incoming.exif";
my $DB_NAME=$PRJ_ENV{"DBNAME"};
my $DATE=UserUtils::currentDate();

print "IMPORT von EXIF Dateien:\n";
print "-----------------------\n\n";
print "Parameter:\n";
print "----------\n";
print "Nutzer:   ",$PRJ_ENV{"USER"},"\n";
print "Zeit:     ",$DATE,"\n";
print "SESSION:  ",$SESSION,"\n";
print "Dateien:  ",$SRC_PATTERN,"\n";
print "DB-CLEAN: ",$DB_CLEAN,"\n";
print "TABELLE:  ",$DB_TABLE,"\n\n";

# ------------------------------------------------------
# Abfragevariable initialisieren
# ------------------------------------------------------
my $SQL_CLEAN_EXIF="delete from $DB_TABLE where session = \'$SESSION\';";

# ------------------------------------------------------
# Datenbank aufraeumen 
# ------------------------------------------------------
if ($DB_CLEAN) {
    print "..zuruecksetzen der EXIF-Daten fuer Session \'$SESSION\'\n";
    my $cmd="psql -d $DB_NAME -c \"$SQL_CLEAN_EXIF\"";
    my @res =`$cmd`; print @res if $DEBUG;
    my @num = split(/ /,$res[0]); my $num = $num[1]; chomp($num);
    print "  $num Eintraege geloescht .OK\n\n";
}

# ------------------------------------------------------
# EXIF Dateien oeffnen
# ------------------------------------------------------
print "..suche EXIF-Daten ";
@files=`ls $SRC_PATTERN`;
my $cnt=0; my $num=$#files+1; 
print "$num Dateien gefunden .OK\n\n";

# ------------------------------------------------------
# Daten importieren 
# ------------------------------------------------------
for my $file (@files) {
    my @lines = &openXMP($file); $cnt++;
    print "..lese Datei $cnt von $num ";
    my @data  = &readMetaTags($SESSION, @lines);
    my $qry   = UserUtils::listHashAsSql($DB_TABLE,@data);
    my $cmd   = "psql -d $DB_NAME -c \"$qry\"";
    my @res =`$cmd`; print "..schreibe Daten ",@res;
    undef @lines; undef @data; undef @res;
}
print ".OK\n";

# ========================================================
# Service Routines
# ========================================================

# ------------------------------------------------------
# Metdaten parsen und Feldnamen und Typenkonvetion anbringen
# umwandeln S - String, T - Timestamp, F - Float
# ------------------------------------------------------
sub readMetaTags($@) {
 my $sid = shift;
 my $sdt  = &getTagVal("exif:","DateTimeDigitized",@_);
 my $nm   = &getTagVal("crs:","RawFileName",@_);
 my $tmp  = &getTagVal("crs:","Temperature",@_);
 my $tint = &getTagVal("crs:","Tint",@_);
 my $exp  = &getTagVal("crs:","Exposure",@_);
 my $shd  = &getTagVal("crs:","Shadows",@_);
 my $brg  = &getTagVal("crs:","Brightness",@_);
 my $cnt  = &getTagVal("crs:","Contrast",@_);
 my $sat  = &getTagVal("crs:","Saturation",@_);
 my $srp  = &getTagVal("crs:","Sharpness",@_);
 $sdt = &strDateToIso($sdt);
 return ( 'S:session',   $sid, 'S:raw_file', $nm,
          'T:exif_time', $sdt, 'F:crs_temp',   $tmp,  'F:crs_tint',   $tint,
	  'F:crs_expose',    $exp, 'F:crs_shade',  $shd,  'F:crs_bright', $brg, 
	  'F:crs_contrast',  $cnt, 'F:crs_sature', $sat, 'F:crs_sharp',   $srp);
}

# ------------------------------------------------------
# XMP Datei einlesen und in Array umwandeln
# ------------------------------------------------------
sub openXMP($) {
    my $fname = shift;
    open(FH,"<$fname")  || die "Datei $fname kann nicht gelesen werden!\n";
    while (my $line = <FH>) {
	next if $line =~ /^\s+$/;
	chomp($line);
	$line =~ s/^\s+//g;
	$line =~ s/\s+$//g;
	$lines[++$#lines] = $line; 
    }
    close(FH);
    return @lines;
}

# ------------------------------------------------------
# Tag mit Prefix finden und zurueckgeben
# $1 Prefix
# $2 Tag
# ------------------------------------------------------
sub getTagVal($$@) {
    my %found = parseTag(shift,shift,@_);
    my @vals = values %found;
    my $res = $vals[0]; undef @vals; undef %found;
    return $res;
}


# ------------------------------------------------------
# Tags parsen die zu einem Prefix Suffixpattern gehoeren
# ------------------------------------------------------
sub parseTag($$@) {
    my $pfx=shift;my $sfx=shift;
    my $pat="\(<".$pfx.$sfx."[A-Za-z]*>\)\(.*\)\(<\/".$pfx.$sfx."[A-Za-z]*>\)";
    my @found=grep(/$pat/,@_);
    my %res =();
    for $dta (@found) { 
	my $val = $dta; $val =~ s/$pat/$2/; 
	my $key = $dta; $key =~ s/$pat/$1/g; 
	$key =~ s/$pfx|\/|<|>//g; 
	$res{$key}=$val;

    }
    undef @found;
    return %res;
}

# ------------------------------------------------------
# Datumsformat parsen und in ISO - Format umwandeln
# ------------------------------------------------------
sub strDateToIso($) {
    my $dt = shift;
    # print $dt;
    my @res=split(/T/, $dt); 
    my ($yr,$mo, $dy) = split(/-/,$res[0]);
    my ($hr,$min,$tmp,$tzm) = split(/\:/,$res[1]);
    my ($sec,$tz) = split(/\+/,$tmp);
    undef @res;
    return join("-",($yr,$mo,$dy))." ".join(":",($hr,$min,$sec))."+".$tz;
    # print join("-",($yr,$mo,$dy))," ",join(":",($hr,$min,$sec)),"+",$tz,"\n";
}

# ------------------------------------------------------
# EOF
# ------------------------------------------------------

#!/usr/bin/perl -w
# ========================================================
# Interpolation der GPS Track im Sekundentakt
# (c) 2013 IFAOE.DE,  A. Weidauer 
# ========================================================
use strict;
use lib "/home/census/tools/Perl";
use Config::BildflugWismar2010;
use Data::Dumper;
use Math::Round qw/round/;
use Math::Complex;
use UserUtils;

# ===================================================================
# Standarteinstellungen 
# ===================================================================
my %PRJ_ENV=BildflugWismar2010::projectEnviron();
my $SESSION='2010-03-31';
my $SRC_PATTERN=$PRJ_ENV{"DATA"}."/gps-tracks/*.txt";
my $DB_CLEAN=1;    
my $DB_NAME=$PRJ_ENV{"DBNAME"};
my $DATE=UserUtils::currentDate();
my $EPSG=32632;
my $DEBUG=0;
my $TFILE=$PRJ_ENV{"DATA"}."/gps-tracks/tracks";
my $TLIMIT=60;
my $CONF=3;

# -------------------------------------------------------------------
# Variablen vorbereiten 
# -------------------------------------------------------------------
# Abfrage fuer mittlere Abstaende 
my $SQL_QUERY_DIST_STATS ="select avg(dt) as adt,\\\
 avg(dr) as adr, stddev(dt) as sdt,\\\
 stddev(dr) as sdr\\\
 from incoming.track_dist\\\
 where dt < $TLIMIT and s = \'$SESSION\';
";

# Trackzaehler fuer die Session zuruecksetzen 
my $SQL_CLR_TRACK_NUM   = "update incoming.tracks\\\
 set trc_num=-1";

# Template Tracknummer setzen 
my $SQL_UPD_TRACK_NUM   = "update incoming.tracks\\\
 set trc_num=%d where trc_pk in (%s)";

# Abfrage zur Ermittlung der Differenzdaten
my $SQL_QUERY_DIFF_DATA = "select pk, s, n, t, dt, dr from\\\
 incoming.track_dist where s=\'$SESSION\' order by t;";

# Abfrage zur Ermittlung der Differenzdaten
my $SQL_QUERY_XYZ_DATA = "select pk, n, extract(epoch from t) as nt,\\\
 x, y, z, pln_rad, pln_grd from\\\
 incoming.track_stack where s=\'$SESSION\' and n=%d order by t;";

# Trackzaehler fuer die Session zuruecksetzen 
my $SQL_CLR_TRACK_HRES   = "delete from incoming.tracks_hires\\\
 where session=\'$SESSION\'";

# Abfragevariable initialisieren
my $cmd =""; my $qry=""; my @res=();

# -------------------------------------------------------------------
# Zuruecksetzen der Tracking-Nummerierung
# -------------------------------------------------------------------
print "\n..zuruecksetzen der Track-Nummerierung fuer Session \'$SESSION\'\n";
$cmd="psql -A -t -d $DB_NAME -c \"$SQL_CLR_TRACK_NUM\"";
@res =`$cmd`; print @res if $DEBUG;
my @num = split(/ /,$res[0]); my $num = $num[1]; chomp($num);
print "..$num Tracking-Punkte gefunden\n";

# -------------------------------------------------------------------
# Abstandsstatisik berechnen und herausgeben 
# -------------------------------------------------------------------
$cmd="psql -A -t -d $DB_NAME -c \"$SQL_QUERY_DIST_STATS\"";
@res =`$cmd`; print @res if $DEBUG;
my ($adt,$adr,$sdt,$sdr) = split(/\|/,$res[0]);chomp($sdr);
my $ldt = $adt+$CONF*$sdt; my $ldr = $adr+$CONF*$sdr;
# $ldt=50; $ldr=500;
print "\n..berechne Abstandsstatistik Session \'$SESSION\'\n";
print "Statistik:\n";
print "----------\n";
printf (" Faktor Konfidenz:      %d\n", $CONF);
printf (" Mittel Distanz Zeit:   %3.2f [s]\n", $adt);
printf (" St.Abw. Distanz Zeit:  %3.2f [s]\n", $sdt);
printf (" Mittel Distanz Raum:   %3.2f [m]\n", $adr);
printf (" St.Abw. Distanz Raum:  %3.2f [m]\n", $sdr);
printf (" Obere Grenze Zeiten:   %3.2f [s]\n", $ldt);
printf (" Obere Grenze Lage:     %3.2f [m]\n", $ldr);

# -------------------------------------------------------------------
# Positionen einlesen und sortieren 
# -------------------------------------------------------------------
print "\n..Einlesen der Positionen und sortieren der Tracks\n";
$cmd="psql -A -t -d $DB_NAME -c \"$SQL_QUERY_DIFF_DATA\"";
my @data=(); @data =`$cmd`; print @data if $DEBUG;

# Variablen vorbereiten 
my %tracks = (); my $trc_cnt=0;
my @cstack = ();
my @tstack = () if $DEBUG;
my @rstack = () if $DEBUG;
my $pnum_in=$#data+1; my $pnum_out=0;

print @data,"\n" if $DEBUG;

# Sortierung vornehmen 
for my $line (@data) {

    # Werte parsen 
    chomp($line);
    my ($pk, $s, $n, $t, $dt, $dr) = split(/\|/,$line);
    
    # Abbruchbedingung testen 
    if ( $dt>$ldt or $dr>$ldr) {

        if ($DEBUG) {
            print "\ndt:", $dt," dr:", $dr,"\n";
            print "t:",$trc_cnt," ",join(",",@tstack),"\n";            
            print "r:",$trc_cnt," ",join(",",@rstack),"\n"; 
            print "p:",$trc_cnt," ",join(",",@cstack),"\n"; 
        }
        
        if ($#cstack > 0) {
            # Punktstatistik aktualisieren
            $pnum_out += $#cstack+1;
            # Schluesselaufzaehlung umwandeln
            $tracks{++$trc_cnt} = join(", ",@cstack);
        }
        
        @cstack = (); # Stack leeren
        @rstack = () if $DEBUG;; # Stack leeren
        @tstack = () if $DEBUG;; # Stack leeren

        
    } else {
        $tstack[++$#tstack] = $dt if $DEBUG;
        $cstack[++$#cstack] = $pk;
        $rstack[++$#rstack] = round($dr) if $DEBUG;
    }
        
    
}

if ($#cstack > 0) {
            $pnum_out += $#cstack+1;
            # Schluesselaufzaehlung umwandeln
            $tracks{++$trc_cnt} = join(", ",@cstack);
} 


# -------------------------------------------------------------------
# Tracksortierung schreiben
# -------------------------------------------------------------------
print "Tracks und Punkte:\n";
print "------------------\n";
print "Punkte im Dateneingang: ",$pnum_in,"\n";
print "Verbleibende Punkte:    ",$pnum_out,"\n";
print "Anzahl der Tracks:      ",$trc_cnt,"\n";

print "\n..Schreibe Track-Sorierung in die Tabelle\n";
print "Tracks:\n";
print "-------\n";

for $trc_cnt (sort( keys %tracks)) {
    my $pks = $tracks{$trc_cnt};
    $qry = sprintf($SQL_UPD_TRACK_NUM, $trc_cnt, $pks);
    print $qry if $DEBUG;
    print " Track $trc_cnt: ";
    @res = `psql -d $DB_NAME -c \"$qry\"`;
    print @res;
}

# -------------------------------------------------------------------
# Tracks interpolieren 
# -------------------------------------------------------------------
print "\n..loesche interpolierte Positionen ";
print $SQL_CLR_TRACK_HRES,"\n" if $DEBUG;
@res = `psql -d $DB_NAME -c \"$SQL_CLR_TRACK_HRES\"`;
print @res,"\n";

print "\n..interpoliere neue Positionen\n";
print "Tracks:\n";
print "-------\n";

# Datenfelder fuer die Ausgabe initialisieren 
my @sfstack=qw(trc_pk trc_pv session trc_num trc_time utm_x utm_y utm_z pln_rad pln_grd);
for $trc_cnt (sort( keys %tracks)) {

    # Track zeitsortiert einlesen
    print " Track $trc_cnt:";   
    my $pks = $tracks{$trc_cnt};
    $qry = sprintf($SQL_QUERY_XYZ_DATA, $trc_cnt, $pks);
    my @data = `psql -A -t -d $DB_NAME -c \"$qry\"`;
    print @data if $DEBUG;
    print " READ ",$#data+1,"\n";

    # Variablen fuer Zeitpunkt t1 initialisieren
    my ($pk1,  $n1, $t1, $x1, $y1, $z1, $pr1, $pg1) = (0,0,-1,0,0,0,0,0);

    # Ausgabe-Stack initialisieren 
    my @svstack=();

    # Kontrollstack
    my %was = ();
    
    # Iteration ueber ale Punktpaare vorher t1-t2
    for my $line (@data) {
        chomp($line);
        
        # Initialisierung der t1 Vektors am Anfang des Tracks und raus 
        if ($t1 == -1) {
            ($pk1, $n1, $t1, $x1, $y1, $z1, $pr1, $pg1) = split(/\|/,$line);
            next;
        }
        # Einlesen des t2 Vektors 
        my ($pk2, $n2, $t2, $x2, $y2, $z2, $pr2, $pg2) = split(/\|/,$line);

        # Zeitdifferenz berechnen
        my $dt = $t2-$t1;

        # Ortsdifferenz berechnen 
        my $dx = ($x2-$x1); my $dy = ($y2-$y1); my $dz = ($z2-$z1);
        my $dpr = ($pr2-$pr1); my $dpg = ($pg2-$pg1); 
                 
        my $dr = sqrt($dx*$dx+$dy*$dy+$dz*$dz);

        # Ausgabe wenn Punkt nicht in DB
        my $key = $t1."|".$x1."|".$y1."|".$z1;
        if ( ! $was{$key} ) {
                # Zeitmarke in Datum umrechnen
                my $cdt ="to_timestamp(".$t1.")";

                # Auf dem Stack ablegen
                $svstack[++$#svstack]="(".join(",",( $pk1, 0,"\'".$SESSION."\'",
                                     $n1, $cdt, $x1, $y1, $z1, $pr1, $pg1)).")";
                # als gesendet markieren
                $was{$key} = 1;
         }
        
         # Abbruch wenn Zeit- oder Ortsdifferenz = 0
         next if $dr==0 || $dt==0;
        
        # Progression berechnen 
        $dx/=$dt; $dy/=$dt; $dz/=$dt; $dpg/=$dt; $dpr/=$dt;         

        # Punkte innerhalb des Intervalls linear interpolieren
        # eigentlich muesste (0..$dt-1) reichen ! Sekundentakt
        for my $i (1..$dt) {

            # Zwischeschritte berechnen
            $t1++; $x1+=$dx; $y1+=$dy; $z1+=$dz; $pr1+=$dpr; $pg1+=$dpg;

            # Ausgabe wenn Punkt nicht in DB
            my $key = $t1."|".$x1."|".$y1."|".$z1;
            if ( ! $was{$key} ) {
                # Zeitmarke in Datum umrechnen
                my $cdt ="to_timestamp(".$t1.")";

                # Quellenmarker fuer Ende anpassen 
                my $num =$i; my $pk=$pk1;
                $num=0, $pk=$pk2 if $i==$dt;
                
                # Auf dem Stack ablegen
                $svstack[++$#svstack]="(".join(",",( $pk, $num,"\'".$SESSION."\'",
                                 $n1, $cdt, $x1, $y1, $z1, $pr1, $pg1)).")";
                # als gesendet markieren
                $was{$key} = 1;
            }
        }
        # Aktualisieren des t1 Vektors
        ($pk1,  $n1, $t1, $x1, $y1, $z1, $pr1, $pg1) =
                         ($pk2, $n2, $t2, $x2, $y2, $z2, $pr2, $pg2);
        # Stack auf die Datenbank schieben
        $qry ="insert into incoming.tracks_hires ("
            .join(",",@sfstack).") values "
            .join(",",@svstack).";";

        @res = `psql -d $DB_NAME -c \"$qry\"`;
        print "  ", @res;
        @svstack=();
    }
}

print "\n..erstelle Geoobjekte ";
$qry="update incoming.tracks_hires set utm_pos=st_setsrid(st_makepoint(utm_x,utm_y,utm_z),$EPSG);";
@res = `psql -d $DB_NAME -c \"$qry\"`;
print " ", @res;

# -------------------------------------------------------------------
# EOF
# -------------------------------------------------------------------


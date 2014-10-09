#!/usr/bin/perl -w
# mk-trfm-birds.pl --- 
# ========================================================
# Georeferenzierung der Vogelszaehlung
# (c) 2013 IFAOE.DE,  A. Weidauer 
# ========================================================
# Author: Alexander Weidauer <alex.weidauer@huckfinn.de>
# Created: 02 Oct 2013
# Version: 0.01
# ========================================================
use warnings;
use strict;
use lib "/home/census/tools/Perl";
use Config::BildflugWismar2010;
use Data::Dumper;
use UserUtils;

# ===================================================================
# Standarteinstellungen 
# ===================================================================
my %PRJ_ENV=BildflugWismar2010::projectEnviron();
my $USER         = $PRJ_ENV{"USER"};
my $DATE         = UserUtils::currentDate();
my $DATA_PATH    = $PRJ_ENV{"DATA"};
my $DB_NAME      = $PRJ_ENV{"DBNAME"};
my $DB_CLEAN     = 1;
my $SESSION      = '2010-03-31';
my $EPSG         = 32632;
my $DEBUG        = 0;
my $CENSUS_FILE  = $DATA_PATH."/bird-census/census";
my $CAM          = "AIC.P45";
my $TIF_TMPL     = "CF%06d.TIF";

# ===================================================================
# SQL Abfragen 
# ===================================================================
# Abfrage Kameradaten
my $SQL_QRY_CAM="select cam_type, cam_cols, cam_rows, cam_dx, cam_dy, cam_cs\\\
 from incoming.cam where cam_type=\'$CAM\';";

# ===================================================================
# Abfrage Image LOG
# ===================================================================
my $SQL_QRY_PP="select img_file, trc_num, st_x(utm_ul) as ulx, st_y(utm_ul) as uly, st_x(utm_ll) as llx, st_y(utm_ll) as lly, st_x(utm_ur) as urx, st_y(utm_ur) as ury, st_x(utm_lr) as lrx, st_y(utm_lr) as lry from incoming.image_log where  utm_ll is not null and utm_ul is not null and  utm_lr is not null and utm_ur is not null and session =\'$SESSION\' and img_file=\'%s\';";

# ===================================================================
# DB Aufraumen 
# ===================================================================
my $SQL_CLR_IMG="DELETE FROM incoming.census WHERE session =\'$SESSION\';";

if ($DB_CLEAN) {
    print "\n..zuruecksetzen der Zensusdaten fuer Session \'$SESSION\' ";
    my $cmd="psql -d $DB_NAME -c \"$SQL_CLR_IMG\"";
    my @res =`$cmd`; print ".entfernt ", @res;
}


# =============================================================
print "\n..einlesen der Kameradatendaten ";
my $cmd="psql -A -t -d $DB_NAME -c \"$SQL_QRY_CAM\""; my @res =`$cmd`;
die "Es wurden keine Daten fuer Kamera $CAM gefunden!\n" if $#res < 0; 
my ($cam_id, $cam_cols, $cam_rows, $cam_dx, $cam_dy, $cam_cs) =
    split(/\|/,$res[0]); chomp($cam_cs);
print " .OK\n";


# ===================================================================
# Variablen deklarieren
# ===================================================================
my $cur_img='';
my ($img_file, $trc_num,
    $geo_ulx, $geo_uly, $geo_llx, $geo_lly,
    $geo_urx, $geo_ury, $geo_lrx, $geo_lry) =
    ('',0,0,0,0,0,0,0,0,0);

# ===================================================================
# Hash fuer blider ohne Tracks 
# ===================================================================
my %skipped = ();

# Zensus lesen
my @data=`cat $CENSUS_FILE`;

# print @data;
my $head=0;

# Datenbankfelder definieren 
my @fields = qw(S:session trc_num S:img_file S:taxon S:gender
                quality S:behavior num scol srow utm_x utm_y utm_cpos);

# Recordstatistik 
my $numd = $#data+1;
my $cntd =0; my $cntq =0; my $cnts =0;

print "\n..importiere $numd Vogelzaehlungen\n";

for my $data (@data) {
    if (! $head ) { $head=1; next; }
    chomp($data);
    $cntd++;
    
    print " ..$cntd von $numd / verworfen $cnts / akzeptiert $cntq\n"; 
    # Zensus parsen
    my ($trc, $img, $tax, $gnd, $qly, $bhv, $cnt, $col, $row )
        = split(/\|/,$data);
    
    # Bildname bauen
    $img = sprintf($TIF_TMPL,$img);

    # Aussteigen falls Bild im skipped cache
    $cnts++, next if $skipped{$img};
    print "IMG: ",$img,"\n" if $DEBUG;

    # Nachladen der Transformationsdaten
    # falls sich der Bildname aendert 
    if ( !( $cur_img eq $img)  ) {
        # Daten holen   
        my $qry  = sprintf($SQL_QRY_PP,$img); 
        $cmd  = "psql -A -t -d $DB_NAME -c \"$qry\"";
        my $sdata =`$cmd`; chomp($sdata);
        print "ILOG:", $sdata,"\n" if $DEBUG;

        # Markieren falls es keine Transformationen gibt
        $skipped{$img}=1, $cnts++, next if ! $sdata;

        # Daten parsen
        ($img_file, $trc_num,
         $geo_ulx, $geo_uly, $geo_llx, $geo_lly,
         $geo_urx, $geo_ury, $geo_lrx, $geo_lry) =
                split (/\|/, $sdata);
        # Aktuelles Bild setzen
        $cur_img = $img;        
    }
    
    # Manipulation der Unstimmigkeiten in der Zensus Datei
    # Wellenringe und Abtauchwellen verwerfen
    $cnts++, next if $tax =~ /Wellenring/;
    $cnts++, next if $tax =~ /Abtauchwelle/;
    
    # Qualitaet numerisch setzen
    $qly = '5' if $tax =~/\?/;
    $qly = '4' if $qly =~/unsicher/;
    $qly = '3' if $qly =~/mittel/;
    $qly = '2' if $qly =~/sicher/;

    # Arten mit ? anpassen
    $tax = 'Gründelente' if $tax =~ /Gründelente\?/;
    $tax = 'Eiderente' if $tax =~ /Eiderente\?/;
    $tax = 'Heringsmöwe' if $tax =~ /Möwe?/;

    # Kontrollausgabe
    print "UL: $geo_ulx, $geo_uly\nLL: $geo_llx, $geo_lly\n" if $DEBUG;
    print "UR: $geo_urx, $geo_ury\nLR: $geo_lrx, $geo_lry\n" if $DEBUG;
    print "CR: $cam_cols, $cam_rows\n" if $DEBUG;
   
    # PAsspunktensemble bauen
    my @gpul = ($geo_ulx, $geo_uly,         0,        0);
    my @gpll = ($geo_llx, $geo_lly,         0, $cam_rows);
    my @gpur = ($geo_urx, $geo_ury, $cam_cols,        0);
    my @gplr = ($geo_lrx, $geo_lry, $cam_cols, $cam_rows);

    # Transformation der Pixel auf Weltkoordinaten
    $cmd = "gpc_trfm -i -s '\|\'".
    " -p ".join(" ",@gpll).
    " -p ".join(" ",@gplr).
    " -p ".join(" ",@gpul).
    " -p ".join(" ",@gpur).
    " -t $col, $row ".
    "  | grep -v \'^#\'";

    #  Pixel und  Weltkoordinaten auslesen
    my $tdata = `$cmd`; chomp($tdata);
    my ($sx, $sy, $wx, $wy, $ex, $ey) = split (/\|/, $tdata);

    #  Datenbankvektor konfektionieren
    my @values= ($SESSION, $trc_num, $img, $tax, $gnd, $qly, $bhv,
     $cnt, $col, $row, $wx, $wy, &mkGeoPoint($EPSG,$wx, $wy));

    #  SQL String bauen 
    my $qry= UserUtils::listDataAsSqlInsert("incoming.census",\@fields,\@values);

    #  An die Datenbank schicken
    my $cmd="psql -A -t -d $DB_NAME -c \"$qry\""; my @res =`$cmd`;
    die "Fehler beim Transformieren des Zensus $img! \n" if ! $res[0] =~/INSERT/;       $cntq++; 
}
print "\n.OK\n";


# -------------------------------------------------------------------
# Definition eines Punktes
# srid - EPSG, x, y - Koordinaten
# -------------------------------------------------------------------
sub mkGeoPoint {
    my ($srid, $x, $y) =@_;
    my $txt = "SRID=$srid ;POINT($x $y)";
    return " GeomFromEWKT(\'$txt\') ";
}


__END__

=head1 NAME

mk-trfm-birds.pl - Describe the usage of script briefly

=head1 SYNOPSIS

mk-trfm-birds.pl [options] args

      -opt --long      Option description

=head1 DESCRIPTION

Stub documentation for mk-trfm-birds.pl, 

=head1 AUTHOR

Alexander Weidauer, E<lt>huckfinn@whee.bigopensky.deE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright (C) 2013 by Alexander Weidauer

This program is free software; you can redistribute it and/or modify
it under the same terms as Perl itself, either Perl version 5.8.2 or,
at your option, any later version of Perl 5 you may have available.

=head1 BUGS

None reported... yet.

=cut

-- ------------------------------------------------------------------
-- Datenbankschem fuer einlaufende Daten speziell fuer den Bildflug
-- vom 2010-03 Wismarbucht und dem Neuaufbau der Bilddatenbank
-- ------------------------------------------------------------------
-- (c) 2013 IFAOE.DE;  Alexander Weidauer  weidauer@ifaoe.de
-- ------------------------------------------------------------------
drop schema incoming cascade;
create schema incoming;

-- ------------------------------------------------------------------
-- Funktion zur Erstellung von Liniensegmenten 2D
-- ------------------------------------------------------------------
drop function if exists geo_mk_segment( srid int, x1 float8, y1 float8, x2 float8, y2 float8);
create function geo_mk_segment( srid int, x1 float8, y1 float8, x2 float8, y2 float8)
returns geometry as $$
declare txt text;
begin
  txt = 'srid='||$1||';linestring('||$2||' '||$3||','||$4||' '||$5||')';
  -- raise notice '%', txt;           
  return geomfromewkt(txt);
end;
$$  language plpgsql;

-- ------------------------------------------------------------------
-- Funktion zur Erstellung von Liniensegmenten 3D
-- ------------------------------------------------------------------
drop function if exists geo_mk_segment(
     	      srid int, 
	      x1 float8, y1 float8, z1 float8, 
	      x2 float8, y2 float8, z2 float8);
create function geo_mk_segment(
       srid int, 
       x1 float8, y1 float8, z1 float8, 
       x2 float8, y2 float8, z2 float8)
returns geometry as $$
declare txt text;
begin
  txt = 'srid='||$1||';linestring('||$2||' '||$3||' '||$4||', '
  ||$5||' '||$6||' '||$7||')';
  -- raise notice '%', txt;           
  return geomfromewkt(txt);
end;
$$  language plpgsql;

-- select geo_mk_segment(32632, 650808.33, 983461.96,3.0,650808.33,5984514.66,6.0);

-- ------------------------------------------------------------------
-- Funktion zur Erstellung von Rechtecken 2D
-- ------------------------------------------------------------------
drop function if exists geo_mk_rect(
     srid int, 
     x1 float8, y1 float8, x2 float8, y2 float8, 
     x3 float8, y3 float8, x4 float8, y4 float8);
create function geo_mk_rect(
       srid int, 
     x1 float8, y1 float8, x2 float8, y2 float8, 
     x3 float8, y3 float8, x4 float8, y4 float8)
returns geometry as $$
declare txt text;
begin
  txt = 'srid='||$1||';polygon(('||x1||' '||y1||','
  ||x3||' '||y3||','||x2||' '||y2||','
  ||x4||' '||y4||','||x1||' '||y1||'))';
  --raise notice '%', txt;           
  return geomfromewkt(txt);
end;
$$  language plpgsql;

-- ------------------------------------------------------------------
-- Stammdaten fuer die Kameras 
-- ------------------------------------------------------------------
drop table if exists incoming.cam cascade;
create table incoming.cam (
  cam_pk serial primary key,
  cam_type varchar(32) unique,
  cam_cols int,
  cam_rows int,
  cam_dx float8,
  cam_dy float8,
  cam_hx float8,
  cam_hy float8,
  cam_cs float8
);
insert into incoming.cam(cam_type, cam_cols, cam_rows, 
  cam_dx, cam_dy, cam_hx, cam_hy, cam_cs) values
('AIC.P45', 7240, 5433, 6.8E-6, 6.8E-6, 0.228, 0.023, 0.05181);
--  Patch Tiffbilder update incoming.cam set cam_cols=7240, cam_rows=5433;
-- Original ('AIC.P45', 7228, 5428, 6.8E-6, 6.8E-6, 0.228, 0.023, 0.05181);

-- ------------------------------------------------------------------
-- Tabelle der EXIF Daten fuer die einzelnen Fotos
-- ..sieh import-exif.pl
-- ------------------------------------------------------------------
-- @TODO Voll -- Die Tabelle wurde nach den vom S. Mader für den Bildflug 
-- 2010-03-31 gelieferten Daten erstellt. Aus der Adobe 
-- Spezifikation ist wenig über die Einheiten der einzelnen
-- CRS Tags zu erfahren. Die Faktoren sind demensprechend 
-- nicht klar definiert. Diese Tabelle muss ggf. fuer Bildverarbeitungs-
-- operationen  erweitert werden.
-- ------------------------------------------------------------------
drop table if exists incoming.exif cascade;
create table incoming.exif (
   exif_pk   serial primary key not null,
   raw_file  varchar(128),
   session   varchar(32),
   exif_time timestamp with time zone,
   crs_expose       float8, 
   crs_expose_ofs   float8 default 0.0, 
   crs_expose_fct   float8 default 1.0, 
   crs_tint         float8 default 0.0,
   crs_tint_ofs     float8 default 0.0,
   crs_tint_fct     float8 default 1.0,
   crs_temp         float8,
   crs_temp_ofs     float8 default 0.0,
   crs_temp_fct     float8 default 1.0,
   crs_bright       float8,
   crs_bright_ofs   float8 default 0.0,
   crs_bright_fct   float8 default 1.0,
   crs_contrast     float8,
   crs_contrast_ofs float8 default 0.0,
   crs_contrast_fct float8 default 1.0,
   crs_sature       float8,
   crs_sature_ofs   float8 default 0.0,
   crs_sature_fct   float8 default 1.0,
   crs_shade        float8,
   crs_shade_ofs    float8 default 0.0,
   crs_shade_fct    float8 default 1.0,
   crs_sharp        float8,
   crs_sharp_ofs    float8 default 0.0,
   crs_sharp_fct    float8 default 1.0
 );

comment on table incoming.exif is 
 'Exif-Daten der Eingangsdateien';

comment on column incoming.exif.exif_pk is 
 'Primärschluessel';

comment on column incoming.exif.session is 
 'Session Key z.B. Datum Dateneingang';

comment on column incoming.exif.exif_time is 
 'Datum der Aufnahme exif:DateTimeDigitized';

comment on column incoming.exif.crs_temp  is 
 'Farbtemperatur crs:Temperature';

comment on column incoming.exif.crs_temp_fct  is 
 'Farbtemperatur Faktor 1.0 = K';

comment on column incoming.exif.crs_tint  is 
 'Tönung crs:Tint';

comment on column incoming.exif.crs_tint_fct  is 
 'Faktor der Tönung 1.0 = -150..150 ';

comment on column incoming.exif.crs_temp_fct  is 
 'Farbtemperatur Faktor 1.0 = K';

comment on column incoming.exif.crs_expose is 
 'Belichtungszeit crs:Exposure -4.0..4.0';

comment on column incoming.exif.crs_expose_fct is 
 'Faktor Belichtungszeit ...';

comment on column incoming.exif.crs_bright is 
 'Helligkeit crs:Brighness';

comment on column incoming.exif.crs_bright_fct is 
 'Faktor für die Helligkeit bei 1.0 = 0..150';

comment on column incoming.exif.crs_contrast is 
 'Kontrast crs:Contrast';

comment on column incoming.exif.crs_contrast_fct is 
 'Faktor für Kontrast bei 1.0 = -50..100';

comment on column incoming.exif.crs_sature is 
 'Sättigung crs:Saturation';

comment on column incoming.exif.crs_sature_fct is 
 'Faktor für die Sättigung -100..100';

comment on column incoming.exif.crs_shade is 
 'Shadows crs:Shade';

comment on column incoming.exif.crs_shade_ofs is 
 'Offset Shadows';

comment on column incoming.exif.crs_shade_fct is 
 'Faktor Shadows bei 1.0 = 0..100';

comment on column incoming.exif.crs_sharp is 
 'Sharpness crs:Sharp';

comment on column incoming.exif.crs_shade_ofs is 
 'Offset Sharpness';

comment on column incoming.exif.crs_shade_fct is 
 'Faktor Sharpness bei 1.0 = 0..100';

-- ------------------------------------------------------------------------------
-- Tabelle der einzelnen Punkte die anschliessend zu Tracks zusammengefasst
-- werden muessen. Die Daten werden aus Log-Dateien importiert
-- .. siehe import-tracks.pl
-- ------------------------------------------------------------------------------
drop table if exists incoming.tracks cascade;
delete from geometry_columns where f_table_schema='incoming' and f_table_name='tracks';
create table incoming.tracks(
       trc_pk serial primary key,
       trc_srid int references spatial_ref_sys(srid),
       session varchar(32),
       trc_time timestamp with time zone,
       trc_lon float8 not null,
       trc_lat float8 not null,
       trc_x float8 not null,
       trc_y float8 not null,
       trc_z float8 not null,
       trc_head float8 not null,
       trc_num int, 
       trc_speed float8,
       trc_laps  float8,
       trc_icnt int,
       trc_inum int,
       trc_irel int
);
select AddGeometryColumn('incoming','tracks','utm_pos',32632,'POINT',3);
select AddGeometryColumn('incoming','tracks','ll_pos',4326,'POINT',3);

-- ------------------------------------------------------------------------------
-- Positions-Stack zur Bestimmung von Samplinglatenzen und Geschwindigkeiten
-- In dieser View werden die Ortssignale zeitlich zsortiert und Durchnummeriert
-- ------------------------------------------------------------------------------
drop view if exists incoming.track_stack cascade;
create view incoming.track_stack as
select trc_pk as pk, 
       trc_num as n,
       session as s,
       ROW_NUMBER() OVER (ORDER BY trc_time ASC) AS ix, 
       trc_time as t, 
       trc_x as x, 
       trc_y as y,
       trc_z as z,
       - (trc_head-90)/180*pi() as pln_rad,
       trc_head as pln_grd
from  incoming.tracks as a order by trc_time;


-- ------------------------------------------------------------------------------
-- Raeumliche und zeitliche Distanzen auf dem Track
-- In dieser View werden Positiosmeldungen um einen Zeitpunkt geshiftet und in
-- Zeitpunkt t und Zeit t-1 zur Bildung von Distanzen und Differenzen in Raum
-- und Zeit zusammengefasst.
-- Die Tabelle ist Grundlage fuer das Zusammenstellen von Punktzugehoerigkeiten
-- auf einer Trajektorie. Punkte die ausshalb von Samplingdistanzen (0-100m) und
-- Samplingintervallen (0-50s) bilden Terminale auf der Trajektorie und markieren
-- einzelne Tracks. 
-- ------------------------------------------------------------------------------
drop view if exists incoming.track_dist cascade;
create view incoming.track_dist as
 select a.pk as pk,      -- Primaerschluessel Tracktabelle
 a.s as s,               -- Session Key fuer den Eingangsdatensatz
 a.n,                    -- Tracknumme jetzt 
 a.t,                    -- Zeit
 a.pln_rad,              -- Heading Radiant 
 a.pln_grd,              -- Heading Grad
 extract(epoch from b.t-a.t)   as dt, -- Zeit in Sekunden
 sqrt((b.x-a.x)^2+(b.y-a.y)^2) as dr  -- Distanz in Metern 
 from      incoming.track_stack as a 
 left join incoming.track_stack as b 
            on ( a.ix=(b.ix-1) ) 
 order by a.t;

-- ------------------------------------------------------------------
-- Tabelle der interpolierten Tracks
-- ------------------------------------------------------------------
drop table if exists incoming.tracks_hires cascade;
delete from geometry_columns where f_table_schema='incoming' and f_table_name='tracks_hires';
create table incoming.tracks_hires (
       ipl_pk serial primary key,
       trc_pk int,
       trc_pv int,
       trc_time timestamp with time zone,
       trc_num integer,
       session varchar(32),
       utm_x float8,
       utm_y float8,
       utm_z float8,
       pln_rad float8,       -- Heading Radiant 
       pln_grd float8       -- Heading Grad
);
select AddGeometryColumn('incoming','tracks_hires','utm_pos',32632,'POINT',3);


-- ------------------------------------------------------------------------------
-- Positionierungs Stack
-- ------------------------------------------------------------------------------
drop view if exists   incoming.image_pos_stack cascade;
create view incoming.image_pos_stack as
     select
          trc_pk as pk, trc_pv as pv,
     	  ROW_NUMBER() OVER (ORDER BY trc_time ASC) AS ix,
          trc.session as s, trc_num as n, trc_time as t, 
	  round((crs_expose/1E6)::NUMERIC,7) as tx,
     	  round(utm_x::NUMERIC,3) as x, 
	  round(utm_y::NUMERIC,3) as y, 
	  round(utm_z::NUMERIC,3) as z,
          pln_rad, pln_grd,
	  raw_file as file
	  from incoming.tracks_hires as trc
	  left join incoming.exif on trc_time=(exif_time - '01:01:06'::time) 
	  where raw_file is not null 
	  order by trc_time, trc_num;

-- ------------------------------------------------------------------------------
-- Richtungs-Geschwindigkeits-Stack
-- ------------------------------------------------------------------------------
drop view if exists incoming.image_dpos_stack cascade;
create view incoming.image_dpos_stack as
   select
          a.pk, a.pv,
          a.file as file,
          a.t, a.tx, a.s, a.n, a.ix as ix,
          a.x as x, a.y as y, a.z as z,
          a.pln_rad, a.pln_grd,
          b.x as x0, 
          b.y as y0, 
          b.z as z0, 
   	  b.x-a.x as dx, 
	  b.y-a.y as dy, 
	  b.z-a.z as dz,
	  (extract(epoch from b.t-a.t)::float8)::NUMERIC as dt,
	  round(sqrt( (b.x-a.x)^2 +(b.y-a.y)^2  +(b.z-a.z)^2)::NUMERIC,3) as dr
   from incoming.image_pos_stack as a
   left join incoming.image_pos_stack as b on (a.ix = b.ix-1 and a.n=b.n)
   where  (extract(epoch from b.t-a.t)::float8)>0
   order by t,n;


-- ------------------------------------------------------------------------------
-- Richtungs-Geschwindigkeits-Bild-Stack
-- ------------------------------------------------------------------------------
drop view if exists incoming.image_dpos_merge cascade;
create view incoming.image_dpos_merge as
  select 
    s as session,
    n as trc_num,
    t as tm_sample,
    x as utm_x,
    y as utm_y,
    z as utm_z,
    x+dx as utm_px,
    y+dy as utm_py,
    z+dz as utm_pz,
    dx , dy, dz, dr, dt,
    dr/dt as v,
    (tx*1000) as tm_expose,
    file as img_file,
    atan2(dy/dr,dx/dr) as trc_rad,
    -atan2(dy/dr,dx/dr)/pi()*180+90 as trc_grd,
    pln_rad, pln_grd
    -- trc_head as pln_head_grd
   from incoming.image_dpos_stack
   left join incoming.tracks on (t=trc_time)
   order by t;
   
-- ------------------------------------------------------------------------------
-- Bild zu Zeitzuordnung + technische Parameter wie Aufloesung....
-- ------------------------------------------------------------------------------
drop table if exists incoming.image_log cascade;
delete from geometry_columns
   where f_table_schema='incoming'
   and f_table_name='image_log';
create table  incoming.image_log (
 ilg_pk serial primary key,
 session varchar(32),
 trc_num  int,
 cam_type varchar(32) references incoming.cam(cam_type),
 tm_sample timestamp with time zone,
 tm_expose float8,
 img_file varchar(128),
 utm_x float8,
 utm_y float8,
 utm_z float8,
 utm_px float8,
 utm_py float8,
 utm_pz float8,
 dx float8,
 dy float8,
 dz float8,
 dr float8,
 dt float8,
 trc_rad  float8,
 trc_grd  float8,
 trc_ngrd float8,
 pln_rad  float8,
 pln_grd  float8,
 pln_ngrd float8,
 img_width  int,
 img_height int,
 world_x    float8,
 world_y    float8,
 res_x      float8,
 res_y      float8,
 delta_v    float8
);
select AddGeometryColumn('incoming','image_log','utm_ul' ,32632,'POINT',2);
select AddGeometryColumn('incoming','image_log','utm_ll' ,32632,'POINT',2);
select AddGeometryColumn('incoming','image_log','utm_ur' ,32632,'POINT',2);
select AddGeometryColumn('incoming','image_log','utm_lr' ,32632,'POINT',2);
select AddGeometryColumn('incoming','image_log','utm_ipos',32632,'POINT',3);
select AddGeometryColumn('incoming','image_log','utm_ppos',32632,'POINT',3);
select AddGeometryColumn('incoming','image_log','utm_dir',32632,'LINESTRING',3);
select AddGeometryColumn('incoming','image_log','utm_roi',32632,'POLYGON',2);

-- ------------------------------------------------------------------------------
-- Tabelle der Zaehldaten
-- ------------------------------------------------------------------
drop table if exists incoming.census;
delete from geometry_columns
   where f_table_schema='incoming'
   and f_table_name='census';
create table incoming.census(
 cns_pk serial primary key,
 session varchar(32),
 trc_num int,
 img_file varchar(128),
 taxon    varchar(128),
 gender   varchar(128),
 quality  int,
 behavior varchar(128),
 num      int,
 scol     int,
 srow     int,
 utm_x    float8,
 utm_y    float8
);
select AddGeometryColumn('incoming','census','utm_cpos' ,32632,'POINT',2);

-- ------------------------------------------------------------------------------
-- Tabelle der Bildsamples fuer Vogeldaten
-- ------------------------------------------------------------------------------
drop table if exists incoming.image_samples;
create table incoming.image_samples(
 isp_pk serial primary key,
 cns_ref int references incoming.census(cns_pk),
 session varchar(32),
 img_file varchar(128),
 size int);

-- ------------------------------------------------------------------------------
-- EOF
-- ------------------------------------------------------------------------------

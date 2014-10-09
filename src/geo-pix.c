// ===========================================================================
// Transformation von Welt auf Pixelkoordinaten GDAL Biliothek 
// (c) - 2013 A. Weidauer  alex.weidauer@huckfinn.de
// All rights reserved to A. Weidauer
// ===========================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gdal.h>
#include "alg.h"

// Aufraeumen als zentrale Anlaufpunkt 
void finalize(GDALDatasetH  hDataset) {
  if (hDataset) GDALClose(hDataset);
}

// Fehler Handhaben und Aufraeumen anschieben 
void handleError(const char * msg,
                 const char * detail, int num,
                 GDALDatasetH  hDataset) {
  fprintf(stderr,msg, detail);
  finalize(hDataset);
  exit(num);
}

// Transformation von Pixel auf Weltkoordianten
void calcPixelToWorld(double * trfm,
                     double col, double row,
                     double * x , double * y) {
  *x = trfm[0] + trfm[1] * col + trfm[2] * row;
  *y = trfm[3] + trfm[4] * col + trfm[5] * row;
}

// Transformation von Welt auf Pixelkoordianten 
int calcWorldToPixel(double * trfm,
                     double x, double y,
                     long * col , long * row) {

  //@MAXIMA:IN linsolve([x=a[0]+a[1]*c+a[2]*r, 
  //@MAXIMA:IN           y=a[3]+a[4]*c+a[5]*r], [r,c]);
  //@MAXIMA:OUT [r =  (a[1]*(a[3]-y)+a[4]*x-a[0]*a[4])/(a[2]*a[4]-a[1]*a[5]),
  //@MAXIMA:OUT  c = -(a[2]*(a[3]-y)+a[5]*x-a[0]*a[5])/(a[2]*a[4]-a[1]*a[5])]
  double div = (trfm[2]*trfm[4]-trfm[1]*trfm[5]);
  if (div<DBL_EPSILON*2) return 0;
  double dcol = -(trfm[2]*(trfm[3]-y)+trfm[5]*x-trfm[0]*trfm[5])/div;
  double drow =  (trfm[1]*(trfm[3]-y)+trfm[4]*x-trfm[0]*trfm[4])/div;
  *col = round(dcol); *row = round(drow);
  return 1;
}

// ===========================================================================
// Hauptprogramm
// ===========================================================================
int main(int argc, char* argv[])
{
  // Standart Fehler Template
  char * error_tmpl =
       "USAGE: %s GEOTIFF [-d] [-v] [-s \"|\"] -p geo_x geo_y [-p geo_x geo_y]\n";
  
  // Fehler-Template "nur einmal vewenden"
  char * error_tmpl_single = "Option '%s' nur einmal am Anfang verwenden!";

  //Ausgabe Hilfe
   if ( argc < 3) {
      fprintf(stderr, error_tmpl,argv[0]);
      exit(1);
    }  

   // Dateiname der Geotiff-Datei     
   char * pszFname = argv[1];

   // Parametercounter
   int i=1;

   // Ausgabeseparator und Separator wurde gesetzt
   char * pszSep = " "; int hasSep = 0;

   // Debug Modus
   int DEBUG=0; int hasDebug = 0;

   // Aktuelles CLI-Tag
   char * cur_arg = NULL;

   // CLI-Tags fuer X und Y parsen
   char * prm_x   = NULL;
   char * prm_y   = NULL;

   //  X und Y parsen Koordinate
   double x = 0;
   double y = 0;

   //  c und r - Spalte und Zeile im Bild
   long   c = 0;
   long   r = 0;

   // Datensatz Geotiff
   GDALDatasetH  hDataset;
   double        trfm[6];

   // Alle GDAL Treiber registrieren   
   GDALAllRegister();

   // Greetings
   printf("# TOOL: GEOTIFF WORLD TO PIXEL TRANSFORM ---------------------------\n");
   
   // Geotiff oeffnen
   printf("# FILE: %s\n",pszFname);
   hDataset = GDALOpen( pszFname, GA_ReadOnly );
   
   // Geotiff Fehler abfangen 
   if( hDataset == NULL ) {
     handleError("Datensatz %s kann nicht geoeffnet werden!\n",
                 pszFname, 10, hDataset);
   }

   // Transformation aus dem Geotiff holen und ausgeben
   if( GDALGetGeoTransform( hDataset, trfm ) == CE_None ) {
        printf("# TRANSFORM: \n");
        printf("#  X = %.6f + %.6f * COL + %.6f * ROW\n",
                   trfm[0], trfm[1], trfm[2] );
        printf("#  Y = %.6f + %.6f * COL + %.6f * ROW\n# EOF:\n",
                   trfm[3], trfm[4], trfm[5] );
   } else {
       handleError("Keine Transformation im TIFF vorhanden!\n","",10,hDataset);
   }  

    // Header drucken 
    printf("# HEAD: WORLD.X WORLD.Y IMAGE.COL IMAGE.ROW ERROR.X ERROR.Y -------\n");
   
   // Parameter Einlesen 
   while (i<argc-1) {

     // Aktuelles Tag lesen 
      cur_arg = argv[++i];
      if (DEBUG)
        printf("# ARG.NUM: %d ARG.VAL: %s ARG.PRM %s\n",
                i, cur_arg, argv[i+1] );
      // Vermeiden dass Debug 2x gelesen wird
      if ( strncmp(cur_arg,"-d",3)==0 && hasDebug ) {
         handleError(error_tmpl_single,"-d", 11, hDataset);
      }
      //  Debug setzen
      else if ( strncmp(cur_arg,"-d",3)==0 && !hasDebug ) {
        hasDebug=1;  DEBUG=1; 
                
      // Vermeiden dass der Separator 2x gelesen wird
      } else if ( strncmp(cur_arg,"-s",3)==0 && hasSep ) {
        handleError(error_tmpl_single, "-s", 11,hDataset);
      }
      // Einlesen des Separators -s
      else if ( strncmp(cur_arg,"-s",3)==0 && !hasSep ) {
        if ( i < argc) {
          pszSep = argv[i+1];
        } else {
          handleError(error_tmpl,argv[0],12, hDataset);
        }
        i++; hasSep=1;
      } // Ende Einlesen des Separators
      else
      //Einlesen von Punkten
      if ( strncmp(cur_arg,"-p",3)==0 ) {

          // Kommandozeile hat genug Argumente ?
          if ( i+2 < argc) {
            prm_x = argv[i+1];
            prm_y = argv[i+2];
          } else {
            handleError(error_tmpl,argv[0],10, hDataset);
          }

          // X-Koordinate scannen 
          if (! sscanf(prm_x,"%lf",&x) ) {
            handleError("Ungueltiger numerischer Wert fuer X: %s\n!",
                        prm_x,1000+i-1, hDataset);
          }

          // Y-Koordinate scannen 
          if (! sscanf(prm_y,"%lf",&y) ) {
            handleError("Ungueltiger numerischer Wert fuer Y: %s\n!",
                        prm_y,1000+i, hDataset);
          }

          // Kontrollausgabe
          if (DEBUG) printf("# PARM.POINT %s %s : DATA.POINT %f %f\n",
                            prm_x, prm_y, x, y);

          // Berechnung durchfuehren
          int res = calcWorldToPixel(trfm,x,y,&c,&r);
          
          if (res) {
            double nx=0.0,ny=0.0;

            // Variablen absoluter Fehler
            calcPixelToWorld(trfm,c,r,&nx,&ny);

            // Kontrollausgabe
            if (DEBUG)
              printf("# X:%f Y:%f C:%ld R:%ld ERRX:%f ERRY:%f\n",
                       x, y, c, r, x-nx, y-ny);

            //Datenausgabe mit Separator
            printf("%f%s%f%s%ld%s%ld%s%f%s%f\n",
                   x,pszSep,y,pszSep,c,pszSep,r,pszSep,x-nx,pszSep,y-ny);
          }
          // Parameterzaehler erhoehen 
          i+=2;
          
      } // EOF Option -p

      // Unbekannte Option
      else {
         handleError("Falscher Parameter %s\n",cur_arg,1000+i, hDataset);
      }
    }
    // Augabeende signailsieren
    printf("# EOF: ------------------------------------------------------------\n");

    // Handles schliessen
    finalize(hDataset);

    // OK alles fine
    exit(0);      
}
// --- EOF ----------------------------------------------------------------

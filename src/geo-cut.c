// =====================================================================
// Ausschneiden von Samplen aus einem Geotiff  
// (c) - 2013 A. Weidauer  alex.weidauer@huckfinn.de
// All rights reserved to A. Weidauer
// =====================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <gdal.h>
#include <cpl_conv.h>
#include <cpl_string.h>
#include "alg.h"

// -------------------------------------------------------------------
int main(int argc, char **argv)
{
	
   char *ERROR_PRM = "Ungueltiger numerischer Wert fuer %s: %s\n!";
	
   // Alle GDAL Treiber registrieren   
   GDALAllRegister();
	
   // Dateiname der Geotiff-Datei     
   const char *format = "GTiff";
   GDALDriverH h_drv = GDALGetDriverByName( format );
   if( h_drv == NULL ) {
	   error_exit(10,"Treiber %s nicht vorhanden!" ,format);
   }

   // Test ob Geotiffdateien erzeugt werden koennen
   char **test_meta;
   test_meta = GDALGetMetadata( h_drv, NULL );
   if( ! CSLFetchBoolean( test_meta, GDAL_DCAP_CREATE, FALSE ) ) {
	   error_exit(10,"Das Format %s kann nicht erzeugt werden" ,format);
	}
      
   // 3 Kommandozeilenparameter
   if (argc<6) {
	   error_exit(10,
	   "Fehlende Parameter\nUsage %s IN OUT EXT SZ ID X Y ID X Y ID X Y....!\n",
	   argv[0]);
   }
   
   // Eingabemuster einlesen
   char *ifile = argv[1];

   // Ausgabemuster einlesen
   char *ofile = argv[2];

   // zusammengesetzte Ausgabedatei 
   char cfile[512];

   // Dateierweiterung setzen
   char *ext   = argv[3];

   // Fenstergroesse
   int size = 64; 
   if (! sscanf(argv[4],"%d",&size) ) {
        error_exit(1000+3,ERROR_PRM,"SZ",argv[4]);
   }
   double trfm[] ={0,0,0,0,0,0};

   // Vektoren fuer die Positionen und ID
   int_vector_t id;
   int_vector_init(&id, 10);
   dbl_vector_t pos_x;
   dbl_vector_init(&pos_x, 10);
   dbl_vector_t pos_y;
   dbl_vector_init(&pos_y, 10);
   
   // Positionen einlesen 
   int a = 5; double dbl; int pk;
   while( a < argc-2 ) {

	   // X Koordinate parsen
       if (! sscanf(argv[a],"%d",&pk) ) {
          error_exit(1000+a,ERROR_PRM,"ID", argv[a]);
       }
	   int_vector_add(&id,pk);
       
	   // X Koordinate parsen
       if (! sscanf(argv[a+1],"%lf",&dbl) ) {
          error_exit(1000+a+1,ERROR_PRM,"X", argv[a+1]);
       }
	   dbl_vector_add(&pos_x,dbl);

	   // Y Koordinate parsen
       if (! sscanf(argv[a+2],"%lf",&dbl) ) {
          error_exit(1000+a+2,ERROR_PRM,"Y", argv[a+2]);
       }
	   dbl_vector_add(&pos_y,dbl);

	   a+=3;
   }
      
   // Alle GDAL Treiber registrieren   
   GDALAllRegister();
   
   // Geotiff oeffnen
   printf("# IN FILE:  %s\n", ifile);
   GDALDatasetH h_dset = GDALOpen( ifile, GA_ReadOnly);

   printf("# OUT FILE: %s\n", ofile);

   printf("# EXTENTION: %s\n", ext);
   
   // Transformation holen 
   if( GDALGetGeoTransform( h_dset, trfm ) == CE_None ) {
        printf("# TRANSFORM: \n");
        printf("#  X = %.6f + %.6f * COL + %.6f * ROW\n",
                   trfm[0], trfm[1], trfm[2] );
        printf("#  Y = %.6f + %.6f * COL + %.6f * ROW\n# EOF:\n",
                   trfm[3], trfm[4], trfm[5] );
   } else {
     error_exit(10, "Keine Transformation im TIFF vorhanden!\n");
   }  
   
   // Geotiff Fehler abfangen 
   if( h_dset == NULL ) {
     error_exit(10, "Datensatz %s kann nicht geoeffnet werden!\n",
                ifile);
   }

  // Bilddimensionen ermitteln 
  int img_width  = GDALGetRasterXSize( h_dset ); 
  int img_height = GDALGetRasterYSize( h_dset );
  int num_bands  = GDALGetRasterCount (h_dset );   

  // Bilddimensionen ermitteln 
  GDALRasterBandH h_band[num_bands];
  GDALDataType    h_type[num_bands];
  int             h_tsize[num_bands];

  for(int b=0 ; b<num_bands; b++) {
    h_band[b]  = GDALGetRasterBand( h_dset, b+1 );   
    h_type[b]  = GDALGetRasterDataType(h_band[b]);   
    h_tsize[b] = GDALGetDataTypeSize(h_type[b]);
  }  

  // Erzeuge Bildschnitte
  for (int c=0; c< pos_x.length; c++ ) {
	
	 // Welt zu Pixeltransformation
	 long icol = -1; long irow = -1;
     trfm_geo_pix(trfm, pos_x.data[c], pos_y.data[c],
                &icol , &irow);

	 // Dateinamen erzeugen
	 sprintf(cfile,"%s.%d%s",ofile, id.data[c],ext);

	 // Test ob das Schnittfenstrer passt
	 if (icol-size/2<=0 || 
	      irow-size/2<=0 || 
	      icol+size/2>=img_width || 
	      irow+size/2>=img_height){
		printf ("IGN %d %s\n",id.data[c],ofile);
		continue;
	 }		  	

	 // Schneiden
  	 printf ("ADD %d %s\n",id.data[c],ofile);
	      
	 int ioffs_col = icol-size/2;  
     int ioffs_row = irow-size/2; 
	
	 // Tiff-Datei anlegen 
     // char **options = NULL;
     GDALDatasetH h_dset_out = GDALCreate( h_drv, 
         cfile, size, size, num_bands, h_type[0], NULL);
	
	 // Iteration ueber alle Baender 
     for (int b=0; b< num_bands; b++) {

   	    // IO Buffer allozieren @todo static
	    void *io_buffer = CPLMalloc(h_tsize[0] * size * size);

		// Pixel lesen 
        GDALRasterIO( h_band[b], GF_Read, 
                      ioffs_col, ioffs_row, size, size, 
                      io_buffer, size, size, h_type[b], 0, 0 );
		// Pixel schreiben  
		GDALRasterBandH h_band_out = GDALGetRasterBand(h_dset_out, b+1);
		GDALRasterIO( h_band_out, GF_Write, 0, 0, size, size, 
                      io_buffer, size, size, h_type[b], 0, 0 );  
	    // IO Buffer freigeben
        CPLFree(io_buffer);
     }
     // resultierendes Tiff flushen und schliessen
     GDALClose( h_dset_out );         

  } // EOF Positions

  // Eingangsbild
  GDALClose( h_dset);         
  return 0;
}

// --- EOF -----------------------------------------------------------


// ==================================================================
// Berechnung der mittleren Affinen Transformation
// (c) - 2008 A. Weidauer  alex.weidauer@huckfinn.de
// All rights reserved to A. Weidauer
// ==================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alg.h"

//---------------------------------------------------------------------
/**
 * Transformation einer Koordinate
 * @param trfm Affine Transformation
 * @param x Quellkoordinate X
 * @param y Quellkoordinate Y
 * @param tx Zielkoordinate X
 * @param ty Zielkoordinate Y
 */
void trfm_calc(double *trfm, double x, double y, double *tx, double *ty) {
    *tx = trfm[0] * x + trfm[1] * y + trfm[2];
    *ty = trfm[3] * x + trfm[4] * y + trfm[5];
}

// ====================================================================
// Hauptprogramm
// ====================================================================

int main(int argc, char* argv[]) {

    // Standart Fehler Template
    char *msg_usage =
            "USAGE: %s [-h -d -s CHAR] -p x y -p x y -p x y [-p ...] -t x y [-t x y .....]\n";

    // Fehler-Template "nur einmal vewenden"
    char *error_tmpl_single = "Option '%s' nur einmal am Anfang verwenden!";

    // Numerischer Fehler
    char *error_tmpl_num = "Ungueltiger numerischer Wert fuer %s: %s\n!";

	// Ausgabedekorationen
    char *FOOTER = 
    "# EOF -----------------------------------------------------------------\n";
    char *TITLE =
    "# TOOL: GPC TRANSFORM -------------------------------------------------\n";      
    char *HEADER =
    "# HEAD: SRC.X SRC.Y TRFM.X TRFM.Y ERROR.X ERROR.Y ---------------------\n";
    
    // Ausgabeseparator und Separator wurde gesetzt
    char *sep = " "; int parse_sep = 0;

    // Debug Modus
    int DEBUG = 0; int parse_debug = 0;

    // Aktuelles CLI-Tag
    char *cur_arg = NULL;

    // Passpunktvektoren  X und Y, Quelle und Ziel
    dbl_vector_t pass_src_x;
    dbl_vector_init(&pass_src_x, 3);
    dbl_vector_t pass_src_y;
    dbl_vector_init(&pass_src_y, 3);
    dbl_vector_t pass_dst_x;
    dbl_vector_init(&pass_dst_x, 3);
    dbl_vector_t pass_dst_y;
    dbl_vector_init(&pass_dst_y, 3);

    // Transformation
    double trfm[6] = {0, 0, 0, 0, 0, 0};
    // Ruecktransformation
    double itrfm[6] = {0, 0, 0, 0, 0, 0};

    // Fuer Berechnungen und CLI parsen
    // Quelldatum
    double x = 0;
    double y = 0;
    // Transformiertes Datum
    double tx = 0;
    double ty = 0;
    // Invertransformiertes Datum
    double itx = 0;
    double ity = 0;

    // Erstes Koordinatendatum wurde gelesen
    int parse_coord = 0;

    // Variablen fuer den inversen Modus
    int inverse = 0;
    int parse_inverse = 0;

    // Parametercounter
    int cnt_arg = 0;

    // Greetings
    printf(TITLE);

    // Austeigenfalss keine Parameter
    if (argc < 1) {
        fprintf(stderr, msg_usage, argv[0]);
        exit(1);
    }

    // Parameter Einlesen 
    while (cnt_arg < argc - 1) {

        // Aktuelles Tag lesen 
        cur_arg = argv[++cnt_arg];
        
        // Kontrollausgabe Parameter
        if (DEBUG) {
            printf("# ARG.NUM: %d ARG.VAL: %s ARG.PRM %s\n",
                cnt_arg, cur_arg, argv[cnt_arg + 1]);
        }
        
        // Hilfe ausgeben 
        if (strncmp(cur_arg, "-h", 3) == 0 || strncmp(cur_arg, "--help", 7) == 0) {
            printf("Berechnung von affinen Transformationen und");
            printf("Koordinatentransformationenmit Hilfe eines Satzen");
            printf("von Passpukten.\n\n");
            printf(msg_usage, "gpc_trfm");
            printf("\nAusgabeformat:\n");
            printf("# ... Metadaten\n");
            printf("src.x src.x trfm.x trfm.y err.x err.y\n");
            printf("...\n\n");
            printf("Schalter:\n");
            printf(" -h Hilfe anzeigen\n -d DEBUG AN\n -i invers rechnen\n");
            printf(" -s CHAR Separator\n");
            printf(" -p sx sy dx dy -p sx sy dx dy ... Passpunkte\n");
            printf("    sx sy  Quellkoordinate\n");
            printf("    dx dy  Zielkoordinate\n");
            printf(" -c x y -c x y ...Koordinate die transformiert wird\n\n");
            printf("Beispiele:\n");
            printf("  gpc_trfm -s \"|\" -p 1 0 10 0 -p 0 1 0 10 -p 1 1 10 10 -t 1 1 -t 1 0\n");
            printf("  gpc_trfm -i -p 1 0 10 0 -p 0 1 0 10 -p 1 1 10 10 -t 1 1 -t 1 0\n\n");
            printf("(c) - 2008 Alexander Weidauer; alex.weidauer@huckfinn.de\n\n");
            exit(0);
            
        // Vermeiden das Debug 2x gesetzt wird    
        } else if (strncmp(cur_arg, "-d", 3) == 0 && parse_debug) {
            error_exit(11,error_tmpl_single, "-d");
        }
        
        //  Debug setzen
        else if (strncmp(cur_arg, "-d", 3) == 0 && !parse_debug) {
            parse_debug = 1;
            DEBUG = 1;
        }
        
        // Vermeiden dass Inverse 2x gelesen wird
        else if (strncmp(cur_arg, "-i", 3) == 0 && parse_inverse) {
            error_exit(11,error_tmpl_single, "-i");
        }
        
        //  Inverse Transformation setzen
        else if (strncmp(cur_arg, "-i", 3) == 0 && !parse_inverse) {
            parse_inverse = 1;
            inverse = 1;

         // Vermeiden dass der Separator 2x gelesen wird
        } else if (strncmp(cur_arg, "-s", 3) == 0 && parse_sep) {
            error_exit(11, error_tmpl_single, "-s");
        }
        
        // Einlesen des Separators -s
        else if (strncmp(cur_arg, "-s", 3) == 0 && !parse_sep) {
            if (cnt_arg < argc) {
                sep = argv[cnt_arg + 1];
            } else {
                error_exit(12, msg_usage, argv[0]);
            }
            cnt_arg++;
            parse_sep = 1;
        }// Ende Einlesen des Separators

            // Passpunkte im Koordinatenmodus nicht moeglich
        else if (strncmp(cur_arg, "-p", 3) == 0 && parse_coord) {
            error_exit(1000 + cnt_arg,
            "Mischen von Passpunkten und Daten nicht moeglich!");
        } else
            //Einlesen von Punkten
            if (strncmp(cur_arg, "-p", 3) == 0 && !parse_coord) {

            // Kommandozeile hat genug Argumente ?
            if (cnt_arg + 4 >= argc) {
                error_exit(10, msg_usage, argv[0]);
            }

            // X-Koordinate scannen 
            if (!sscanf(argv[cnt_arg + 1], "%lf", &x)) {
              error_exit(1000 + cnt_arg + 1,
                error_tmpl_num,"SRC.X",argv[cnt_arg + 1]);
            }

            // Y-Koordinate scannen 
            if (!sscanf(argv[cnt_arg + 2], "%lf", &y)) {
              error_exit(1000 + cnt_arg + 2,
                error_tmpl_num,"SRC.Y",argv[cnt_arg + 2]);
            }

            // Kontrollausgabe
            if (DEBUG) printf("# CLI.POINT %s %s : PASS.SRC %f %f\n",
                    argv[cnt_arg + 1], argv[cnt_arg + 2], x, y);

            dbl_vector_add(&pass_src_x, x);
            dbl_vector_add(&pass_src_y, y);

            // X-Koordinate scannen 
            if (!sscanf(argv[cnt_arg+3], "%lf", &x)) {
              error_exit(1000 + cnt_arg + 3,
                error_tmpl_num,"DST.Y",argv[cnt_arg + 3]);
            }
            
            if (!sscanf(argv[cnt_arg+4], "%lf", &y)) {
              error_exit(1000 + cnt_arg + 3,
                error_tmpl_num,"DST.Y",argv[cnt_arg + 4]);
            }

            // Kontrollausgabe
            if (DEBUG) printf("# CLI.POINT %s %s : PASS.DST %f %f\n",
                    argv[cnt_arg + 3], argv[cnt_arg + 4], x, y);

            dbl_vector_add(&pass_dst_x, x);
            dbl_vector_add(&pass_dst_y, y);

            // Parameterzaehler erhoehen 
            cnt_arg += 4;

        }// EOF Option -p
        
        // Einlesen von zu transformierenden Daten
        else if (strncmp(cur_arg, "-t", 3) == 0) {
            
            // Transformationen erzuegen falls in den Koordinatenmodus 
            // geschaltet wurde
            if (!parse_coord) {
                
                // Hintransformation
                trfm_create(&pass_src_x, &pass_src_y,
                        &pass_dst_x, &pass_dst_y, trfm);

                // Ruecktransformation
                trfm_create(&pass_dst_x, &pass_dst_y,
                        &pass_src_x, &pass_src_y, itrfm);
                
                //Kontrollausgabe
                printf("# TRFM:  X = %f * x + %f * y + %f\n",
                 trfm[0], trfm[1], trfm[2]);
                printf("# TRFM:  Y = %f * x + %f * y + %f\n",
                 trfm[3], trfm[4], trfm[5]);
                printf("# ITRFM: X = %f * x + %f * y + %f\n",
                 itrfm[0], itrfm[1], itrfm[2]);
                printf("# ITRFM: Y = %f * x + %f * y + %f\n",
                 itrfm[3], itrfm[4], itrfm[5]);

                printf(HEADER);
                
                // Passpunktvektoren freigeben 
                dbl_vector_free(&pass_src_x);
                dbl_vector_free(&pass_src_y);
                dbl_vector_free(&pass_dst_x);
                dbl_vector_free(&pass_dst_y);
            }
            
            // Koordinatenmodus anschalten
            parse_coord = 1;

            // Kommandozeile hat genug Argumente ?
            if (cnt_arg + 2 >= argc) {
                error_exit(10, msg_usage, argv[0]);
            }

            // X-Koordinate scannen 
            if (!sscanf(argv[cnt_arg + 1], "%lf", &x)) {
              error_exit(1000 + cnt_arg + 1,
                error_tmpl_num,"TRFM.X",argv[cnt_arg + 1]);
            }

            // Y-Koordinate scannen 
            if (!sscanf(argv[cnt_arg + 2], "%lf", &y)) {
              error_exit(1000 + cnt_arg + 1,
                error_tmpl_num,"TRFM.Y",argv[cnt_arg + 2]);
            }

            // Berechnung der Koordinaten
            if (inverse) {
                trfm_calc(itrfm, x, y, &tx, &ty);
                trfm_calc(trfm, tx, ty, &itx, &ity);
            } else {
                trfm_calc(trfm, x, y, &tx, &ty);
                trfm_calc(itrfm, tx, ty, &itx, &ity);
            }
            
            // Ausgabe 
            printf("%f%s%f%s%f%s%f%s%f%s%f\n", 
                    x, sep, y, sep,
                    tx, sep, ty, sep,
                    x - itx, sep, y - ity);

            // Parameterzahler um 2 Positionen erhoehen
            cnt_arg += 2;
        }            // Unbekannte Option

        // Schalter nicht gefunden
        else {
            error_exit(1000 + cnt_arg,
            "Falscher Parameter %s\n", cur_arg);
        }
    }
    printf(FOOTER);
    exit(0);
}
// EOF--------------------------------------------------------------------------

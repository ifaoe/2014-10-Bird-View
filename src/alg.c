// =====================================================================
// Serviceroutinen
// (c) - 2013 A. Weidauer  alex.weidauer@huckfinn.de
// All rights reserved to A. Weidauer
// =====================================================================
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_linalg.h>
#include "alg.h"

// ---------------------------------------------------------------
/**
 * Imfehlerfall verlassen
 * @param code Exit code
 * @param message Fehlernachricht Template
 * @param ... Parameter String
 */
void error_exit(int code, const char *message, ...) {
    va_list arglist;
    va_start(arglist,message);
    vfprintf(stderr, message, arglist);
    va_end(arglist);
    exit(code);
}

// ---------------------------------------------------------------
/** Berechnet eine Transformation mit gegebenen 
 *  Quell- und Zielkoordinaten so dass der mittlere quadratische Fehler 
 *  minimal wird.
 *  @param src_x - Der Vektor der X-Koodianten des Quellkoordinatensystems
 *  @param src_y - Der Vektor der Y-Koodianten des Quellkoordinatensystems
 *  @param dst_x - Der Vektor der X-Koodianten des Zielkoordinatensystems
 *  @param dst_y - Der Vektor der Y-Koodianten des Zielkoordinatensystems
 *  @param result - Verktor vom Typ double[6] der die Transformation haelt
 *  @returns 1 falls erfolgreich 
 *          -1 falls zu wenig Passpunkte, 
 *          -2 Vektoren nich gleichmaechtig
 *  falls die Koodinatenvekoren nicht die gleiche Laenge besitzen oder 
 *  weniger als 3 Koordinateneintraege enthalten.
 */
int trfm_create(dbl_vector_t *src_x,
        dbl_vector_t *src_y,
        dbl_vector_t *dst_x,
        dbl_vector_t *dst_y,
        double *result) {

    // Testen ob die Vektoren genuegend Werte besitzen
    if (src_x->length < 3) return -1;

    // Testen ob die Vektoren alle gleich lang sind
    if (src_x->length != src_y->length ||
            dst_x->length != dst_y->length ||
            src_x->length != dst_x->length ||
            src_y->length != dst_y->length)
        return -2; 

    // Gleichungssyteme initalisieren 
    gsl_matrix *M = gsl_matrix_alloc(3, 3);
    gsl_vector *B1 = gsl_vector_alloc(3);
    gsl_vector *B2 = gsl_vector_alloc(3);
    gsl_vector *CX = gsl_vector_alloc(3);
    gsl_vector *CY = gsl_vector_alloc(3);
    
    // Nullen eintragen
    gsl_matrix_set_zero(M);
    gsl_vector_set_zero(B1);
    gsl_vector_set_zero(B2);
    gsl_vector_set_zero(CX);
    gsl_vector_set_zero(CY);

    // Gleichungsystem aufbauen
    for (int i = 0; i < src_x->length; i++) {

        // Normalform der Quellen 1 ..Summe ueber alle
        M->data[0 * M->tda + 0] += src_x->data[i] * src_x->data[i];
        M->data[0 * M->tda + 1] += src_x->data[i] * src_y->data[i];
        M->data[0 * M->tda + 2] += src_x->data[i];

        // Normalform der Quellen 2 ..Summe ueber alle
        M->data[1 * M->tda + 0] += src_x->data[i] * src_y->data[i];
        M->data[1 * M->tda + 1] += src_y->data[i] * src_y->data[i];
        M->data[1 * M->tda + 2] += src_y->data[i];

        // Mischterme Quelle- Zeilkoordinaten X ..Summe ueber alle
        B1->data[0 * B1->stride] += dst_x->data[i] * src_x->data[i];
        B1->data[1 * B1->stride] += dst_x->data[i] * src_y->data[i];
        B1->data[2 * B1->stride] += dst_x->data[i];

        // Mischterme Quelle- Zeilkoordinaten Y ..Summe ueber alle
        B2->data[0 * B2->stride] += dst_y->data[i] * src_x->data[i];
        B2->data[1 * B2->stride] += dst_y->data[i] * src_y->data[i];
        B2->data[2 * B2->stride] += dst_y->data[i];
    }

    // Restliche Terme der Normalform setzen
    gsl_matrix_set(M, 2, 2, src_x->length);
    gsl_matrix_set(M, 2, 1, gsl_matrix_get(M, 1, 2));
    gsl_matrix_set(M, 2, 0, gsl_matrix_get(M, 0, 2));

    // Gleichungsysteme loesen 
    int s = 0;
    gsl_permutation * P = gsl_permutation_alloc(3);
    gsl_linalg_LU_decomp(M, P, &s);
    
    gsl_linalg_LU_solve(M, P, B1, CX);
    gsl_linalg_LU_solve(M, P, B2, CY);

    // Vektoren und Matrizen des Gleichungssytems freigeben 
    gsl_matrix_free(M);
    gsl_vector_free(B1);
    gsl_vector_free(B2);
    gsl_permutation_free(P);

    // Loesungsvektoren in das Resultat schreiben.
    for (int i = 0; i < 3; ++i) {
        result[i] = gsl_vector_get(CX, i);
        result[i + 3] = gsl_vector_get(CY, i);
    }
    // Loesungsvektoren freigeben
    gsl_vector_free(CX);
    gsl_vector_free(CY);
}

// -------------------------------------------------------------------
/** 
 * Datei die Extention wegnehmen
 * @param txt Text mit Endung
 * @return Text ohne Dateiendung
 */
char *strip_ext(char *txt) {
	size_t len = strlen(txt);
	if (len==0) return(txt);
	for (int l=len; l>0; l--) {
		if (txt[l]=='.') {
			txt[l] = '\0';
			return(txt);
		}
	}
	return(txt);
}
// -------------------------------------------------------------------
/**
 * Transformation von Pixel auf Weltkoordianten
 * @param col Spalte im Bild
 * @param row Zeile im Bild
 * @param x X-Koordinate globales Koordsys.
 * @param y Y-Koordinate globales Koordsys.
 */
void trfm_pix_geo(double *trfm,
                  double col, double row,
                  double *x , double *y) {
  *x = trfm[0] + trfm[1] * col + trfm[2] * row;
  *y = trfm[3] + trfm[4] * col + trfm[5] * row;
}


// -------------------------------------------------------------------
/**
 * Transformation von Welt auf Pixelkoordianten 
 * @param trfm Transformation
 * @param x X-Koordinate globales Koordsys.
 * @param y Y-Koordinate globales Koordsys.
 * @param col Spalte im Bild
 * @param row Zeile im Bild
 * @return true falls Berechnung OK
 */
int trfm_geo_pix(double *trfm,
                double x, double y,
                long *col , long *row) {

  //@MAXIMA:IN linsolve([x=a[0]+a[1]*c+a[2]*r, 
  //@MAXIMA:IN           y=a[3]+a[4]*c+a[5]*r], [r,c]);
  //@MAXIMA:OUT [r =  (a[1]*(a[3]-y)+a[4]*x-a[0]*a[4])/
  //@MAXIMA:OUT            				(a[2]*a[4]-a[1]*a[5]),
  //@MAXIMA:OUT  c = -(a[2]*(a[3]-y)+a[5]*x-a[0]*a[5])/
  //@MAXIMA:OUT            				(a[2]*a[4]-a[1]*a[5])]
  double div = (trfm[2]*trfm[4]-trfm[1]*trfm[5]);
  if (div < DBL_EPSILON * 2) return 0;
  double dcol = -(trfm[2]*(trfm[3]-y)+trfm[5]*x-trfm[0]*trfm[5])/div;
  double drow =  (trfm[1]*(trfm[3]-y)+trfm[4]*x-trfm[0]*trfm[4])/div;
  *col = round(dcol); *row = round(drow);
  return 1;
}



// ---------------------------------------------------------------
/**
 *  Dynamischer double Vektor Initialisierung
 * @param vec der Vektor
 * @param size Initiale Groesse
 */

void dbl_vector_init(dbl_vector_t *vec, size_t size) {
    vec->data = (double *) malloc(size * sizeof (double));
    vec->length = 0;
    vec->mem_size = size;
}

// ---------------------------------------------------------------
/**
 * Dynamischer double Vektor Wert hinzufuegen
 * @param vec 
 * @param element
 */
void dbl_vector_add(dbl_vector_t *vec, double element) {
    if (vec->length == vec->mem_size) {
        vec->mem_size += vec->mem_size;
        vec->data = (double *) realloc(vec->data, 
        vec->mem_size * sizeof (double));
    }
    vec->data[vec->length++] = element;
}

// ---------------------------------------------------------------
/**
 * Dynamischer double Vektor freigeben 
 * @param vec Datenvektor
 */
void dbl_vector_free(dbl_vector_t *vec) {
    free(vec->data);
    vec->data = NULL;
    vec->length = vec->mem_size = 0;
}

// ---------------------------------------------------------------
/**
 * Dynamischer int Vektor Initialisierung
 * @param vec der Vektor
 * @param size Initiale Groesse
 */
void int_vector_init(int_vector_t *vec, size_t size) {
    vec->data = (int *) malloc(size * sizeof (int));
    vec->length = 0;
    vec->mem_size = size;
}

// ---------------------------------------------------------------
/**
 * Dynamischer int Vektor Wert hinzufuegen
 * @param vec 
 * @param element
 */
void int_vector_add(int_vector_t *vec, int element) {
    if (vec->length == vec->mem_size) {
        vec->mem_size += vec->mem_size;
        vec->data = (int *) realloc(vec->data, 
        vec->mem_size * sizeof (int));
    }
    vec->data[vec->length++] = element;
}

// ---------------------------------------------------------------
/**
 * Dynamischer int Vektor freigeben 
 * @param vec Datenvektor
 */
void int_vector_free(int_vector_t *vec) {
    free(vec->data);
    vec->data = NULL;
    vec->length = vec->mem_size = 0;
}


// --- EOF ---------------------------------------------------------

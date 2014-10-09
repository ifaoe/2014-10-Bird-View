#ifndef AWE_ALG_H
#define AWE_ALG_H

// =====================================================================
// Serviceroutinen
// (c) - 2013 A. Weidauer  alex.weidauer@huckfinn.de
// All rights reserved to A. Weidauer
// =====================================================================

// Maschinen Null DOUBLE
#define DBL_EPSILON 2.2204460492503131e-16 
#define ERR_TRFM_SIZE_EQUAL 3010;
#define ERR_TRFM_3_POINTS   3020;

// ===========================================================================
/**
 * Dynamischer double Vektor
 * array das Datenablage
 */
typedef struct {
    double * data;
    size_t length;
    size_t mem_size;
} dbl_vector_t;

// ---------------------------------------------------------------
/**
 * Dynamischer int Vektor
 * array das Datenablage
 */
typedef struct {
    int * data;
    size_t length;
    size_t mem_size;
} int_vector_t;

// =================================================================
/**
 * Imfehlerfall verlassen
 * @param code Exit code
 * @param message Fehlernachricht Template
 * @param ... Parameter String
 */
void error_exit(int code, const char *message, ...);

// ---------------------------------------------------------------
/**
 *  Dynamischer double Vektor Initialisierung
 * @param vec der Vektor
 * @param size Initiale Groesse
 */
void dbl_vector_init(dbl_vector_t *vec, size_t size);

// ---------------------------------------------------------------
/**
 * Dynamischer double Vektor Wert hinzufuegen
 * @param vec 
 * @param element
 */
void dbl_vector_add(dbl_vector_t *vec, double element);

// ---------------------------------------------------------------
/**
 * Dynamischer double Vektor freigeben 
 * @param vec Datenvektor
 */
void dbl_vector_free(dbl_vector_t *vec);

// -------------------------------------------------------------------
/**
 * Dynamischer int Vektor Initialisierung
 * @param vec der Vektor
 * @param size Initiale Groesse
 */
void int_vector_init(int_vector_t *vec, size_t size);

// ---------------------------------------------------------------
/**
 * Dynamischer int Vektor Wert hinzufuegen
 * @param vec 
 * @param element
 */
void int_vector_add(int_vector_t *vec, int element);

// ---------------------------------------------------------------
/**
 * Dynamischer int Vektor freigeben 
 * @param vec Datenvektor
 */
void int_vector_free(int_vector_t *vec);

// -------------------------------------------------------------------
/**
 * Transformation von Pixel auf Weltkoordianten
 * @param col Spalte im Bild
 * @param row Zeile im Bild
 * @param x X-Koordinate globales Koordsys.
 * @param y Y-Koordinate globales Koordsys.
 */
void trfm_pix_geo(double *trfm, double col, double row,
                  double *x , double *y);

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
int trfm_geo_pix(double *trfm, double x, double y,
                long *col , long* row);

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
 *  @exit Fehler
 *  falls die Koodinatenvekoren nicht die gleiche Laenge besitzen oder 
 *  weniger als 3 Koordinateneintraege enthalten.
 */
int trfm_create(dbl_vector_t *src_x,  dbl_vector_t *src_y,
				dbl_vector_t *dst_x,  dbl_vector_t *dst_y,
                double *result);


#endif

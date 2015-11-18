#include <math.h>

/* include postgres headers */
#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "utils/builtins.h"

/* include wcslib headers */
#include <wcshdr.h>
#include <wcsfix.h>
#include <wcs.h>
#include <getwcstab.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

/** 
 * convert C string to text pointer.
 */
#define GET_TEXT(cstrp) \
   DatumGetTextP(DirectFunctionCall1(textin, CStringGetDatum(cstrp)))

/** 
 * convert text pointer to C string.
 */

#define GET_STR(textp)  \
   DatumGetCString(DirectFunctionCall1(textout, PointerGetDatum(textp)))

/* Declare exported functions */
PG_FUNCTION_INFO_V1(angdist);
PG_FUNCTION_INFO_V1(onchip);
PG_FUNCTION_INFO_V1(ad2string);

int sphdpa(int nfield, double lng0, double lat0, double lng[], double lat[], double *dist, double *pa);

Datum ad2string(PG_FUNCTION_ARGS) {
	int clen = 30;
	double a, d;
	int rah, ram, decg, decm;
	double ras, decs;
	char buffer[clen];
	
	a = PG_GETARG_FLOAT8(0);
	d = PG_GETARG_FLOAT8(1);

	a = a/15.0;
	rah = (int) a;
	ram = (int) ((a-rah)*60.0);
	ras = ((a - rah)*60.0-ram)*60.0;
 
	decg = (int) d;
	decm = (int) ((fabs(d) - abs(decg))*60.0);
	decs = ((fabs(d) - abs(decg))*60.0 - decm)*60.0;
	
	sprintf(buffer, "%02d:%02d:%05.2f %+02d:%02d:%05.2f", rah, ram, ras, decg, decm, decs);
	
	
	PG_RETURN_TEXT_P(cstring_to_text(buffer));
	
}

Datum angdist(PG_FUNCTION_ARGS) {
	int nfield=1;
	double lat[1], lat0, lng[1], lng0, dist, pa;

	if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2) || PG_ARGISNULL(3)) {
		PG_RETURN_NULL();
	}

	lng0   = PG_GETARG_FLOAT8(0);
	lat0   = PG_GETARG_FLOAT8(1);
	lng[0] = PG_GETARG_FLOAT8(2);
	lat[0] = PG_GETARG_FLOAT8(3);

	sphdpa(nfield, lng0, lat0, lng, lat, &dist, &pa);

	PG_RETURN_FLOAT8(dist);
}

Datum onchip(PG_FUNCTION_ARGS) {
	int i,j, res;
	int naxis=2, naxis1, naxis2;
	int stat;
	double ra, dec, world[2];
	double pixcrd[2], imgcrd[2], phi, theta;

	double *cdij;
	double crval[2], crpix[2], cd[2][2], pv21=0, pv23=0, pv25=0;
	char *ctype1, *ctype2;
	char ctype[2][9] = {"RA---TAN", "DEC--TAN"};
	char *zpn="ZPN";
	struct wcsprm *wcs;
	struct pvcard PV[3];

	ra = PG_GETARG_FLOAT8(0);
	dec = PG_GETARG_FLOAT8(1);
	naxis1 = PG_GETARG_INT32(2);
	naxis2 = PG_GETARG_INT32(3);
	ctype1 = GET_STR(PG_GETARG_CSTRING(4));
	ctype2 = GET_STR(PG_GETARG_CSTRING(5));
	crval[0] = PG_GETARG_FLOAT8(6);
	crval[1] = PG_GETARG_FLOAT8(7);
	crpix[0] = PG_GETARG_FLOAT8(8);
	crpix[1] = PG_GETARG_FLOAT8(9);
	cd[0][0] = PG_GETARG_FLOAT8(10);
	cd[0][1] = PG_GETARG_FLOAT8(11);
	cd[1][0] = PG_GETARG_FLOAT8(12);
	cd[1][1] = PG_GETARG_FLOAT8(13);
	pv21 = PG_GETARG_FLOAT8(14);
	pv23 = PG_GETARG_FLOAT8(15);
	pv25 = PG_GETARG_FLOAT8(16);

	if (strstr(zpn, ctype1)) {
		pv21 = PG_GETARG_FLOAT8(14);
		pv23 = PG_GETARG_FLOAT8(15);
		pv25 = PG_GETARG_FLOAT8(16);
	}

	/*
	if PG_ARGISNULL(14) {
		pv21 = 0.0;
	} else {
		pv21 = PG_GETARG_FLOAT8(14);
	}

	if PG_ARGISNULL(15) {
		pv23 = 0.0;
	} else {
		pv23 = PG_GETARG_FLOAT8(15);
	}

	if PG_ARGISNULL(16) {
		pv25 = 0.0;
	} else {
		pv25 = PG_GETARG_FLOAT8(16);
	}
	*/

	/* initialize wcs structure */
	wcs = malloc(sizeof(struct wcsprm));
	wcs->flag = -1;
	wcsini(1, naxis, wcs);

	for (i = 0; i < naxis; i++) {
    		wcs->crpix[i] = crpix[i];
		wcs->crval[i] = crval[i];
		wcs->cdelt[i] = 1.0;
  	}

	/* using cd matrix representation */
	wcs->altlin |= 2;
	cdij = wcs->cd;
	for (i = 0; i < naxis; i++) {
		for (j = 0; j < naxis; j++) {
			*(cdij++) = cd[i][j];
		}
	}

	strcpy(ctype[0], ctype1);
	strcpy(ctype[1], ctype2);
	for (i = 0; i < naxis; i++) {
		strcpy(wcs->ctype[i], &ctype[i][0]);
	}

	wcs->npv=0;

	if (PG_ARGISNULL(14)) {
		wcs->npv = 0;
	} else {
		PV[0].i = 2;
		PV[0].m = 1;
		PV[0].value = pv21;
                PV[1].i = 2;
                PV[1].m = 3;
                PV[1].value = pv23;
                PV[2].i = 2;
                PV[2].m = 5;
                PV[2].value = pv25;
		
		wcs->npv = 3;
		for (i = 0; i < 3; i++) {
			wcs->pv[i] = PV[i];
		}
	}

	wcsset(wcs);

	world[wcs->lng] = ra;
	world[wcs->lat] = dec;

	res = wcss2p(wcs, 1, 0, world, &phi, &theta, imgcrd, pixcrd, &stat);

	ereport( INFO,
		( errcode( ERRCODE_SUCCESSFUL_COMPLETION ),
		errmsg( "%f %f -> %f %f %d  : %s %s %f %f %f \n", world[0], world[1], pixcrd[0], pixcrd[1], res, ctype1, ctype2, pv21, pv23, pv25   )));
	

	if (pixcrd[0]>0 & pixcrd[0]<=naxis1 & pixcrd[1]>0 & pixcrd[1]<=naxis2) {
		res=1;
	} else {
		res=0;
	}

	PG_RETURN_BOOL(res);
	
}

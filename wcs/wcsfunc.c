
/* include postgres headers */
#include "postgres.h"
#include <string.h>
#include "fmgr.h"

/* include wcslib headers */
#include <wcshdr.h>
#include <wcsfix.h>
#include <wcs.h>
#include <getwcstab.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

/* Declare exported functions */
PG_FUNCTION_INFO_V1(angdist);
PG_FUNCTION_INFO_V1(onchip);

int sphdpa(int nfield, double lng0, double lat0, double lng[], double lat[], double *dist, double *pa);

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
	struct wcsprm *wcs;

	ra = PG_GETARG_FLOAT8(0);
	dec = PG_GETARG_FLOAT8(1);
	naxis1 = PG_GETARG_INT32(2);
	naxis2 = PG_GETARG_INT32(3);
	ctype1 = PG_GETARG_CSTRING(4);
	ctype2 = PG_GETARG_CSTRING(5);
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

	for (i = 0; i < naxis; i++) {
		strcpy(wcs->ctype[i], &ctype[i][0]);
	}

	wcs->npv = 0;

	wcsset(wcs);

	world[wcs->lng] = ra;
	world[wcs->lat] = dec;

	res = wcss2p(wcs, 1, 0, world, &phi, &theta, imgcrd, pixcrd, &stat);

	ereport( INFO,
		( errcode( ERRCODE_SUCCESSFUL_COMPLETION ),
		errmsg( "%f %f -> %f %f %d   : %s \n", world[0], world[1], pixcrd[0], pixcrd[1], res, ctype1   )));
	

	if (pixcrd[0]>0 & pixcrd[0]<=naxis1 & pixcrd[1]>0 & pixcrd[1]<=naxis2) {
		res=1;
	} else {
		res=0;
	}

	PG_RETURN_BOOL(res);
	
}

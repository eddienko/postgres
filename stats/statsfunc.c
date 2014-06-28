// See: https://github.com/ringerc/scrapcode/blob/master/postgresql/array_sum/intarrayadd.c

/* include postgres headers */
#include "postgres.h"
#include <string.h>
#include "fmgr.h"
#include "utils/array.h"
#include <gsl/gsl_statistics.h>
#include "math.h"

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

/* Declare exported functions */
PG_FUNCTION_INFO_V1(array_mad);
PG_FUNCTION_INFO_V1(array_quantile);


Datum array_mad(PG_FUNCTION_ARGS) {
	// The formal PostgreSQL array object
	ArrayType *array;

	// The array element type
	Oid arrayElementType;

	// The array element type width
	int16 arrayElementTypeWidth;

	// The array element type "is passed by value" flags (not used, should always be true)
	bool arrayElementTypeByValue;

	// The array element type alignment codes (not used)
	char arrayElementTypeAlignmentCode;

	// The array contents, as PostgreSQL "datum" objects
	Datum *arrayContent;

	// List of "is null" flags for the array contents
	bool *arrayNullFlags;

	// The size of each array
	int arrayLength;

	int i,j, nelem;
	double median, mad;
	double *inarray;

	if (PG_ARGISNULL(0)) 
		ereport(ERROR, (errmsg("Null arrays not accepted")));

	// Get array from input
	array = PG_GETARG_ARRAYTYPE_P(0);

	if (ARR_NDIM(array) != 1) 
		ereport(ERROR, (errmsg("One-dimesional arrays are required")));

	if (array_contains_nulls(array)) 
		ereport(ERROR, (errmsg("Array contains null elements")));

	arrayLength = (ARR_DIMS(array))[0];
	arrayElementType = ARR_ELEMTYPE(array);
	get_typlenbyvalalign(arrayElementType, &arrayElementTypeWidth, &arrayElementTypeByValue, &arrayElementTypeAlignmentCode);
	deconstruct_array(array, arrayElementType, arrayElementTypeWidth, arrayElementTypeByValue, arrayElementTypeAlignmentCode, &arrayContent, &arrayNullFlags, &arrayLength);
	
	inarray = (double*)malloc(arrayLength*sizeof(double));  
	for (i=0; i<arrayLength; i++) {
		inarray[i] = DatumGetFloat4(arrayContent[i]);
	}

	gsl_sort (inarray, 1, arrayLength);

	median = gsl_stats_median_from_sorted_data (inarray, 1, arrayLength);
	for (i=0; i<arrayLength; i++) {
		inarray[i] = fabs(inarray[i]-median);
	}
	gsl_sort (inarray, 1, arrayLength);
	mad = 1.486 * gsl_stats_median_from_sorted_data (inarray, 1, arrayLength);

	PG_RETURN_FLOAT8(mad);
}

Datum array_quantile(PG_FUNCTION_ARGS) {
	// The formal PostgreSQL array object
	ArrayType *array;

	// The array element type
	Oid arrayElementType;

	// The array element type width
	int16 arrayElementTypeWidth;

	// The array element type "is passed by value" flags (not used, should always be true)
	bool arrayElementTypeByValue;

	// The array element type alignment codes (not used)
	char arrayElementTypeAlignmentCode;

	// The array contents, as PostgreSQL "datum" objects
	Datum *arrayContent;

	// List of "is null" flags for the array contents
	bool *arrayNullFlags;

	// The size of each array
	int arrayLength;

	int i,j, nelem;
	double median, mad, f, quantile;
	double *inarray;

	if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) 
		ereport(ERROR, (errmsg("Null arrays not accepted")));

	// Get array and quantile from input
	array = PG_GETARG_ARRAYTYPE_P(0);
	f = PG_GETARG_FLOAT8(1);

	if (ARR_NDIM(array) != 1) 
		ereport(ERROR, (errmsg("One-dimesional arrays are required")));

	if (array_contains_nulls(array)) 
		ereport(ERROR, (errmsg("Array contains null elements")));

	arrayLength = (ARR_DIMS(array))[0];
	arrayElementType = ARR_ELEMTYPE(array);
	get_typlenbyvalalign(arrayElementType, &arrayElementTypeWidth, &arrayElementTypeByValue, &arrayElementTypeAlignmentCode);
	deconstruct_array(array, arrayElementType, arrayElementTypeWidth, arrayElementTypeByValue, arrayElementTypeAlignmentCode, &arrayContent, &arrayNullFlags, &arrayLength);
	
	inarray = (double*)malloc(arrayLength*sizeof(double));  
	for (i=0; i<arrayLength; i++) {
		inarray[i] = DatumGetFloat4(arrayContent[i]);
	}

	gsl_sort (inarray, 1, arrayLength);

	quantile = gsl_stats_quantile_from_sorted_data(inarray, 1, arrayLength, f);

	PG_RETURN_FLOAT8(quantile);
}


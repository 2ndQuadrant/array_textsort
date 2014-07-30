/*-------------------------------------------------------------------------
 *
 * array_textsort.c
 *	Function for textual array element sorting
 *
 * Authors: 
 *
 * Copyright (c) 2010-2011, 2014 2ndQuadrant Deutschland GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by 2ndQuadrant.
 * 4. Neither the name of 2ndQuadrant nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY 2NDQUADRANT ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL 2NDQUADRANT BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *-------------------------------------------------------------------------
 */

#include <postgres.h>
#include "access/tupmacs.h"

#include <lib/stringinfo.h>
#include <utils/array.h>
#include "utils/lsyscache.h"
#include <utils/builtins.h>
#include <catalog/pg_type.h>

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(array_textsort);
PG_FUNCTION_INFO_V1(array_distinct);

Datum array_textsort(PG_FUNCTION_ARGS);
Datum array_distinct(PG_FUNCTION_ARGS);

/* helper function, comparison callback for qsort() */
static int textcmp(const void *v1, const void *v2)
{
  const char *p1, *p2;
  char *c1, *c2, *d1, *d2;
  int len1, len2, result;

  /* get total element lengths */
  len1 =  VARSIZE(*((const char **)v1)) - VARHDRSZ;
  len2 =  VARSIZE(*((const char **)v2)) - VARHDRSZ;

  /* skip over the header */
  p1 = VARDATA(*((const char **)v1));
  p2 = VARDATA(*((const char **)v2));

  /* allocate buffers for NUL terminated copies */
  c1 = d1 = palloc(len1 + 1);
  c2 = d2 = palloc(len2 + 1);

  /* create copies as the original strings are not
     necessarily NUL terminated, strcoll() needs
     this though (there is no strncoll()) */
  strncpy(c1, p1, len1);
  strncpy(c2, p2, len2);

  /* add NUL terminaitors at end of copy */
  c1[len1] = '\0';
  c2[len2] = '\0';

  /* get comparison result */
  result = strcoll(c1, c2);

  /* clean up */
  pfree(c1);
  pfree(c2);
  
  /* done */
  return result;
}

Datum
array_textsort(PG_FUNCTION_ARGS)
{
  ArrayType  *input = PG_GETARG_ARRAYTYPE_P(0);
  ArrayType  *output;
  int nelems = ArrayGetNItems(ARR_NDIM(input), ARR_DIMS(input));
  char *data_p = ARR_DATA_PTR(input);
  char **elems;
  int16	elmlen;
  bool elmbyval;
  char elmalign;
  int i;

  get_typlenbyvalalign(ARR_ELEMTYPE(input), &elmlen, &elmbyval, &elmalign);

  /* checking for valid array
     basically this just checks that there are no null values
     type checking was already done by the caller */
  if (ARR_HASNULL(input) && array_contains_nulls(input)) {
    ereport(ERROR,
	    (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
	     errmsg("array must not contain nulls")));
  }
  /* not sure whether this is really important, but so far
     this flag was always false, so i have no idea what
     happens if it's ever going to be true ... better
     refuse to work in such cases right away */
  if (elmbyval) {
    ereport(ERROR,
	    (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
	     errmsg("array entries are not by value")));
  }

  /* allocate space for pointers to the individual array elements */
  elems = palloc(nelems * sizeof(char *));

  /* extract actual elements from array data */
  for (i = 0; i < nelems; i++) {
    elems[i] = data_p;
    data_p += att_align_nominal(att_addlength_pointer(0, elmlen, data_p), elmalign);
  }

  /* sort the elements using textcmp() as comparison function */
  qsort(elems, nelems, sizeof(char *), textcmp);

  /* create a copy of the input array to store the results in
     no further action needed as we are just changing the order
     of elements but not the number or total size */
  output = PG_GETARG_ARRAYTYPE_P_COPY(0);
  data_p = ARR_DATA_PTR(output);

  /* copy elements from input array to new position in output */
  for (i = 0; i < nelems; i++) {
    char *source = elems[i];
    int len = att_align_nominal(att_addlength_pointer(0, elmlen, source), elmalign);
    memcpy(data_p, source, len);
    data_p += len;
  }

  /* clean up */
  pfree(elems);

  /* return result */
  PG_RETURN_POINTER(output);
}

Datum
array_distinct(PG_FUNCTION_ARGS)
{
  ArrayType  *input = PG_GETARG_ARRAYTYPE_P(0);
  ArrayType  *output;
  int nelems = ArrayGetNItems(ARR_NDIM(input), ARR_DIMS(input));
  char *data_p = ARR_DATA_PTR(input);
  char **elems;
  char *lastelem;
  int16	elmlen;
  bool elmbyval;
  char elmalign;
  Datum *d;
  int i, j;
  
  get_typlenbyvalalign(ARR_ELEMTYPE(input), &elmlen, &elmbyval, &elmalign);
  
  /* checking for valid array
     basically this just checks that there are no null values
     type checking was already done by the caller */
  if (ARR_HASNULL(input) && array_contains_nulls(input)) {
    ereport(ERROR,
	    (errcode(ERRCODE_NULL_VALUE_NOT_ALLOWED),
	     errmsg("array must not contain nulls")));
  }
  /* not sure whether this is really important, but so far
     this flag was always false, so i have no idea what
     happens if it's ever going to be true ... better
     refuse to work in such cases right away */
  if (elmbyval) {
    ereport(ERROR,
	    (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
	     errmsg("array entries are not by value")));
  }
  
  /* allocate space for pointers to the individual array elements */
  elems = palloc(nelems * sizeof(char *));
  
  /* extract actual elements from array data */
  for (i = 0; i < nelems; i++) {
    elems[i] = data_p;
    data_p += att_align_nominal(att_addlength_pointer(0, elmlen, data_p), elmalign);
  }
  
  /* sort the elements using textcmp() as comparison function */
  qsort(elems, nelems, sizeof(char *), textcmp);

  d = (Datum *)palloc(nelems * sizeof(Datum));

  lastelem = NULL;
  for (i = 0, j = 0; i < nelems; i++) {
    if (lastelem) { 
      int32 l1 = VARSIZE(elems[i]);
      int32 l2 = VARSIZE(lastelem);
      if (l1 == l2 && !memcmp(VARDATA(elems[i]),VARDATA(lastelem), l1 - 4))
        continue;
    }
    lastelem = elems[i];
    d[j++] = PointerGetDatum(elems[i]);
  }

  output = construct_array(d, j, TEXTOID, -1, 0, 'i');

  /* cleanup */
  pfree(elems);
  pfree(d);

  PG_RETURN_ARRAYTYPE_P(output);
}

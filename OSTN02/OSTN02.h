//
//  OSTN02.h
//  OSTN02
//
//  Created by George MacKerron on 24/12/2011.
//  Copyright (c) 2011 George MacKerron. All rights reserved.
//

#ifndef OSTN02_OSTN02_h
#define OSTN02_OSTN02_h

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "crc32.h"

// #define use_long  // comment this line out for double precision -- it makes no difference within a millimetre on any test

#ifdef use_long
#define DBL       long double
#define L(x)      x ## L
#define DBLFMT    "Lf"
#define SIN       sinl
#define COS       cosl
#define TAN       tanl
#define SQRT      sqrtl
#define NUMDESC   "long double"
#else
#define DBL       double
#define L(x)      x
#define DBLFMT    "lf"
#define SIN       sin
#define COS       cos
#define TAN       tan
#define SQRT      sqrt
#define NUMDESC   "double"
#endif

#define CDBL      const DBL

#define LLFMT     "lat: % 11.6" DBLFMT ", lon: % 11.6" DBLFMT ", elevation: %8.3" DBLFMT
#define ENFMT     "  E: %11.3" DBLFMT ",   N: %11.3" DBLFMT ", elevation:  %7.3" DBLFMT " (%s / %s)"

#define BOLD      "\033[1m"
#define UNBOLD    "\033[22m"
#define INVERSE   "\033[7m"
#define UNINVERSE "\033[27m"
#define ULINE     "\033[4m"
#define UNULINE   "\033[24m"


typedef struct {
  unsigned char deg;  // range 0 - 180 (S or W)
  unsigned char min;  // range 0 - 60
  DBL sec;
  bool westOrSouth;
} DegMinSec;

typedef struct {
  DBL e;
  DBL n;
  DBL elevation;
  unsigned char geoid;
} EastingNorthing;

typedef struct {
  DBL lat;
  DBL lon;
  DBL elevation;
  unsigned char geoid;
} LatLonDecimal;

typedef struct {
  DegMinSec lat;
  DegMinSec lon;
  DBL elevation;
} LatLonDegMinSec;

typedef struct {
  DBL semiMajorAxis;
  DBL semiMinorAxis;
} Ellipsoid;

typedef struct {
  DBL centralMeridianScale;
  LatLonDecimal trueOriginLatLon;
  EastingNorthing trueOriginEastingNorthing;
} MapProjection;

typedef struct {
  unsigned int eMin   : 10;
  unsigned int eCount : 10;
  unsigned int offset : 20;
} __attribute__((packed)) OSTN02Index;

typedef struct {
  unsigned int eShift : 15;
  unsigned int nShift : 15;
  unsigned int gShift : 14;
  unsigned int gFlag  :  4;
} __attribute__((packed)) OSTN02Datum;

EastingNorthing latLonToEastingNorthing(const LatLonDecimal latLon, const Ellipsoid ellipsoid, const MapProjection projection);
EastingNorthing ETRS89LatLonToETRSEastingNorthing(const LatLonDecimal latLon);
EastingNorthing OSTN02Shifts(const int eIndex, const int nIndex);
EastingNorthing ETRS89EastingNorthingToOSGB36EastingNorthing(const EastingNorthing en);
LatLonDecimal   latLonDecimalFromLatLonDegMinSec(const LatLonDegMinSec dms);
char            *gridRefFromOSGB36EastingNorthing(const EastingNorthing en, const bool spaces, const int res);  // be sure to free(result) after use
char            *tetradFromOSGB36EastingNorthing(const EastingNorthing en);                                     // be sure to free(result) after use
bool            test(const bool noisily);

#endif

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


#define use_long  // comment this line out for double precision

#ifdef use_long
#define dbl      long double
#define cdbl     const long double
#define sfx      L
#define SIN      sinl
#define COS      cosl
#define TAN      tanl
#define SQRT     sqrtl
#define numdesc  "long double"
#define llFmtStr "lat: % 11.6Lf, lon: % 11.6Lf, elevation: %8.3Lf"
#define enFmtStr "  E: %11.3Lf,   N: %11.3Lf, elevation:  %7.3Lf (%s / %s)"

#else

#define dbl      double
#define cdbl     const double
#define sfx
#define SIN      sin
#define COS      cos
#define TAN      tan
#define SQRT     sqrt
#define numdesc  "double"
#define llFmtStr "lat: % 11.6f, lon: % 11.6f, elevation: %8.3f"
#define enFmtStr "  E: %11.3f,   N: %11.3f, elevation:  %7.3f (%s / %s)"

#endif


#define BOLD      "\033[1m"
#define UNBOLD    "\033[22m"
#define INVERSE   "\033[7m"
#define UNINVERSE "\033[27m"
#define ULINE     "\033[4m"
#define UNULINE   "\033[24m"


typedef struct {
  unsigned char deg;  // range 0 - 180 (S or W)
  unsigned char min;  // range 0 - 60
  dbl sec;
  bool westOrSouth;
} DegMinSec;

typedef struct {
  dbl e;
  dbl n;
  dbl elevation;
  unsigned char geoid;
} EastingNorthing;

typedef struct {
  dbl lat;
  dbl lon;
  dbl elevation;
  unsigned char geoid;
} LatLonDecimal;

typedef struct {
  DegMinSec lat;
  DegMinSec lon;
  dbl elevation;
} LatLonDegMinSec;

typedef struct {
  dbl semiMajorAxis;
  dbl semiMinorAxis;
} Ellipsoid;

typedef struct {
  dbl centralMeridianScale;
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
char            *gridRefFromOSGB36EastingNorthing(const EastingNorthing en, const bool spaces, const int res) ;
bool            test(const bool noisily);

#endif

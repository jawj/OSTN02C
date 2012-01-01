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
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <OpenSSL/md5.h>


#define use_long  // comment this line out for double precision

#ifdef use_long
#define numtype  long double
#define cnumtype const long double
#define SIN      sinl
#define COS      cosl
#define TAN      tanl
#define SQRT     sqrtl
#define numdesc  "long double"
#define llFmtStr "%s lat: % 11.6Lf,  lon: % 11.6Lf,  elevation: %11.6Lf%s"
#define enFmtStr "%s   E: %11.3Lf,    N: %11.3Lf,  elevation:  %7.3Lf (%s / %s)%s"

#else

#define numtype  double
#define cnumtype const double
#define SIN      sin
#define COS      cos
#define TAN      tan
#define SQRT     sqrt
#define numdesc  "double"
#define llFmtStr "%s lat: % 11.6f,  lon: % 11.6f,  elevation: %11.6f%s"
#define enFmtStr "%s   E: %11.3f,    N: %11.3f,  elevation:  %7.3f (%s / %s)%s"

#endif

char *OSGB36GeoidNames[15];
char *OSGB36GeoidRegions[15];

typedef struct {
	unsigned char deg;  // range 0 - 180 (S or W)
	unsigned char min;  // range 0 - 60
	numtype sec;
  bool westOrSouth;
} DegMinSec;

typedef struct {
	numtype e;
	numtype n;
  numtype elevation;
  signed char geoid;
} EastingNorthing;

typedef struct {
	numtype lat;
	numtype lon;
  numtype elevation;
  unsigned char geoid;
} LatLonDecimal;

typedef struct {
	DegMinSec lat;
	DegMinSec lon;
  numtype elevation;
} LatLonDegMinSec;

typedef struct {
	numtype semiMajorAxis;
	numtype semiMinorAxis;
} Ellipsoid;

typedef struct {
	numtype centralMeridianScale;
  LatLonDecimal trueOriginLatLon;
	EastingNorthing trueOriginEastingNorthing;
} MapProjection;

typedef struct {
	unsigned short eShift;
	unsigned short nShift;
  unsigned short gShift;
  unsigned char  gFlag;
} OSTN02Record;  // corresponds to our OSTN02 data byte format

EastingNorthing latLonToEastingNorthing(LatLonDecimal latLon, Ellipsoid ellipsoid, MapProjection projection);
EastingNorthing ETRS89LatLonToETRSEastingNorthing(LatLonDecimal latLon);
EastingNorthing OSTN02Shifts(const short eIndex, const short nIndex);
EastingNorthing ETRS89EastingNorthingToOSGB36EastingNorthing(EastingNorthing en);
LatLonDecimal   latLonDecimalFromLatLonDegMinSec(LatLonDegMinSec dms);
void            doTests(void);

#endif

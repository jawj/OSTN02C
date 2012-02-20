//
//  dblRelated.h
//  OSTN02
//
//  Created by George MacKerron on 20/02/2012.
//  Copyright (c) 2012 George MacKerron. All rights reserved.
//

#ifndef OSTN02_dblRelated_h
#define OSTN02_dblRelated_h

#ifdef USE_LONG
#define L(x)      x ## L
#define DBLFMT    "Lf"
#define SIN       sinl
#define COS       cosl
#define TAN       tanl
#define SQRT      sqrtl
#define ABS       fabs
#define NUMDESC   "long double"
#else
#define L(x)      x
#define DBLFMT    "lf"
#define SIN       sin
#define COS       cos
#define TAN       tan
#define SQRT      sqrt
#define ABS       fabsl
#define NUMDESC   "double"
#endif

#define CDBL      const DBL
#define LLFMT     "lat: % 11.6" DBLFMT ", lon: % 11.6" DBLFMT ", elevation: %8.3" DBLFMT
#define ENFMT     "  E: %11.3" DBLFMT ",   N: %11.3" DBLFMT ", elevation:  %7.3" DBLFMT " (%s / %s)"

#endif

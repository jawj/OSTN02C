//
//  main.c
//  OSTN02
//
//  Created by George MacKerron on 24/12/2011.
//  Copyright (c) 2011 George MacKerron. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "OSTN02.h"

int main (int argc, const char * argv[]) {
  if (argc == 2 && strcmp(argv[1], "--tests") == 0) {
    doTests();
    return EXIT_SUCCESS;
  } else if (argc == 4) {
    LatLonDecimal latLon;
    sscanf(argv[1], "%Lf", &latLon.lat);
    sscanf(argv[2], "%Lf", &latLon.lon);
    sscanf(argv[3], "%Lf", &latLon.elevation);
    latLon.geoid = 0;
    EastingNorthing en = ETRS89EastingNorthingToOSGB36EastingNorthing(ETRS89LatLonToETRSEastingNorthing(latLon));
    if (en.geoid == 0) {
      printf("Coordinates outside range.\n");
      return EXIT_FAILURE;
    }
    printf(llFmtStr, "ETRS89 in ", latLon.lat, latLon.lon, latLon.elevation, "\n");
    printf(enFmtStr, "OSGB36 out", en.e, en.n, en.elevation, OSGB36GeoidRegions[en.geoid], OSGB36GeoidNames[en.geoid], "\n");
    return EXIT_SUCCESS;
  } else {
    printf("This tool converts an ETRS89/WGS84 lat/lon/elevation to OSGB36 Easting/Northing/elevation using OSTN02 (compiled %s %s, %s precision).\n", __DATE__, __TIME__, numdesc);
    printf("Usage: 'OSTN02 --tests' runs tests with OS data.\n");
    printf("       'OSTN02 [lat] [lon] [elevation]' converts coordinates.\n");
    return EXIT_FAILURE;
  }
}


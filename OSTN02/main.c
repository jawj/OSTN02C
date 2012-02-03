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
#include "geoids.data"

int main (int argc, const char * argv[]) {
  
  // with arg --tests, run tests
  if (argc == 2 && strcmp(argv[1], "--tests") == 0) {
    bool passed = test(true);
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
    
  // with one set of coords (lat lon elevation) given on the command line, convert with verbose output
  } else if (argc == 4) {
    LatLonDecimal latLon;
    sscanf(argv[1], "%" DBLFMT, &latLon.lat);
    sscanf(argv[2], "%" DBLFMT, &latLon.lon);
    sscanf(argv[3], "%" DBLFMT, &latLon.elevation);
    latLon.geoid = 0;
    EastingNorthing en = ETRS89EastingNorthingToOSGB36EastingNorthing(ETRS89LatLonToETRSEastingNorthing(latLon));
    if (en.geoid == 0) {
      printf("Coordinates outside OSTN02 range.\n\n");
      return EXIT_FAILURE;
    }
    char *gridRef = gridRefFromOSGB36EastingNorthing(en, true, 10);
    char *tetrad = tetradFromOSGB36EastingNorthing(en);
    printf("ETRS89 in  ");
    printf(LLFMT, latLon.lat, latLon.lon, latLon.elevation);
    printf("\nOSGB36 out ");
    printf(ENFMT, en.e, en.n, en.elevation, OSGB36GeoidRegions[en.geoid], OSGB36GeoidNames[en.geoid]);
    printf(", ref: %s, tetrad: %s\n\n", gridRef, tetrad);
    free(tetrad);
    free(gridRef);
    return EXIT_SUCCESS;
  
  // with any other arguments, show help text
  } else if (argc > 1) {
    printf("%sOSTN02C%s -- https://github.com/jawj/OSTN02C -- Built %s %s (%s precision).\n\n", INVERSE, UNINVERSE, __DATE__, __TIME__, NUMDESC);
    printf("This tool converts ETRS89/WGS84 coordinates to OSGB36 using OSTN02 and OSGM02.\n\n");
    printf("Usage: %sOSTN02 %slat%s %slon%s %selevation%s%s converts one set of coordinates with human-friendly output.\n", BOLD, ULINE, UNULINE, ULINE, UNULINE, ULINE, UNULINE, UNBOLD);
    printf("       %sOSTN02%s (without arguments) does batch conversion, reading from STDIN and writing to STDOUT (see below).\n", BOLD, UNBOLD);
    printf("       %sOSTN02 --tests%s verifies data integrity and runs conversion tests with known coordinates.\n", BOLD, UNBOLD);
    printf("       %sOSTN02 --help%s displays this message.\n\n", BOLD, UNBOLD);
    
    printf("For batch conversion:\n");
    printf("* Input rows should have 3 columns -- lat, lon and elevation -- with any reasonable separator.\n");
    printf("* Output rows have 4 comma-separated columns: easting, northing, elevation, and a geoid flag.\n");
    printf("* Out-of-range input coordinates produce all-zero output rows.\n");
    printf("* Malformatted input terminates processing, with a non-zero exit code.\n\n");
    
    printf("Copyright © George MacKerron 2012 (http://mackerron.com). Released under the MIT licence (http://www.opensource.org/licenses/mit-license.php).\n");
    printf("OSTN02 and OSGM02 are trademarks of Ordnance Survey. Incorporated OSTN02 and OSGM02 data are © Crown copyright 2002. All rights reserved.\n\n");
    return EXIT_SUCCESS;
    
  // without arguments, convert 3-column CSV/TSV/similar (lat, lon, elevation) to 4-column CSV (easting, northing, elevation, geoid flag)
  } else {
    LatLonDecimal latLon;
    int scanResult;
    while((scanResult = scanf("%" DBLFMT "%*[ \t,;:]%" DBLFMT "%*[ \t,;:]%" DBLFMT "%*[ \t,;:\n\r]", &latLon.lat, &latLon.lon, &latLon.elevation)) == 3) {
      latLon.geoid = 0;
      EastingNorthing en = ETRS89EastingNorthingToOSGB36EastingNorthing(ETRS89LatLonToETRSEastingNorthing(latLon));
      printf("%" DBLFMT ",%" DBLFMT ",%" DBLFMT ",%d\n", en.e, en.n, en.elevation, en.geoid);
    }
    return scanResult == EOF ? EXIT_SUCCESS : EXIT_FAILURE;
  }
  
}


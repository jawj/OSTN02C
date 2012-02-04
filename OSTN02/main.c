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
  
  // with arg --test, run tests
  if (argc == 2 && strcmp(argv[1], "--test") == 0) {
    bool passed = test(true);
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
    
  // with arg --list-geoids, show geoid datum names and regions
  } else if (argc == 2 && strcmp(argv[1], "--list-geoids") == 0) {
    puts(BOLD "\nFlag  Datum          Region" UNBOLD);
    const int len = LENGTH_OF(OSGB36GeoidNames);
    for (int i = 0; i < len; i ++)
      printf("  %2d  %-13s  %s\n", i, OSGB36GeoidNames[i], OSGB36GeoidRegions[i]);
    puts("");
    
  // with one set of coords (lat lon elevation) given on the command line, convert with verbose output
  } else if (argc == 4) {
    LatLonDecimal latLon;
    latLon.lat = latLon.lon = latLon.elevation = 0;
    sscanf(argv[1], "%" DBLFMT, &latLon.lat);
    sscanf(argv[2], "%" DBLFMT, &latLon.lon);
    sscanf(argv[3], "%" DBLFMT, &latLon.elevation);
    printf("\nETRS89 in  ");
    printf(LLFMT, latLon.lat, latLon.lon, latLon.elevation);
    EastingNorthing en = ETRS89EastingNorthingToOSGB36EastingNorthing(ETRS89LatLonToETRSEastingNorthing(latLon));
    if (en.geoid == 0) {
      puts("\nCoordinates outside OSTN02 range\n");
      return EXIT_FAILURE;
    }
    char *gridRef = gridRefFromOSGB36EastingNorthing(en, false, 10);
    printf("\nOSGB36 out ");
    printf(ENFMT, en.e, en.n, en.elevation, OSGB36GeoidRegions[en.geoid], OSGB36GeoidNames[en.geoid]);
    printf("\n           ref:  %s\n\n", gridRef);
    free(gridRef);
    return EXIT_SUCCESS;
  
  // with any other arguments, show help text
  } else if (argc > 1) {
    puts("\n" INVERSE "OSTN02C" UNINVERSE " -- https://github.com/jawj/OSTN02C -- Built " __DATE__ " " __TIME__ " (" NUMDESC " precision)\n"
         "\n"
         "This tool converts ETRS89/WGS84 coordinates to OSGB36 using OSTN02 and OSGM02\n"
         "\n"
         "Usage: " BOLD "OSTN02 " ULINE "lat" UNULINE " " ULINE "lon" UNULINE " " ULINE "elevation" UNULINE UNBOLD " converts one set of coordinates with human-friendly output\n"
         "       " BOLD "OSTN02" UNBOLD " (without arguments) does batch conversion, reading from STDIN and writing to STDOUT (see below)\n"
         "       " BOLD "OSTN02 --list-geoids" UNBOLD " lists the datum name and region for each geoid datum flag\n"
         "       " BOLD "OSTN02 --test" UNBOLD " checks embedded data integrity and runs conversion tests with known coordinates\n"
         "       " BOLD "OSTN02 --help" UNBOLD " displays this message\n"
         "\n"
         "For batch conversion:\n"
         "* Input rows must have 3 columns -- lat, lon and elevation -- with any reasonable separator (,;:| \\t)\n"
         "* Output rows have 4 comma-separated columns: easting, northing, elevation, and the geoid datum flag\n"
         "* In case of out-of-range input coordinates, all output columns will be zero\n"
         "* Malformatted input terminates processing, and results in a non-zero exit code\n"
         "\n"
         "Software copyright (c) George MacKerron 2012 (http://mackerron.com)\n"
         "Released under the MIT licence (http://www.opensource.org/licenses/mit-license.php)\n"
         "OSTN02 and OSGM02 are trademarks of Ordnance Survey\n"
         "Embedded OSTN02 and OSGM02 data are (c) Crown copyright 2002. All rights reserved.\n");
    return EXIT_SUCCESS;
    
  // without arguments, convert 3-column CSV/TSV/similar (lat, lon, elevation) to 4-column CSV (easting, northing, elevation, geoid flag)
  } else {
    LatLonDecimal latLon;
    int scanResult;
    while((scanResult = scanf("%" DBLFMT "%*[ \t,;:|]%" DBLFMT "%*[ \t,;:|]%" DBLFMT "%*[ \t,;:|\r]\n", 
                              &latLon.lat, &latLon.lon, &latLon.elevation)) == 3) {
      EastingNorthing en = ETRS89EastingNorthingToOSGB36EastingNorthing(ETRS89LatLonToETRSEastingNorthing(latLon));
      printf("%.3" DBLFMT ",%.3" DBLFMT ",%.3" DBLFMT ",%d\n", en.e, en.n, en.elevation, en.geoid);
    }
    return scanResult == EOF ? EXIT_SUCCESS : EXIT_FAILURE;
  }
  
}


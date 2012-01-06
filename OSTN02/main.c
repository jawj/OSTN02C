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
  if (argc == 2 && strcmp(argv[1], "--tests") == 0) {
    bool passed = test(true);
    return passed ? EXIT_SUCCESS : EXIT_FAILURE;
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
  } else {
    printf("%sOSTN02C%s - Built %s %s (%s precision).\n", INVERSE, UNINVERSE, __DATE__, __TIME__, NUMDESC);
    printf("Copyright © George MacKerron 2012 (http://mackerron.com). Released under the MIT licence (http://www.opensource.org/licenses/mit-license.php).\n");
    printf("OSTN02 and OSGM02 are trademarks of Ordnance Survey. Incorporated OSTN02 and OSGM02 data are © Crown copyright 2002. All rights reserved.\n\n");
    printf("Usage: %sOSTN02 --tests%s verifies data integrity and runs tests.\n", BOLD, UNBOLD);
    printf("       %sOSTN02 %slat%s %slon%s %selevation%s%s converts ETRS/WGS84 coordinates to OSGB36 using OSTN02 + OSGM02.\n\n", BOLD, ULINE, UNULINE, ULINE, UNULINE, ULINE, UNULINE, UNBOLD);
    return EXIT_FAILURE;
  }
}


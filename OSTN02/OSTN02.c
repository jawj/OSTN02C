//
//  OSTN02.c
//  OSTN02
//
//  Created by George MacKerron on 24/12/2011.
//  Copyright (c) 2011 George MacKerron. All rights reserved.
//

#include "OSTN02.h"
#include "OSTN02.index.struct-array"
#include "OSTN02.data.struct-array"
#include "testCoords.struct-arrays"

#define piOver180 0.0174532925199432957692369076848861271344287188854172545609L

char *OSGB36GeoidNames[] = {
  "N/A",
  "Newlyn",
  "St Marys",
  "Douglas02",
  "Stornoway",
  "St Kilda",
  "Lerwick",
  "Newlyn",
  "Fair Isle",
  "Flannan Isles",
  "North Rona",
  "Sule Skerry",
  "Foula",
  "Malin Head",
  "Belfast"
};

char *OSGB36GeoidRegions[] = {
  "Outside model boundary",
  "UK mainland",
  "Scilly Isles",
  "Isle of Man",
  "Outer Hebrides",
  "St Kilda",
  "Shetland Isles",
  "Orkney Isles",
  "Fair Isle",
  "Flannan Isles",
  "North Rona",
  "Sule Skerry",
  "Foula",
  "Republic of Ireland",
  "Northern Ireland"
};

EastingNorthing latLonToEastingNorthing(const LatLonDecimal latLon, const Ellipsoid ellipsoid, const MapProjection projection) {
  cdbl a   = ellipsoid.semiMajorAxis;
  cdbl b   = ellipsoid.semiMinorAxis;
  cdbl f0  = projection.centralMeridianScale;
  cdbl toE = projection.trueOriginEastingNorthing.e;
  cdbl toN = projection.trueOriginEastingNorthing.n;
  
  // Convert degrees to radians
  cdbl phi     = piOver180 * latLon.lat;
  cdbl lambda  = piOver180 * latLon.lon;
  cdbl phi0    = piOver180 * projection.trueOriginLatLon.lat;
  cdbl lambda0 = piOver180 * projection.trueOriginLatLon.lon;
  
  // Calculations
  cdbl deltaPhi = phi - phi0;
  cdbl sumPhi   = phi + phi0;
  cdbl sinPhi   = SIN(phi);
  cdbl sinPhi2  = sinPhi * sinPhi;
  cdbl cosPhi   = COS(phi);
  cdbl cosPhi2  = cosPhi * cosPhi;
  cdbl cosPhi3  = cosPhi2 * cosPhi;
  cdbl cosPhi5  = cosPhi3 * cosPhi2;
  cdbl tanPhi   = TAN(phi);
  cdbl tanPhi2  = tanPhi * tanPhi;
  cdbl tanPhi4  = tanPhi2 * tanPhi2;
  
  cdbl a2   = a * a;
  cdbl e2   = (a2 - b * b) / a2;
  cdbl n    = (a - b) / (a + b);
  cdbl n2   = n * n;
  cdbl n3   = n2 * n;
  cdbl oneMinusE2SinPhi2 = 1.0L - e2 * sinPhi2;
  cdbl sqrtOneMinusE2SinPhi2 = SQRT(oneMinusE2SinPhi2);
  cdbl aF0  = a * f0;
  cdbl v    = aF0 / sqrtOneMinusE2SinPhi2;
  cdbl rho  = aF0 * (1.0L - e2) / (oneMinusE2SinPhi2 * sqrtOneMinusE2SinPhi2);
  cdbl eta2 = v / rho - 1.0L;
  cdbl m    = b * f0 * ( (1.0L + n + (5.0L / 4.0L) * n2 + (5.0L / 4.0L) * n3) * deltaPhi
                       - (3.0L * n + 3.0L * n2 + (21.0L / 8.0L) * n3) * SIN(deltaPhi) * COS(sumPhi)
                       + ((15.0L / 8.0L) * n2 + (15.0L / 8.0L) * n3) * SIN(2.0L * deltaPhi) * COS(2.0L * sumPhi)
                       - (35.0L / 24.0L) * n3 * SIN(3.0L * deltaPhi) * COS(3.0L * sumPhi) 
                       );
  
  cdbl one    = m + toN;
  cdbl two    = (v /   2.0L) * sinPhi * cosPhi;
  cdbl three  = (v /  24.0L) * sinPhi * cosPhi3 * (5.0L - tanPhi2 + 9.0L * eta2);
  cdbl threeA = (v / 720.0L) * sinPhi * cosPhi5 * (61.0L - 58.0L * tanPhi2 + tanPhi4);
  cdbl four   = v * cosPhi;
  cdbl five   = (v /   6.0L) * cosPhi3 * (v / rho - tanPhi2);
  cdbl six    = (v / 120.0L) * cosPhi5 * (5.0L - 18.0L * tanPhi2 + tanPhi4 + 14.0L * eta2 - 58.0L * tanPhi2 * eta2);
  
  cdbl deltaLambda  = lambda - lambda0;
  cdbl deltaLambda2 = deltaLambda  * deltaLambda;
  cdbl deltaLambda3 = deltaLambda2 * deltaLambda;
  cdbl deltaLambda4 = deltaLambda3 * deltaLambda;
  cdbl deltaLambda5 = deltaLambda4 * deltaLambda;
  cdbl deltaLambda6 = deltaLambda5 * deltaLambda;
  
  EastingNorthing en;
  en.n = one + two * deltaLambda2 + three * deltaLambda4 + threeA * deltaLambda6;
  en.e = toE + four * deltaLambda + five * deltaLambda3 + six * deltaLambda5;
  en.elevation = latLon.elevation;
  en.geoid = latLon.geoid;
  
  return en;
}

EastingNorthing ETRS89LatLonToETRSEastingNorthing(const LatLonDecimal latLon) {
  Ellipsoid ETRS89Ellipsoid;
  ETRS89Ellipsoid.semiMajorAxis = 6378137.0L;
  ETRS89Ellipsoid.semiMinorAxis = 6356752.3141L;  
  
  MapProjection nationalGridProj;
  nationalGridProj.centralMeridianScale = 0.9996012717L;
  nationalGridProj.trueOriginLatLon.lat = 49.0L;
  nationalGridProj.trueOriginLatLon.lon = -2.0L;
  nationalGridProj.trueOriginEastingNorthing.e =  400000.0L;
  nationalGridProj.trueOriginEastingNorthing.n = -100000.0L;
  
  return latLonToEastingNorthing(latLon, ETRS89Ellipsoid, nationalGridProj);
}

EastingNorthing OSTN02Shifts(const int eIndex, const int nIndex) {
  EastingNorthing shifts;
  shifts.e = shifts.n = shifts.elevation = shifts.geoid = 0;
  if (nIndex < 0 || nIndex > 1250) return shifts;

  OSTN02Index dataIndex = OSTN02Indices[nIndex];
  if (eIndex < dataIndex.eMin || eIndex >= dataIndex.eMin + dataIndex.eCount) return shifts;
  
  unsigned int dataOffset = dataIndex.offset + (eIndex - dataIndex.eMin);
  OSTN02Datum record = OSTN02Data[dataOffset];
  if (record.gFlag == 0) return shifts;
  
  shifts.e         = (((dbl) record.eShift) / 1000.0L) + 86.0L;
  shifts.n         = (((dbl) record.nShift) / 1000.0L) - 82.0L;
  shifts.elevation = (((dbl) record.gShift) / 1000.0L) + 43.0L;
  shifts.geoid     = record.gFlag;
  return shifts;
}

EastingNorthing ETRS89EastingNorthingToOSGB36EastingNorthing(const EastingNorthing en) {
  EastingNorthing shifted;
  shifted.e = shifted.n = shifted.elevation = shifted.geoid = 0;

  int  e0 = (int) (en.e / 1000.0L);
  int  n0 = (int) (en.n / 1000.0L);
  cdbl dx = en.e - (dbl) (e0 * 1000);
  cdbl dy = en.n - (dbl) (n0 * 1000);
  cdbl t  = dx / 1000.0L;
  cdbl u  = dy / 1000.0L;
  
  EastingNorthing shifts0 = OSTN02Shifts(e0    , n0    );
  if (shifts0.geoid == 0) return shifted;
  
  EastingNorthing shifts1 = OSTN02Shifts(e0 + 1, n0    );
  if (shifts1.geoid == 0) return shifted;
  
  EastingNorthing shifts2 = OSTN02Shifts(e0 + 1, n0 + 1);
  if (shifts2.geoid == 0) return shifted;
  
  EastingNorthing shifts3 = OSTN02Shifts(e0    , n0 + 1);
  if (shifts3.geoid == 0) return shifted;
  
  cdbl weight0 = (1.0L - t) * (1.0L - u);
  cdbl weight1 =         t  * (1.0L - u);
  cdbl weight2 =         t  *         u;
  cdbl weight3 = (1.0L - t) *         u;
  
  shifted.e = en.e + weight0 * shifts0.e
                   + weight1 * shifts1.e 
                   + weight2 * shifts2.e 
                   + weight3 * shifts3.e;
  
  shifted.n = en.n + weight0 * shifts0.n 
                   + weight1 * shifts1.n
                   + weight2 * shifts2.n 
                   + weight3 * shifts3.n;
  
  shifted.elevation = en.elevation - (weight0 * shifts0.elevation 
                                    + weight1 * shifts1.elevation
                                    + weight2 * shifts2.elevation
                                    + weight3 * shifts3.elevation);
  
  bool left   = dx < 500.0L;
  bool bottom = dy < 500.0L;
  EastingNorthing nearestShift = left ? (bottom ? shifts0 : shifts3) : (bottom ? shifts1 : shifts2);
  shifted.geoid = nearestShift.geoid;
  
  return shifted;
}

LatLonDecimal latLonDecimalFromLatLonDegMinSec(const LatLonDegMinSec dms) {
  LatLonDecimal dec;
  dec.lat = (dms.lat.westOrSouth ? -1.0L : 1.0L) * (((dbl) dms.lat.deg) + ((dbl) dms.lat.min) / 60.0L + dms.lat.sec / 3600.0L);
  dec.lon = (dms.lon.westOrSouth ? -1.0L : 1.0L) * (((dbl) dms.lon.deg) + ((dbl) dms.lon.min) / 60.0L + dms.lon.sec / 3600.0L);
  dec.elevation = dms.elevation;
  dec.geoid = 0;
  return dec;
}

bool test(bool noisily) {
  short numTested = 0;
  short numPassed = 0;
  bool  testPassed;
  
  // check data integrity
  
  unsigned long dataCRC = crc32((unsigned char *) OSTN02Data, sizeof(OSTN02Data));
  testPassed = dataCRC == originalDataCRC;
  numTested ++;
  if (testPassed) numPassed ++;
  if (noisily) {
    printf("Original CRC32 (data):  %li\n", originalDataCRC);
    printf("%sComputed CRC32 (data):  %li%s\n\n", (testPassed ? "" : BOLD), dataCRC, (testPassed ? "" : UNBOLD));
  }
  
  unsigned long indicesCRC = crc32((unsigned char *) OSTN02Indices, sizeof(OSTN02Indices));
  testPassed = indicesCRC == originalIndicesCRC;
  numTested ++;
  if (testPassed) numPassed ++;
  if (noisily) {
    printf("Original CRC32 (index): %li\n", originalIndicesCRC);
    printf("%sComputed CRC32 (index): %li%s\n\n", (testPassed ? "" : BOLD), indicesCRC, (testPassed ? "" : UNBOLD));
  }
  
  // check test conversions against known good results

  char len = sizeof(testETRSCoords) / sizeof(LatLonDegMinSec);
  for (char i = 0; i < len; i ++) {
    LatLonDecimal testLatLonDec = latLonDecimalFromLatLonDegMinSec(testETRSCoords[i]);
    EastingNorthing realEN = testOSGB36Coords[i];
    EastingNorthing testEN = ETRS89EastingNorthingToOSGB36EastingNorthing(ETRS89LatLonToETRSEastingNorthing(testLatLonDec));
    
    char *ETRS89Str, *realENStr, *testENStr;
    asprintf(&ETRS89Str, llFmtStr, testLatLonDec.lat, testLatLonDec.lon, testLatLonDec.elevation);
    asprintf(&realENStr, enFmtStr, realEN.e, realEN.n, realEN.elevation, OSGB36GeoidRegions[realEN.geoid], OSGB36GeoidNames[realEN.geoid]);
    asprintf(&testENStr, enFmtStr, testEN.e, testEN.n, testEN.elevation, OSGB36GeoidRegions[testEN.geoid], OSGB36GeoidNames[testEN.geoid]);
    
    testPassed = strcmp(realENStr, testENStr) == 0;
    numTested ++;
    if (testPassed) numPassed ++;
    
    if (noisily) {
      printf("ETRS89          %s\n",   ETRS89Str);
      printf("OSGB36 actual   %s\n",   realENStr);
      printf("%sOSGB36 computed %s\n\n%s", (testPassed ? "" : BOLD), testENStr, (testPassed ? "" : UNBOLD));
    }
    
    free(ETRS89Str); free(realENStr); free(testENStr);
  }
  
  bool allPassed = numTested == numPassed;
  if (noisily) printf("%s%i tests; %i passed; %i failed%s\n\n", (allPassed ? "" : BOLD), numTested, numPassed, numTested - numPassed, (allPassed ? "" : UNBOLD));
  return allPassed;
}

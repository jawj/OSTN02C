//
//  OSTN02.c
//  OSTN02
//
//  Created by George MacKerron on 24/12/2011.
//  Copyright (c) 2011 George MacKerron. All rights reserved.
//

#include "OSTN02.h"
#include "OSTN02.byte-array"

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
  cnumtype a   = ellipsoid.semiMajorAxis;
  cnumtype b   = ellipsoid.semiMinorAxis;
  cnumtype f0  = projection.centralMeridianScale;
  cnumtype toE = projection.trueOriginEastingNorthing.e;
  cnumtype toN = projection.trueOriginEastingNorthing.n;
  
  // Convert degrees to radians
  cnumtype phi     = piOver180 * latLon.lat;
  cnumtype lambda  = piOver180 * latLon.lon;
  cnumtype phi0    = piOver180 * projection.trueOriginLatLon.lat;
  cnumtype lambda0 = piOver180 * projection.trueOriginLatLon.lon;
  
  // Calculations
  cnumtype deltaPhi = phi - phi0;
  cnumtype sumPhi   = phi + phi0;
  cnumtype sinPhi   = SIN(phi);
  cnumtype sinPhi2  = sinPhi * sinPhi;
  cnumtype cosPhi   = COS(phi);
  cnumtype cosPhi2  = cosPhi * cosPhi;
  cnumtype cosPhi3  = cosPhi2 * cosPhi;
  cnumtype cosPhi5  = cosPhi3 * cosPhi2;
  cnumtype tanPhi   = TAN(phi);
  cnumtype tanPhi2  = tanPhi * tanPhi;
  cnumtype tanPhi4  = tanPhi2 * tanPhi2;
  
  cnumtype a2   = a * a;
  cnumtype e2   = (a2 - b * b) / a2;
  cnumtype n    = (a - b) / (a + b);
  cnumtype n2   = n * n;
  cnumtype n3   = n2 * n;
  cnumtype oneMinusE2SinPhi2 = 1.0L - e2 * sinPhi2;
  cnumtype sqrtOneMinusE2SinPhi2 = SQRT(oneMinusE2SinPhi2);
  cnumtype aF0  = a * f0;
  cnumtype v    = aF0 / sqrtOneMinusE2SinPhi2;
  cnumtype rho  = aF0 * (1.0L - e2) / (oneMinusE2SinPhi2 * sqrtOneMinusE2SinPhi2);
  cnumtype eta2 = v / rho - 1.0L;
  cnumtype m    = b * f0 * ( (1.0L + n + (5.0L / 4.0L) * n2 + (5.0L / 4.0L) * n3) * deltaPhi
                           - (3.0L * n + 3.0L * n2 + (21.0L / 8.0L) * n3) * SIN(deltaPhi) * COS(sumPhi)
                           + ((15.0L / 8.0L) * n2 + (15.0L / 8.0L) * n3) * SIN(2.0L * deltaPhi) * COS(2.0L * sumPhi)
                           - (35.0L / 24.0L) * n3 * SIN(3.0L * deltaPhi) * COS(3.0L * sumPhi) 
                           );
  
  cnumtype one    = m + toN;
  cnumtype two    = (v /   2.0L) * sinPhi * cosPhi;
  cnumtype three  = (v /  24.0L) * sinPhi * cosPhi3 * (5.0L - tanPhi2 + 9.0L * eta2);
  cnumtype threeA = (v / 720.0L) * sinPhi * cosPhi5 * (61.0L - 58.0L * tanPhi2 + tanPhi4);
  cnumtype four   = v * cosPhi;
  cnumtype five   = (v /   6.0L) * cosPhi3 * (v / rho - tanPhi2);
  cnumtype six    = (v / 120.0L) * cosPhi5 * (5.0L - 18.0L * tanPhi2 + tanPhi4 + 14.0L * eta2 - 58.0L * tanPhi2 * eta2);
  
  cnumtype deltaLambda  = lambda - lambda0;
  cnumtype deltaLambda2 = deltaLambda  * deltaLambda;
  cnumtype deltaLambda3 = deltaLambda2 * deltaLambda;
  cnumtype deltaLambda4 = deltaLambda3 * deltaLambda;
  cnumtype deltaLambda5 = deltaLambda4 * deltaLambda;
  cnumtype deltaLambda6 = deltaLambda5 * deltaLambda;
  
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

EastingNorthing OSTN02Shifts(const short eIndex, const short nIndex) {
  OSTN02Record record;
  unsigned int  recordIndex = eIndex + (nIndex * 701);
  unsigned long offset = recordIndex * 7;
  
  /*
  // using an external data blob
  FILE *fp = fopen("OSTN02.data", "rb");
  fseek(fp, offset, SEEK_SET);
  fread(&record, 1, 7, fp);
  fclose(fp);
  */
  
  // using embedded data array
  memcpy(&record, &OSTN02Data[offset], 7);
  
  EastingNorthing shifts; 
  if (record.gFlag == (unsigned char) 0) {
    shifts.geoid = 0;
  } else {
    shifts.e         = (((numtype) record.eShift) / 1000.0L) + 86.0L;
    shifts.n         = (((numtype) record.nShift) / 1000.0L) - 82.0L;
    shifts.elevation = (((numtype) record.gShift) / 1000.0L) + 43.0L;
    shifts.geoid     = record.gFlag;
  }
  
  return shifts;
}

EastingNorthing ETRS89EastingNorthingToOSGB36EastingNorthing(const EastingNorthing en) {
  EastingNorthing shifted;
  shifted.e = shifted.n = shifted.elevation = shifted.geoid = 0;
  
  if (en.e < 0.0L || en.e >= 700000.0L || en.n < 0.0L || en.n >= 1250000.0L) return shifted;  
  
  short e0 = (short) (en.e / 1000.0L);
  short n0 = (short) (en.n / 1000.0L);
  cnumtype dx = en.e - (numtype) (e0 * 1000);
  cnumtype dy = en.n - (numtype) (n0 * 1000);
  cnumtype t  = dx / 1000.0L;
  cnumtype u  = dy / 1000.0L;
  
  EastingNorthing shifts0 = OSTN02Shifts(e0    , n0    );
  EastingNorthing shifts1 = OSTN02Shifts(e0 + 1, n0    );
  EastingNorthing shifts2 = OSTN02Shifts(e0 + 1, n0 + 1);
  EastingNorthing shifts3 = OSTN02Shifts(e0    , n0 + 1);
  
  if (shifts0.geoid == 0 || shifts1.geoid == 0 || shifts2.geoid == 0 || shifts3.geoid == 0) return shifted;
  
  cnumtype weight0 = (1.0L - t) * (1.0L - u);
  cnumtype weight1 =         t  * (1.0L - u);
  cnumtype weight2 =         t  *         u;
  cnumtype weight3 = (1.0L - t) *         u;
  
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

LatLonDecimal latLonDecimalFromLatLonDegMinSec(LatLonDegMinSec dms) {
  LatLonDecimal dec;
  dec.lat = (dms.lat.westOrSouth ? -1.0L : 1.0L) * (((numtype) dms.lat.deg) + ((numtype) dms.lat.min) / 60.0L + dms.lat.sec / 3600.0L);
  dec.lon = (dms.lon.westOrSouth ? -1.0L : 1.0L) * (((numtype) dms.lon.deg) + ((numtype) dms.lon.min) / 60.0L + dms.lon.sec / 3600.0L);
  dec.elevation = dms.elevation;
  dec.geoid = 0;
  return dec;
}

void doTests(void) {
  printf("Test MD5: ");
  unsigned char digest[MD5_DIGEST_LENGTH];
  MD5(OSTN02Data, sizeof(OSTN02Data), digest);
  for (char i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", digest[i]);
  printf("\nReal MD5: a1748516214f10799249563a50d7c26d\n\n");
  
  LatLonDegMinSec testETRSCoords[] = {
    {.lat = {.deg = 53, .min = 46, .sec = 44.796925L, .westOrSouth = false}, 
     .lon = {.deg =  3, .min =  2, .sec = 25.637665L, .westOrSouth = true }, .elevation =  64.940L},
    {.lat = {.deg = 51, .min = 25, .sec = 39.170761L, .westOrSouth = false}, 
     .lon = {.deg =  2, .min = 32, .sec = 38.674270L, .westOrSouth = true }, .elevation = 104.018L},
    {.lat = {.deg = 58, .min = 30, .sec = 56.173025L, .westOrSouth = false}, 
     .lon = {.deg =  6, .min = 15, .sec = 39.292403L, .westOrSouth = true }, .elevation = 115.026L},
    {.lat = {.deg = 54, .min = 53, .sec = 43.524259L, .westOrSouth = false},
     .lon = {.deg =  2, .min = 56, .sec = 17.798693L, .westOrSouth = true }, .elevation =  93.541L},
    {.lat = {.deg = 51, .min = 51, .sec = 32.072283L, .westOrSouth = false}, 
     .lon = {.deg =  4, .min = 18, .sec = 30.689158L, .westOrSouth = true }, .elevation =  81.351L},
    {.lat = {.deg = 51, .min = 53, .sec = 39.718951L, .westOrSouth = false}, 
     .lon = {.deg =  0, .min = 53, .sec = 50.075790L, .westOrSouth = false}, .elevation =  75.273L},
    {.lat = {.deg = 53, .min = 20, .sec = 41.290104L, .westOrSouth = false}, 
     .lon = {.deg =  2, .min = 38, .sec = 25.775546L, .westOrSouth = true }, .elevation =  88.411L},
    {.lat = {.deg = 52, .min = 15, .sec = 19.057739L, .westOrSouth = false}, 
     .lon = {.deg =  2, .min =  9, .sec = 16.510138L, .westOrSouth = true }, .elevation = 101.526L},
    {.lat = {.deg = 55, .min = 55, .sec = 29.217549L, .westOrSouth = false}, 
     .lon = {.deg =  3, .min = 17, .sec = 41.251876L, .westOrSouth = true }, .elevation = 119.032L},
    {.lat = {.deg = 54, .min =  7, .sec =  0.665196L, .westOrSouth = false}, 
     .lon = {.deg =  0, .min =  4, .sec = 39.832776L, .westOrSouth = true }, .elevation =  86.778L},
    {.lat = {.deg = 57, .min =  8, .sec = 20.490695L, .westOrSouth = false}, 
     .lon = {.deg =  2, .min =  2, .sec = 54.817138L, .westOrSouth = true }, .elevation = 108.610L},
    {.lat = {.deg = 55, .min = 51, .sec = 14.398307L, .westOrSouth = false}, 
     .lon = {.deg =  4, .min = 17, .sec = 47.364560L, .westOrSouth = true }, .elevation =  71.617L},
    {.lat = {.deg = 57, .min = 29, .sec = 10.500012L, .westOrSouth = false}, 
     .lon = {.deg =  4, .min = 13, .sec =  9.350362L, .westOrSouth = true }, .elevation =  66.178L},
    {.lat = {.deg = 54, .min = 19, .sec = 45.103478L, .westOrSouth = false}, 
     .lon = {.deg =  4, .min = 23, .sec = 18.568248L, .westOrSouth = true }, .elevation =  94.503L},
    {.lat = {.deg = 54, .min =  5, .sec = 11.987451L, .westOrSouth = false}, 
     .lon = {.deg =  4, .min = 38, .sec =  4.278066L, .westOrSouth = true }, .elevation =  84.366L},
    {.lat = {.deg = 52, .min = 45, .sec =  4.920748L, .westOrSouth = false}, 
     .lon = {.deg =  0, .min = 24, .sec =  5.527717L, .westOrSouth = false}, .elevation =  66.431L},
    {.lat = {.deg = 53, .min = 48, .sec =  0.774717L, .westOrSouth = false}, 
     .lon = {.deg =  1, .min = 39, .sec = 49.650033L, .westOrSouth = true }, .elevation = 215.609L},
    {.lat = {.deg = 49, .min = 57, .sec = 36.220979L, .westOrSouth = false}, 
     .lon = {.deg =  5, .min = 12, .sec = 10.965961L, .westOrSouth = true }, .elevation = 124.269L},
    {.lat = {.deg = 51, .min = 29, .sec = 21.716326L, .westOrSouth = false}, 
     .lon = {.deg =  0, .min =  7, .sec = 11.732031L, .westOrSouth = true }, .elevation =  66.057L},
    {.lat = {.deg = 53, .min = 24, .sec = 58.626568L, .westOrSouth = false}, 
     .lon = {.deg =  4, .min = 17, .sec = 21.050495L, .westOrSouth = true }, .elevation = 100.776L},
    {.lat = {.deg = 53, .min = 24, .sec = 58.713306L, .westOrSouth = false}, 
     .lon = {.deg =  4, .min = 17, .sec = 21.040535L, .westOrSouth = true }, .elevation = 100.854L},
    {.lat = {.deg = 57, .min =  0, .sec = 21.841075L, .westOrSouth = false}, 
     .lon = {.deg =  5, .min = 49, .sec = 42.120935L, .westOrSouth = true }, .elevation =  68.494L},
    {.lat = {.deg = 51, .min = 24, .sec =  2.815934L, .westOrSouth = false}, 
     .lon = {.deg =  3, .min = 33, .sec =  4.620554L, .westOrSouth = true }, .elevation = 112.371L},
    {.lat = {.deg = 54, .min = 58, .sec = 44.841864L, .westOrSouth = false}, 
     .lon = {.deg =  1, .min = 36, .sec = 59.676644L, .westOrSouth = true }, .elevation = 125.878L},
    {.lat = {.deg = 51, .min = 22, .sec = 28.092933L, .westOrSouth = false}, 
     .lon = {.deg =  1, .min = 26, .sec = 40.370305L, .westOrSouth = false}, .elevation =  99.439L},
    {.lat = {.deg = 52, .min = 15, .sec =  5.794233L, .westOrSouth = false}, 
     .lon = {.deg =  0, .min = 54, .sec = 44.962452L, .westOrSouth = true }, .elevation = 131.594L},
    {.lat = {.deg = 52, .min = 57, .sec = 43.887942L, .westOrSouth = false}, 
     .lon = {.deg =  1, .min = 11, .sec = 50.915622L, .westOrSouth = true }, .elevation =  93.825L},
    {.lat = {.deg = 50, .min = 55, .sec = 52.605759L, .westOrSouth = false}, 
     .lon = {.deg =  1, .min = 27, .sec =  1.851626L, .westOrSouth = true }, .elevation = 100.405L},
    {.lat = {.deg = 50, .min = 26, .sec = 19.889717L, .westOrSouth = false}, 
     .lon = {.deg =  4, .min =  6, .sec = 31.124303L, .westOrSouth = true }, .elevation = 215.251L},
    {.lat = {.deg = 50, .min = 34, .sec = 32.291946L, .westOrSouth = false}, 
     .lon = {.deg =  1, .min = 17, .sec = 52.161977L, .westOrSouth = true }, .elevation =  94.688L},
    {.lat = {.deg = 59, .min = 51, .sec = 14.756913L, .westOrSouth = false}, 
     .lon = {.deg =  1, .min = 16, .sec = 29.528804L, .westOrSouth = true }, .elevation = 149.890L},
    {.lat = {.deg = 58, .min = 34, .sec = 52.336612L, .westOrSouth = false}, 
     .lon = {.deg =  3, .min = 43, .sec = 34.716767L, .westOrSouth = true }, .elevation =  98.634L},
    {.lat = {.deg = 49, .min = 55, .sec = 20.150196L, .westOrSouth = false}, 
     .lon = {.deg =  6, .min = 17, .sec = 59.199098L, .westOrSouth = true }, .elevation = 100.000L},
    {.lat = {.deg = 53, .min = 20, .sec = 49.312599L, .westOrSouth = false}, 
     .lon = {.deg =  4, .min = 51, .sec =  3.503091L, .westOrSouth = true }, .elevation = 100.000L},
    {.lat = {.deg = 56, .min = 10, .sec = 31.115299L, .westOrSouth = false}, 
     .lon = {.deg =  2, .min = 22, .sec = 31.048596L, .westOrSouth = true }, .elevation = 100.000L},
    {.lat = {.deg = 57, .min = 48, .sec = 48.666318L, .westOrSouth = false}, 
     .lon = {.deg =  8, .min = 34, .sec = 42.760597L, .westOrSouth = true }, .elevation = 100.000L},
    {.lat = {.deg = 58, .min = 12, .sec = 45.440933L, .westOrSouth = false}, 
     .lon = {.deg =  7, .min = 35, .sec = 33.200272L, .westOrSouth = true }, .elevation = 140.404L},
    {.lat = {.deg = 59, .min =  5, .sec = 48.178240L, .westOrSouth = false}, 
     .lon = {.deg =  5, .min = 49, .sec = 40.776272L, .westOrSouth = true }, .elevation = 140.716L},
    {.lat = {.deg = 59, .min =  5, .sec = 36.061263L, .westOrSouth = false}, 
     .lon = {.deg =  4, .min = 25, .sec =  3.276270L, .westOrSouth = true }, .elevation = 100.000L},
    {.lat = {.deg = 60, .min =  7, .sec = 59.091315L, .westOrSouth = false}, 
     .lon = {.deg =  2, .min =  4, .sec = 25.781605L, .westOrSouth = true }, .elevation = 140.716L},
    {.lat = {.deg = 59, .min = 32, .sec =  4.948596L, .westOrSouth = false}, 
     .lon = {.deg =  1, .min = 37, .sec = 30.610770L, .westOrSouth = true }, .elevation = 100.000L},
    {.lat = {.deg = 59, .min =  2, .sec = 14.779356L, .westOrSouth = false}, 
     .lon = {.deg =  3, .min = 12, .sec = 52.344038L, .westOrSouth = true }, .elevation = 100.000L},
    {.lat = {.deg = 58, .min = 43, .sec =  8.173859L, .westOrSouth = false}, 
     .lon = {.deg =  3, .min =  4, .sec = 26.133727L, .westOrSouth = true }, .elevation = 100.000L},
    {.lat = {.deg = 58, .min = 43, .sec = 15.898312L, .westOrSouth = false}, 
     .lon = {.deg =  3, .min =  8, .sec = 16.378343L, .westOrSouth = true }, .elevation = 100.000L},
  };
  
  EastingNorthing testOSGB36Coords[] = {
    {.e = 331534.552L, .n =  431920.792L, .elevation =  12.636L, .geoid =  1},
    {.e = 362269.979L, .n =  169978.688L, .elevation =  54.467L, .geoid =  1},
    {.e = 151968.641L, .n =  966483.777L, .elevation =  58.836L, .geoid =  4},
    {.e = 339921.133L, .n =  556034.759L, .elevation =  41.077L, .geoid =  1},
    {.e = 241124.573L, .n =  220332.638L, .elevation =  27.590L, .geoid =  1},
    {.e = 599445.578L, .n =  225722.824L, .elevation =  30.192L, .geoid =  1},
    {.e = 357455.831L, .n =  383290.434L, .elevation =  36.750L, .geoid =  1},
    {.e = 389544.178L, .n =  261912.151L, .elevation =  51.977L, .geoid =  1},
    {.e = 319188.423L, .n =  670947.532L, .elevation =  66.362L, .geoid =  1},
    {.e = 525745.658L, .n =  470703.211L, .elevation =  41.217L, .geoid =  1},
    {.e = 397160.479L, .n =  805349.734L, .elevation =  58.902L, .geoid =  1},
    {.e = 256340.914L, .n =  664697.266L, .elevation =  17.414L, .geoid =  1},
    {.e = 267056.756L, .n =  846176.969L, .elevation =  13.230L, .geoid =  1},
    {.e = 244780.625L, .n =  495254.884L, .elevation =  39.891L, .geoid =  3},
    {.e = 227778.318L, .n =  468847.386L, .elevation =  29.335L, .geoid =  3},
    {.e = 562180.535L, .n =  319784.993L, .elevation =  20.890L, .geoid =  1},
    {.e = 422242.174L, .n =  433818.699L, .elevation = 165.891L, .geoid =  1},
    {.e = 170370.706L, .n =   11572.404L, .elevation =  71.222L, .geoid =  1},
    {.e = 530624.963L, .n =  178388.461L, .elevation =  20.518L, .geoid =  1},
    {.e = 247958.959L, .n =  393492.906L, .elevation =  46.314L, .geoid =  1},
    {.e = 247959.229L, .n =  393495.580L, .elevation =  46.393L, .geoid =  1},
    {.e = 167634.190L, .n =  797067.142L, .elevation =  13.190L, .geoid =  1},
    {.e = 292184.858L, .n =  168003.462L, .elevation =  60.615L, .geoid =  1},
    {.e = 424639.343L, .n =  565012.700L, .elevation =  76.551L, .geoid =  1},
    {.e = 639821.823L, .n =  169565.856L, .elevation =  55.110L, .geoid =  1},
    {.e = 474335.957L, .n =  262047.752L, .elevation =  83.961L, .geoid =  1},
    {.e = 454002.822L, .n =  340834.941L, .elevation =  45.253L, .geoid =  1},
    {.e = 438710.908L, .n =  114792.248L, .elevation =  54.029L, .geoid =  1},
    {.e = 250359.798L, .n =   62016.567L, .elevation = 163.081L, .geoid =  1},
    {.e = 449816.359L, .n =   75335.859L, .elevation =  48.571L, .geoid =  1},
    {.e = 440725.061L, .n = 1107878.445L, .elevation = 100.991L, .geoid =  6},
    {.e = 299721.879L, .n =   967202.99L, .elevation =  46.011L, .geoid =  1},
    {.e =  91492.135L, .n =   11318.801L, .elevation =  46.882L, .geoid =  2},
    {.e =      0.000L, .n =       0.000L, .elevation =   0.000L, .geoid =  0},
    {.e =      0.000L, .n =       0.000L, .elevation =   0.000L, .geoid =  0},
    {.e =   9587.897L, .n =  899448.993L, .elevation =  43.424L, .geoid =  5},
    {.e =  71713.120L, .n =  938516.401L, .elevation =  83.594L, .geoid =  9},
    {.e = 180862.449L, .n = 1029604.111L, .elevation =  85.197L, .geoid = 10},
    {.e = 261596.767L, .n = 1025447.599L, .elevation =  46.347L, .geoid = 11},
    {.e = 395999.656L, .n = 1138728.948L, .elevation =  89.901L, .geoid = 12},
    {.e = 421300.513L, .n = 1072147.236L, .elevation =  50.951L, .geoid =  8},
    {.e = 330398.311L, .n = 1017347.013L, .elevation =  47.978L, .geoid =  7},
    {.e = 337898.195L, .n =  981746.359L, .elevation =  48.631L, .geoid =  7},
    {.e = 334198.101L, .n =  982046.419L, .elevation =  48.439L, .geoid =  1}
  };
  
  char len = sizeof(testETRSCoords) / sizeof(LatLonDegMinSec);
  for (char i = 0; i < len; i ++) {
    LatLonDecimal testLatLonDec = latLonDecimalFromLatLonDegMinSec(testETRSCoords[i]);
    printf(llFmtStr, "ETRS89     ", testLatLonDec.lat, testLatLonDec.lon, testLatLonDec.elevation, "\n");
    
    EastingNorthing testEN = ETRS89EastingNorthingToOSGB36EastingNorthing(ETRS89LatLonToETRSEastingNorthing(testLatLonDec));
    printf(enFmtStr, "Test OSGB36", testEN.e, testEN.n, testEN.elevation, OSGB36GeoidRegions[testEN.geoid], OSGB36GeoidNames[testEN.geoid], "\n");
    
    EastingNorthing realEN = testOSGB36Coords[i];
    printf(enFmtStr, "Real OSGB36", realEN.e, realEN.n, realEN.elevation, OSGB36GeoidRegions[realEN.geoid], OSGB36GeoidNames[realEN.geoid], "\n\n");    
  }
  
}

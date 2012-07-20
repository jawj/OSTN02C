//
//  OSTN02.c
//  OSTN02
//
//  Created by George MacKerron on 24/12/2011.
//  Copyright (c) 2011 George MacKerron. All rights reserved.
//

#include "OSTN02.h"
#include "dblRelated.h"
#include "crc32.h"
#include "fancyOut.h"
#include "constants.data"
#include "shifts.index.data"
#include "shifts.data"
#include "geoids.data"
#include "gridRef.data"
#include "testCoords.data"
#include "explorerMaps.data"

#define LENGTH_OF(x) (sizeof (x) / sizeof *(x))
#define ASPRINTF_OR_DIE(...) if (asprintf(__VA_ARGS__) < 0) exit(EXIT_FAILURE)

#define originalIndicesCRC 244629328L  // these won't be robust against differing endianness or compiler packing of bit-structs
#define originalDataCRC    790474494L

static CDBL piOver180       = L(0.017453292519943295769236907684886127134428718885417254560971914401710091146034494436822415696345094822);
static CDBL oneEightyOverPi = L(57.29577951308232087679815481410517033240547246656432154916024386120284714832155263244096899585111094418);


DBL gridConvergenceDegreesFromLatLon(const LatLonDecimal latLon, const Ellipsoid ellipsoid, const MapProjection projection) {
  
  CDBL phi     = piOver180 * latLon.lat;
  CDBL lambda  = piOver180 * latLon.lon;
  CDBL lambda0 = piOver180 * projection.trueOriginLatLon.lon;
  
  CDBL a   = ellipsoid.semiMajorAxis;
  CDBL b   = ellipsoid.semiMinorAxis;
  CDBL f0  = projection.centralMeridianScale;
  
  CDBL deltaLambda  = lambda - lambda0;
  CDBL deltaLambda2 = deltaLambda  * deltaLambda;
  CDBL deltaLambda3 = deltaLambda2 * deltaLambda;
  CDBL deltaLambda5 = deltaLambda3 * deltaLambda2;
  
  CDBL sinPhi   = SIN(phi);
  CDBL sinPhi2  = sinPhi * sinPhi;
  CDBL cosPhi   = COS(phi);
  CDBL cosPhi2  = cosPhi * cosPhi;
  CDBL cosPhi4  = cosPhi2 * cosPhi2;
  CDBL tanPhi   = TAN(phi);
  CDBL tanPhi2  = tanPhi * tanPhi;
  
  CDBL af0  = a * f0;
  CDBL af02 = af0 * af0;
  CDBL bf0  = b * f0;
  CDBL bf02 = bf0 * bf0;
  
  CDBL e2    = (af02 - bf02) / af02;
  CDBL nu    = af0 / SQRT(L(1.0) - (e2 * sinPhi2));
  CDBL rho   = (nu * (L(1.0) - e2)) / (L(1.0) - (e2 * sinPhi2));
  CDBL eta2  = (nu / rho) - L(1.0);
  CDBL xiv   = ((sinPhi * cosPhi2) / L(3.0))  * (L(1.0) + L(3.0) * eta2 + L(2.0) * eta2 * eta2);
  CDBL xv    = ((sinPhi * cosPhi4) / L(15.0)) * (L(2.0) - tanPhi2);
  
  CDBL cRads = (deltaLambda * sinPhi) + (deltaLambda3 * xiv) + (deltaLambda5 * xv);
  return oneEightyOverPi * cRads;
}


DBL gridConvergenceDegreesFromEastingNorthing(const EastingNorthing en, const Ellipsoid ellipsoid, const MapProjection projection) {
  
  LatLonDecimal latLon = latLonFromEastingNorthing(en, ellipsoid, projection);
  CDBL phi     = piOver180 * latLon.lat;
  CDBL lambda  = piOver180 * latLon.lon;
  CDBL lambda0 = piOver180 * projection.trueOriginLatLon.lon;
  
  CDBL a   = ellipsoid.semiMajorAxis;
  CDBL b   = ellipsoid.semiMinorAxis;
  CDBL f0  = projection.centralMeridianScale;
  
  CDBL deltaLambda  = lambda - lambda0;
  CDBL deltaLambda2 = deltaLambda  * deltaLambda;
  CDBL deltaLambda3 = deltaLambda2 * deltaLambda;
  CDBL deltaLambda5 = deltaLambda3 * deltaLambda2;
  
  CDBL sinPhi   = SIN(phi);
  CDBL sinPhi2  = sinPhi * sinPhi;
  CDBL cosPhi   = COS(phi);
  CDBL cosPhi2  = cosPhi * cosPhi;
  CDBL cosPhi4  = cosPhi2 * cosPhi2;
  CDBL tanPhi   = TAN(phi);
  CDBL tanPhi2  = tanPhi * tanPhi;
  
  CDBL af0  = a * f0;
  CDBL af02 = af0 * af0;
  CDBL bf0  = b * f0;
  CDBL bf02 = bf0 * bf0;
  
  CDBL e2    = (af02 - bf02) / af02;
  CDBL nu    = af0 / SQRT(L(1.0) - (e2 * sinPhi2));
  CDBL rho   = (nu * (L(1.0) - e2)) / (L(1.0) - (e2 * sinPhi2));
  CDBL eta2  = (nu / rho) - L(1.0);
  CDBL xiv   = ((sinPhi * cosPhi2) / L(3.0))  * (L(1.0) + L(3.0) * eta2 + L(2.0) * eta2 * eta2);
  CDBL xv    = ((sinPhi * cosPhi4) / L(15.0)) * (L(2.0) - tanPhi2);
  
  CDBL cRads = (deltaLambda * sinPhi) + (deltaLambda3 * xiv) + (deltaLambda5 * xv);
  return oneEightyOverPi * cRads;
}

int nextOSExplorerMap(EastingNorthing en, int prevMap) {
  for (int i = prevMap + 1, len = LENGTH_OF(OSExplorerMaps); i < len; i ++) {
    OSMap map = OSExplorerMaps[i];
    if (   en.e >= (DBL) map.emin 
        && en.e <= (DBL) map.emax 
        && en.n >= (DBL) map.nmin 
        && en.n <= (DBL) map.nmax) return i;
  }
  return -1;
}

EastingNorthing eastingNorthingFromLatLon(const LatLonDecimal latLon, const Ellipsoid ellipsoid, const MapProjection projection) {
  CDBL a   = ellipsoid.semiMajorAxis;
  CDBL b   = ellipsoid.semiMinorAxis;
  CDBL f0  = projection.centralMeridianScale;
  CDBL e0  = projection.trueOriginEastingNorthing.e;
  CDBL n0  = projection.trueOriginEastingNorthing.n;
  
  CDBL phi     = piOver180 * latLon.lat;
  CDBL lambda  = piOver180 * latLon.lon;
  CDBL phi0    = piOver180 * projection.trueOriginLatLon.lat;
  CDBL lambda0 = piOver180 * projection.trueOriginLatLon.lon;
  
  CDBL deltaPhi = phi - phi0;
  CDBL sumPhi   = phi + phi0;
  CDBL sinPhi   = SIN(phi);
  CDBL sinPhi2  = sinPhi * sinPhi;
  CDBL cosPhi   = COS(phi);
  CDBL cosPhi2  = cosPhi * cosPhi;
  CDBL cosPhi3  = cosPhi2 * cosPhi;
  CDBL cosPhi5  = cosPhi3 * cosPhi2;
  CDBL tanPhi   = TAN(phi);
  CDBL tanPhi2  = tanPhi * tanPhi;
  CDBL tanPhi4  = tanPhi2 * tanPhi2;
  
  CDBL a2   = a * a;
  CDBL e2   = (a2 - b * b) / a2;
  CDBL n    = (a - b) / (a + b);
  CDBL n2   = n * n;
  CDBL n3   = n2 * n;
  CDBL oneMinusE2SinPhi2 = L(1.0) - e2 * sinPhi2;
  CDBL sqrtOneMinusE2SinPhi2 = SQRT(oneMinusE2SinPhi2);
  CDBL v    = a * f0 / sqrtOneMinusE2SinPhi2;
  CDBL rho  = a * f0 * (L(1.0) - e2) / (oneMinusE2SinPhi2 * sqrtOneMinusE2SinPhi2);
  CDBL eta2 = v / rho - L(1.0);
  CDBL m    = b * f0 * ( (L(1.0) + n + (L(5.0) / L(4.0)) * n2 + (L(5.0) / L(4.0)) * n3) * deltaPhi
                       - (L(3.0) * n + L(3.0) * n2 + (L(21.0) / L(8.0)) * n3) * SIN(deltaPhi) * COS(sumPhi)
                       + ((L(15.0) / L(8.0)) * n2 + (L(15.0) / L(8.0)) * n3) * SIN(L(2.0) * deltaPhi) * COS(L(2.0) * sumPhi)
                       - (L(35.0) / L(24.0)) * n3 * SIN(L(3.0) * deltaPhi) * COS(L(3.0) * sumPhi) 
                       );
  
  CDBL one    = m + n0;
  CDBL two    = (v /   L(2.0)) * sinPhi * cosPhi;
  CDBL three  = (v /  L(24.0)) * sinPhi * cosPhi3 * (L(5.0) - tanPhi2 + L(9.0) * eta2);
  CDBL threeA = (v / L(720.0)) * sinPhi * cosPhi5 * (L(61.0) - L(58.0) * tanPhi2 + tanPhi4);
  CDBL four   = v * cosPhi;
  CDBL five   = (v /   L(6.0)) * cosPhi3 * (v / rho - tanPhi2);
  CDBL six    = (v / L(120.0)) * cosPhi5 * (L(5.0) - L(18.0) * tanPhi2 + tanPhi4 + L(14.0) * eta2 - L(58.0) * tanPhi2 * eta2);
  
  CDBL deltaLambda  = lambda - lambda0;
  CDBL deltaLambda2 = deltaLambda  * deltaLambda;
  CDBL deltaLambda3 = deltaLambda2 * deltaLambda;
  CDBL deltaLambda4 = deltaLambda3 * deltaLambda;
  CDBL deltaLambda5 = deltaLambda4 * deltaLambda;
  CDBL deltaLambda6 = deltaLambda5 * deltaLambda;
  
  EastingNorthing en;
  en.n = one + two * deltaLambda2 + three * deltaLambda4 + threeA * deltaLambda6;
  en.e = e0 + four * deltaLambda + five * deltaLambda3 + six * deltaLambda5;
  en.elevation = latLon.elevation;
  en.geoid = latLon.geoid;
  
  return en;
}

EastingNorthing ETRS89EastingNorthingFromETRS89LatLon(const LatLonDecimal latLon) {
  return eastingNorthingFromLatLon(latLon, GRS80Ellipsoid, NationalGridProj);
}

LatLonDecimal latLonFromEastingNorthing(const EastingNorthing en, const Ellipsoid ellipsoid, const MapProjection projection) {
  CDBL a   = ellipsoid.semiMajorAxis;
  CDBL b   = ellipsoid.semiMinorAxis;
  CDBL f0  = projection.centralMeridianScale;
  CDBL e0  = projection.trueOriginEastingNorthing.e;
  CDBL n0  = projection.trueOriginEastingNorthing.n;
  CDBL n   = (a - b) / (a + b);
  CDBL n2  = n * n;
  CDBL n3  = n2 * n;
  CDBL bf0 = b * f0;
  
  CDBL phi0    = piOver180 * projection.trueOriginLatLon.lat;
  CDBL lambda0 = piOver180 * projection.trueOriginLatLon.lon;
  
  DBL phi = phi0;  // this is phi' in the OS docs
  DBL m = L(0.0);
  DBL deltaPhi, sumPhi;
  do {
    phi      = (en.n - n0 - m) / (a * f0) + phi;
    deltaPhi = phi - phi0;
    sumPhi   = phi + phi0;
    m        = bf0 * ( (L(1.0) + n + (L(5.0) / L(4.0)) * n2 + (L(5.0) / L(4.0)) * n3) * deltaPhi
                     - (L(3.0) * n + L(3.0) * n2 + (L(21.0) / L(8.0)) * n3) * SIN(deltaPhi) * COS(sumPhi)
                     + ((L(15.0) / L(8.0)) * n2 + (L(15.0) / L(8.0)) * n3) * SIN(L(2.0) * deltaPhi) * COS(L(2.0) * sumPhi)
                     - (L(35.0) / L(24.0)) * n3 * SIN(L(3.0) * deltaPhi) * COS(L(3.0) * sumPhi)
                     );
  } while (ABS(en.n - n0 - m) >= L(0.00001));
  
  CDBL sinPhi   = SIN(phi);
  CDBL sinPhi2  = sinPhi * sinPhi;
  CDBL cosPhi   = COS(phi);
  CDBL secPhi   = L(1.0) / cosPhi;
  CDBL tanPhi   = TAN(phi);
  CDBL tanPhi2  = tanPhi * tanPhi;
  CDBL tanPhi4  = tanPhi2 * tanPhi2;
  CDBL tanPhi6  = tanPhi4 * tanPhi2;
  
  CDBL a2   = a * a;
  CDBL e2   = (a2 - b * b) / a2;
  CDBL oneMinusE2SinPhi2 = L(1.0) - e2 * sinPhi2;
  CDBL sqrtOneMinusE2SinPhi2 = SQRT(oneMinusE2SinPhi2);
  CDBL v    = a * f0 / sqrtOneMinusE2SinPhi2;
  CDBL rho  = a * f0 * (L(1.0) - e2) / (oneMinusE2SinPhi2 * sqrtOneMinusE2SinPhi2);
  CDBL eta2 = v / rho - L(1.0);
  
  CDBL v2 = v  * v;
  CDBL v3 = v2 * v;
  CDBL v5 = v3 * v2;
  CDBL v7 = v5 * v2;
  
  CDBL seven   = tanPhi / (L(2.0) * rho * v);
  CDBL eight   = (tanPhi * (L(5.0) + L(3.0) * tanPhi2 + eta2 - L(9.0) * tanPhi2 * eta2)) / (L(24.0) * rho * v3);
  CDBL nine    = (tanPhi * (L(61.0) + L(90.0) * tanPhi2 + L(45.0) * tanPhi4)) / (L(720.0) * rho * v5);
  CDBL ten     = secPhi / v;
  CDBL eleven  = (secPhi * ((v / rho) + L(2.0) * tanPhi2)) / (L(6.0) * v3);
  CDBL twelve  = (secPhi * (L(5.0) + L(28.0) * tanPhi2 + L(24.0) * tanPhi4)) / (L(120.0) * v5);
  CDBL twelveA = (secPhi * (L(61.0) + L(662.0) * tanPhi2 + L(1320.0) * tanPhi4 + L(720.0) * tanPhi6)) / (L(5040.0) * v7);
  
  CDBL deltaE  = en.e - e0;
  CDBL deltaE2 = deltaE  * deltaE;
  CDBL deltaE3 = deltaE2 * deltaE;
  CDBL deltaE4 = deltaE2 * deltaE2;
  CDBL deltaE5 = deltaE3 * deltaE2;
  CDBL deltaE6 = deltaE3 * deltaE3;
  CDBL deltaE7 = deltaE4 * deltaE3;
  
  LatLonDecimal latLon;
  latLon.lat = oneEightyOverPi * (phi - seven * deltaE2 + eight * deltaE4 - nine * deltaE6);
  latLon.lon = oneEightyOverPi * (lambda0 + ten * deltaE - eleven * deltaE3 + twelve * deltaE5 - twelveA * deltaE7);
  latLon.elevation = en.elevation;
  latLon.geoid = en.geoid;
  
  return latLon;
}

LatLonDecimal ETRS89LatLonFromETRS89EastingNorthing(const EastingNorthing en) {
  return latLonFromEastingNorthing(en, GRS80Ellipsoid, NationalGridProj);
}

EastingNorthing OSTN02ShiftsForIndices(const int eIndex, const int nIndex) {
  EastingNorthing shifts;
  shifts.e = shifts.n = shifts.elevation = shifts.geoid = 0;
  if (nIndex < 0 || nIndex > 1250) return shifts;

  const OSTN02Index dataIndex = OSTN02Indices[nIndex];
  if (eIndex < dataIndex.eMin || eIndex >= dataIndex.eMin + dataIndex.eCount) return shifts;
  
  const unsigned int dataOffset = dataIndex.offset + (eIndex - dataIndex.eMin);
  const OSTN02Datum record = OSTN02Data[dataOffset];
  if (record.gFlag == 0) return shifts;
  
  shifts.e         = (((DBL) record.eShift) / L(1000.0)) + L(86.0);
  shifts.n         = (((DBL) record.nShift) / L(1000.0)) - L(82.0);
  shifts.elevation = (((DBL) record.gShift) / L(1000.0)) + L(43.0);
  shifts.geoid     = record.gFlag;
  return shifts;
}

EastingNorthing shiftsForEastingNorthing(const EastingNorthing en) {
  EastingNorthing shifts;
  shifts.e = shifts.n = shifts.elevation = shifts.geoid = 0;
  
  const int e0 = (int) (en.e / L(1000.0));
  const int n0 = (int) (en.n / L(1000.0));
  CDBL      dx = en.e - (DBL) (e0 * 1000);
  CDBL      dy = en.n - (DBL) (n0 * 1000);
  CDBL      t  = dx / L(1000.0);
  CDBL      u  = dy / L(1000.0);
  
  const EastingNorthing shifts0 = OSTN02ShiftsForIndices(e0    , n0    );
  if (shifts0.geoid == 0) return shifts;
  
  const EastingNorthing shifts1 = OSTN02ShiftsForIndices(e0 + 1, n0    );
  if (shifts1.geoid == 0) return shifts;
  
  const EastingNorthing shifts2 = OSTN02ShiftsForIndices(e0 + 1, n0 + 1);
  if (shifts2.geoid == 0) return shifts;
  
  const EastingNorthing shifts3 = OSTN02ShiftsForIndices(e0    , n0 + 1);
  if (shifts3.geoid == 0) return shifts;
  
  CDBL weight0 = (L(1.0) - t) * (L(1.0) - u);
  CDBL weight1 =           t  * (L(1.0) - u);
  CDBL weight2 =           t  *           u;
  CDBL weight3 = (L(1.0) - t) *           u;
  
  shifts.e         = weight0 * shifts0.e
                   + weight1 * shifts1.e 
                   + weight2 * shifts2.e 
                   + weight3 * shifts3.e;
  
  shifts.n         = weight0 * shifts0.n 
                   + weight1 * shifts1.n
                   + weight2 * shifts2.n 
                   + weight3 * shifts3.n;
  
  shifts.elevation = weight0 * shifts0.elevation 
                   + weight1 * shifts1.elevation
                   + weight2 * shifts2.elevation
                   + weight3 * shifts3.elevation;
  
  const bool left   = dx < L(500.0);
  const bool bottom = dy < L(500.0);
  const EastingNorthing nearestShift = left ? (bottom ? shifts0 : shifts3) : (bottom ? shifts1 : shifts2);
  shifts.geoid = nearestShift.geoid;
  
  return shifts;
}

EastingNorthing OSGB36EastingNorthingFromETRS89EastingNorthing(const EastingNorthing en) {
  EastingNorthing shifts = shiftsForEastingNorthing(en);
  if (shifts.geoid == 0) return shifts;
  
  EastingNorthing shifted;
  shifted.e = en.e + shifts.e;
  shifted.n = en.n + shifts.n;
  shifted.elevation = en.elevation - shifts.elevation;
  shifted.geoid = shifts.geoid;
  
  return shifted;
}

EastingNorthing ETRS89EastingNorthingFromOSGB36EastingNorthing(const EastingNorthing en) {
  EastingNorthing shifts, prevShifts, shifted;
  shifts.e = shifts.n = shifted.elevation = shifted.geoid = 0;  // initialising .elevation and .geoid just avoids warnings
  do {
    prevShifts.e = shifts.e;
    prevShifts.n = shifts.n;
    shifted.e = en.e - shifts.e;
    shifted.n = en.n - shifts.n;
    shifts = shiftsForEastingNorthing(shifted);
    if (shifts.geoid == 0) return shifts;
  } while (ABS(shifts.e - prevShifts.e) > L(0.0001) || ABS(shifts.n - prevShifts.n) > L(0.0001));
  
  shifted.elevation = en.elevation + shifts.elevation;
  shifted.geoid = shifts.geoid;  // tells us which geoid datum was used in conversion
  return shifted;
}

char *gridRefFromOSGB36EastingNorthing(const EastingNorthing en, const bool spaces, const int res) { 
  // res is expressed in metres: 1/10/100 -> 3/4/5-digit easting and northing
  const int  eRound = (int) round(en.e / (DBL) res) * res;
  const int  nRound = (int) round(en.n / (DBL) res) * res;
  const int  firstEIndex  = eRound / 500000;
  const int  firstNIndex  = nRound / 500000;
  const int  secondEIndex = (eRound % 500000) / 100000;
  const int  secondNIndex = (nRound % 500000) / 100000;
  const char sq0 = firstLetters[firstNIndex][firstEIndex];
  const char sq1 = secondLetters[secondNIndex][secondEIndex];
  const int  e   = eRound - (500000 * firstEIndex) - (100000 * secondEIndex);
  const int  n   = nRound - (500000 * firstNIndex) - (100000 * secondNIndex);
  char *ref, *fmtStr;
  const int digits = res == 100 ? 3 : (res == 10 ? 4 : 5);
  ASPRINTF_OR_DIE(&fmtStr, "%%c%%c%%s%%0%dd%%s%%0%dd", digits, digits);
  ASPRINTF_OR_DIE(&ref, fmtStr, sq0, sq1, (spaces ? " " : ""), e / res, (spaces ? " " : ""), n / res);
  free(fmtStr);
  return ref;
}

char *tetradFromOSGB36EastingNorthing(const EastingNorthing en) {
  // see http://www.bto.org/volunteer-surveys/birdatlas/taking-part/correct-grid-references/know-your-place
  const int  eTrunc = (int) en.e;  // note: unlike for a grid ref, we never round -- we (implictly) truncate
  const int  nTrunc = (int) en.n;
  const int  firstEIndex  = eTrunc / 500000;
  const int  firstNIndex  = nTrunc / 500000;
  const int  secondEIndex = (eTrunc % 500000) / 100000;
  const int  secondNIndex = (nTrunc % 500000) / 100000;
  const char sq0 = firstLetters[firstNIndex][firstEIndex];
  const char sq1 = secondLetters[secondNIndex][secondEIndex];
  const int  eMinusSq = eTrunc - (500000 * firstEIndex) - (100000 * secondEIndex);
  const int  nMinusSq = nTrunc - (500000 * firstNIndex) - (100000 * secondNIndex);
  const int  eDigit = eMinusSq / 10000;
  const int  nDigit = nMinusSq / 10000;
  const int  tetradEIndex = (eMinusSq % 10000) / 2000;
  const int  tetradNIndex = (nMinusSq % 10000) / 2000;
  char tetradLetter = tetradLetters[tetradNIndex][tetradEIndex];
  char *tetrad;
  ASPRINTF_OR_DIE(&tetrad, "%c%c%d%d%c", sq0, sq1, eDigit, nDigit, tetradLetter);
  return tetrad;
}

LatLonDecimal latLonDecimalFromLatLonDegMinSec(const LatLonDegMinSec dms) {
  LatLonDecimal dec;
  dec.lat = (dms.lat.westOrSouth ? L(-1.0) : L(1.0)) * (((DBL) dms.lat.deg) + ((DBL) dms.lat.min) / L(60.0) + dms.lat.sec / L(3600.0));
  dec.lon = (dms.lon.westOrSouth ? L(-1.0) : L(1.0)) * (((DBL) dms.lon.deg) + ((DBL) dms.lon.min) / L(60.0) + dms.lon.sec / L(3600.0));
  dec.elevation = dms.elevation;
  dec.geoid = 0;
  return dec;
}

bool test(const bool noisily) {
  short numTested = 0;
  short numPassed = 0;
  bool  testPassed;
  
  // check data integrity
  
  const unsigned long dataCRC = crc32((unsigned char *) OSTN02Data, sizeof(OSTN02Data));
  testPassed = dataCRC == originalDataCRC;
  numTested ++;
  if (testPassed) numPassed ++;
  if (noisily) {
    printf("\nOriginal CRC32 (data):  %li\n", originalDataCRC);
    printf("%sComputed CRC32 (data):  %li%s\n\n", (testPassed ? "" : BOLD), dataCRC, (testPassed ? "" : UNBOLD));
  }
  
  const unsigned long indicesCRC = crc32((unsigned char *) OSTN02Indices, sizeof(OSTN02Indices));
  testPassed = indicesCRC == originalIndicesCRC;
  numTested ++;
  if (testPassed) numPassed ++;
  if (noisily) {
    printf("Original CRC32 (index): %li\n", originalIndicesCRC);
    printf("%sComputed CRC32 (index): %li%s\n\n", (testPassed ? "" : BOLD), indicesCRC, (testPassed ? "" : UNBOLD));
  }
  
  // check test conversions against known good results
  
  LatLonDecimal actualLatLon, computedLatLon;
  EastingNorthing actualEN, computedEN;
  char *actualLatLonStr, *computedLatLonStr, *actualENStr, *computedENStr;
  
  const int len = LENGTH_OF(testETRSCoords);
  for (int i = 0; i < len; i ++) {
    
    // actual coords
    
    actualLatLon = latLonDecimalFromLatLonDegMinSec(testETRSCoords[i]);
    actualEN     = testOSGB36Coords[i];
    
    ASPRINTF_OR_DIE(&actualLatLonStr, LLFMT, actualLatLon.lat, actualLatLon.lon, actualLatLon.elevation);
    ASPRINTF_OR_DIE(&actualENStr,     ENFMT, actualEN.e,       actualEN.n,       actualEN.elevation, OSGB36GeoidRegions[actualEN.geoid], OSGB36GeoidNames[actualEN.geoid]);
    
    if (noisily) {
      printf("ETRS89 actual   %s\n",   actualLatLonStr);
      printf("OSGB36 actual   %s\n",   actualENStr);
    }
    
    // computed coords
    
    computedLatLon = ETRS89LatLonFromETRS89EastingNorthing(ETRS89EastingNorthingFromOSGB36EastingNorthing(actualEN));
    computedEN     = OSGB36EastingNorthingFromETRS89EastingNorthing(ETRS89EastingNorthingFromETRS89LatLon(actualLatLon));
    
    ASPRINTF_OR_DIE(&computedLatLonStr, LLFMT, computedLatLon.lat, computedLatLon.lon, computedLatLon.elevation);
    ASPRINTF_OR_DIE(&computedENStr,     ENFMT, computedEN.e,       computedEN.n,       computedEN.elevation, OSGB36GeoidRegions[computedEN.geoid], OSGB36GeoidNames[computedEN.geoid]);
    
    if (actualEN.geoid != 0) {  // i.e. these coordinates are not zeroes, and can be converted sensibly
      testPassed = strcmp(actualLatLonStr, computedLatLonStr) == 0;
      numTested ++;
      if (testPassed) numPassed ++;
      if (noisily) printf("%sETRS89 computed %s\n%s", (testPassed ? "" : BOLD), computedLatLonStr, (testPassed ? "" : UNBOLD));
    }
    
    testPassed = strcmp(actualENStr, computedENStr) == 0;
    numTested ++;
    if (testPassed) numPassed ++;
    if (noisily) printf("%sOSGB36 computed %s\n\n%s", (testPassed ? "" : BOLD), computedENStr, (testPassed ? "" : UNBOLD));
    
    free(actualLatLonStr);
    free(computedLatLonStr);
    free(actualENStr);
    free(computedENStr);
  }
  
  /*
  EastingNorthing testEN;
  testEN.e = 651409.903;
  testEN.n = 313177.270;
  printf("Convergence: %13.11lf \n\n", gridConvergenceDegreesFromEastingNorthing(testEN, Airy1830Ellipsoid, NationalGridProj));
  */
  
  /*
  LatLonDecimal testLL;
  testLL.lat = 51.0;
  testLL.lon = -2.0;
  printf("Convergence: %13.11lf \n\n", gridConvergenceDegreesFromLatLon(testLL, Airy1830Ellipsoid, NationalGridProj));
  */
  
  bool allPassed = numTested == numPassed;
  if (noisily) printf("%i tests; %i passed; %s%i failed%s\n\n", numTested, numPassed, (allPassed ? "" : BOLD), numTested - numPassed, (allPassed ? "" : UNBOLD));
  return allPassed;
}

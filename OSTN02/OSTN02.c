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
#include "testConvergences.data"
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
  CDBL n0   = projection.trueOriginEastingNorthing.n;
  CDBL e0   = projection.trueOriginEastingNorthing.e;
  CDBL f0   = projection.centralMeridianScale;
  CDBL af0  = ellipsoid.semiMajorAxis * f0;
  CDBL bf0  = ellipsoid.semiMinorAxis * f0;
  
  CDBL af02 = af0 * af0;
  CDBL bf02 = bf0 * bf0;
  
  CDBL n    = (af0 - bf0) / (af0 + bf0);
  CDBL n2   = n * n;
  CDBL n3   = n2 * n;
  CDBL e2   = (af02 - bf02) / af02;
  
  CDBL phi0 = piOver180 * projection.trueOriginLatLon.lat;
  DBL phi = (en.n - n0) / af0 + phi0;
  
  while (true) {
    DBL deltaPhi = phi - phi0;
    DBL sumPhi = phi + phi0;
    DBL j3 = (L(1.0) + n + L(5.0) / L(4.0) * n2 + L(5.0) / L(4.0) * n3) * deltaPhi;
    DBL j4 = (L(3.0) * n + L(3.0) * n2 + L(21.0)/L(8.0) * n3) * SIN(deltaPhi) * COS(sumPhi);
    DBL j5 = (L(15.0) / L(8.0) * n2 + L(15.0) / L(8.0) * n3) * SIN(L(2.0) * deltaPhi) * COS(L(2.0) * sumPhi);
    DBL j6 = L(35.0) / L(24.0) * n3 * SIN(L(3.0) * deltaPhi) * COS(L(3.0) * sumPhi);
    DBL M = bf0 * (j3 - j4 + j5 - j6);
    if (ABS(en.n - n0 - M) < L(0.001)) break;
    phi += (en.n - n0 - M) / af0;
  };
  
  CDBL sinPhi = SIN(phi);
  CDBL sinPhi2 = sinPhi * sinPhi;
  CDBL nu = af0 / SQRT(L(1.0) - e2 * sinPhi2);
  CDBL rho = nu * (L(1.0) - e2) / (L(1.0) - e2 * sinPhi2);
  CDBL eta2 = nu / rho - L(1.0);
  CDBL eta4 = eta2 * eta2;
  
  CDBL nu2 = nu * nu;
  CDBL nu3 = nu2 * nu;
  CDBL nu5 = nu3 * nu2;
  CDBL tanPhi = TAN(phi);
  CDBL tanPhi2 = tanPhi * tanPhi;
  CDBL tanPhi4 = tanPhi2 * tanPhi2;
  
  CDBL y = en.e - e0;
  CDBL y3 = y * y * y;
  CDBL y5 = y3 * y * y;
  
  CDBL j31 = tanPhi / nu;
  CDBL j41 = tanPhi / (L(3.0) * nu3) * (L(1.0) + tanPhi2 - eta2 - L(2.0) * eta4);
  CDBL j51 = tanPhi / (L(15.0) * nu5) * (L(2.0) + L(5.0) * tanPhi2 + L(3.0) * tanPhi4);
  
  CDBL c = y * j31 - y3 * j41 + y5 * j51;
  
  return oneEightyOverPi * c;
}

DBL gridConvergenceDegreesFromOSGB36EastingNorthing(const EastingNorthing en) {
  return gridConvergenceDegreesFromEastingNorthing(en, Airy1830Ellipsoid, NationalGridProj);
}

DBL gridConvergenceDegreesFromETRS89LatLon(const LatLonDecimal latLon) {
  return gridConvergenceDegreesFromLatLon(latLon, GRS80Ellipsoid, NationalGridProj);
}

int nextOSExplorerMap(EastingNorthing en, int prevMap) {  // start with prevMap = -1
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

GridRefComponents gridRefComponentsFromOSGB36EastingNorthing(const EastingNorthing en, const int res) {
  // res is expressed in metres: 1/10/100/1000 -> components to use in generating 5/4/3/2-digit eastings and northings
  GridRefComponents grc = {0};
  if (en.geoid == 0) return grc;
  const int eRound = (int) round(en.e / (DBL)res) * res;
  const int nRound = (int) round(en.n / (DBL)res) * res;
  const int firstEIndex  = eRound / 500000;
  const int firstNIndex  = nRound / 500000;
  const int secondEIndex = (eRound % 500000) / 100000;
  const int secondNIndex = (nRound % 500000) / 100000;
  grc.letters[0] = firstLetters[firstNIndex][firstEIndex];
  grc.letters[1] = secondLetters[secondNIndex][secondEIndex];
  grc.e = eRound - (500000 * firstEIndex) - (100000 * secondEIndex);
  grc.n = nRound - (500000 * firstNIndex) - (100000 * secondNIndex);
  grc.resolution = res;
  return grc;
}

char* gridRefFromGridRefComponents(const GridRefComponents grc, const bool spaces) {
  char *ref, *fmtStr;
  const int res = grc.resolution;
  if (res == 0) exit(EXIT_FAILURE);  // keep clang static analysis quiet
  const int digits = res == 1000 ? 2 : res == 100 ? 3 : res == 10 ? 4 : 5;
  ASPRINTF_OR_DIE(&fmtStr, "%%c%%c%%s%%0%dd%%s%%0%dd", digits, digits);
  ASPRINTF_OR_DIE(&ref, fmtStr, grc.letters[0], grc.letters[1], (spaces ? " " : ""), grc.e / res, (spaces ? " " : ""), grc.n / res);
  free(fmtStr);
  return ref;
}

char* gridRefFromOSGB36EastingNorthing(const EastingNorthing en, const int res, const bool spaces) {
  GridRefComponents grc = gridRefComponentsFromOSGB36EastingNorthing(en, res);
  char* gridRef = gridRefFromGridRefComponents(grc, spaces);
  return gridRef;
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

DegMinSec degMinSecFromDecimal(DBL dec) {
  DegMinSec dms;
  dms.westOrSouth = dec < L(0.0) ? true : false;
  dec = ABS(dec);
  dms.deg = (int)FLOOR(dec);
  dec -= dms.deg;
  dms.min = (int)FLOOR(L(60.0) * dec);
  dec -= dms.min / L(60.0);
  dms.sec = dec * L(3600.0);
  return dms;
}

DBL decimalFromDegMinSec(DegMinSec dms) {
  return (dms.westOrSouth ? L(-1.0) : L(1.0)) * (((DBL) dms.deg) + ((DBL) dms.min) / L(60.0) + dms.sec / L(3600.0));
}

LatLonDecimal latLonDecimalFromLatLonDegMinSec(const LatLonDegMinSec dms) {
  LatLonDecimal dec;
  dec.lat = decimalFromDegMinSec(dms.lat);
  dec.lon = decimalFromDegMinSec(dms.lon);
  dec.elevation = dms.elevation;
  dec.geoid = dms.geoid;
  return dec;
}

LatLonDegMinSec latLonDegMinSecFromLatLonDecimal(const LatLonDecimal dec) {
  LatLonDegMinSec dms;
  dms.lat = degMinSecFromDecimal(dec.lat);
  dms.lon = degMinSecFromDecimal(dec.lon);
  dms.elevation = dec.elevation;
  dms.geoid = dec.geoid;
  return dms;
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
  
  int len;
  
  // OSTN02 conversions
  
  LatLonDecimal actualLatLon, computedLatLon;
  EastingNorthing actualEN, computedEN;
  char *actualLatLonStr, *computedLatLonStr, *actualENStr, *computedENStr;
  
  len = LENGTH_OF(testETRSCoords);
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
    if (noisily) printf("%sOSGB36 computed %s%s\n\n", (testPassed ? "" : BOLD), computedENStr, (testPassed ? "" : UNBOLD));
    
    free(actualLatLonStr);
    free(computedLatLonStr);
    free(actualENStr);
    free(computedENStr);
  }
  
  // convergences by Easting/Northing
  
  EastingNorthing convergenceEN;
  DegMinSec actualC, computedC;
  char *actualCStr, *computedCStr;
  
  len = LENGTH_OF(testConvergenceOSGB36Coords);
  for (int i = 0; i < len; i ++) {
    
    // actual coords
    
    convergenceEN = testConvergenceOSGB36Coords[i];
    actualC = testConvergencesFromOSGB36Coords[i];
    ASPRINTF_OR_DIE(&actualCStr, "%i\u00b0 %02i\u2032 %.4f\u2033 %c", actualC.deg, actualC.min, actualC.sec, actualC.westOrSouth ? 'W' : 'E');
    if (noisily) printf("Convergence reference from E/N  %s\n", actualCStr);
    
    // computed coords
    
    computedC = degMinSecFromDecimal(gridConvergenceDegreesFromOSGB36EastingNorthing(convergenceEN));
    ASPRINTF_OR_DIE(&computedCStr, "%i\u00b0 %02i\u2032 %.4f\u2033 %c", computedC.deg, computedC.min, computedC.sec, computedC.westOrSouth ? 'W' : 'E');
    
    testPassed = strcmp(actualCStr, computedCStr) == 0;
    numTested ++;
    if (testPassed) numPassed ++;
    if (noisily) printf("%sConvergence computed from E/N   %s%s\n\n", (testPassed ? "" : BOLD), computedCStr, (testPassed ? "" : UNBOLD));
    
    free(actualCStr);
    free(computedCStr);
  }
  
  // convergences by lat/lon
  
  LatLonDecimal convergenceLatLon;
  
  len = LENGTH_OF(testConvergenceLatLons);
  for (int i = 0; i < len; i ++) {
    
    // actual coords
    
    convergenceLatLon = latLonDecimalFromLatLonDegMinSec(testConvergenceLatLons[i]);
    actualC = testConvergencesFromLatLons[i];
    ASPRINTF_OR_DIE(&actualCStr, "%i\u00b0 %02i\u2032 %.4f\u2033 %c", actualC.deg, actualC.min, actualC.sec, actualC.westOrSouth ? 'W' : 'E');
    if (noisily) printf("Convergence reference from lat/lon  %s\n", actualCStr);
    
    // computed coords
    
    computedC = degMinSecFromDecimal(gridConvergenceDegreesFromLatLon(convergenceLatLon, Airy1830Ellipsoid, NationalGridProj));
    ASPRINTF_OR_DIE(&computedCStr, "%i\u00b0 %02i\u2032 %.4f\u2033 %c", computedC.deg, computedC.min, computedC.sec, computedC.westOrSouth ? 'W' : 'E');
    
    testPassed = strcmp(actualCStr, computedCStr) == 0;
    numTested ++;
    if (testPassed) numPassed ++;
    if (noisily) printf("%sConvergence computed from lat/lon   %s\n\n%s", (testPassed ? "" : BOLD), computedCStr, (testPassed ? "" : UNBOLD));
    
    free(actualCStr);
    free(computedCStr);
  }
  
  bool allPassed = numTested == numPassed;
  if (noisily) printf("%i tests; %i passed; %s%i failed%s\n\n", numTested, numPassed, (allPassed ? "" : BOLD), numTested - numPassed, (allPassed ? "" : UNBOLD));
  return allPassed;
}

#ifdef USE_EMBIND
#include <emscripten/bind.h>

OSMap OSExplorerMapDataForIndex(const int i) {
  return OSExplorerMaps[i];
}

std::string OSExplorerMapNameUTF8ForIndex(const int i) {
  std::string name(OSExplorerMaps[i].nameUTF8);
  return name;
}

std::string OSExplorerMapSheetUTF8ForIndex(const int i) {
  std::string sheet(OSExplorerMaps[i].sheetUTF8);
  return sheet;
}

std::string stringWrapped_tetradFromOSGB36EastingNorthing(const EastingNorthing en) {
  char* tetradChar = tetradFromOSGB36EastingNorthing(en);
  std::string tetrad(tetradChar);
  free(tetradChar);
  return tetrad;
}

std::string stringWrapped_gridRefFromOSGB36EastingNorthing(const EastingNorthing en, const int res, const bool spaces) {
  char* gridRefChar = gridRefFromOSGB36EastingNorthing(en, res, spaces);
  std::string gridRef(gridRefChar);
  free(gridRefChar);
  return gridRef;
}

std::string geoidNameForIndex(const int i) {
  std::string name(OSGB36GeoidNames[i]);
  return name;
}

std::string geoidRegionForIndex(const int i) {
  std::string region(OSGB36GeoidRegions[i]);
  return region;
}

EMSCRIPTEN_BINDINGS(ostn02c) {
  emscripten::value_object<EastingNorthing>("EastingNorthing")
    .field("e", &EastingNorthing::e)
    .field("n", &EastingNorthing::n)
    .field("elevation", &EastingNorthing::elevation)
    .field("geoid", &EastingNorthing::geoid)
  ;
  emscripten::value_object<LatLonDecimal>("LatLonDecimal")
    .field("lat", &LatLonDecimal::lat)
    .field("lon", &LatLonDecimal::lon)
    .field("elevation", &LatLonDecimal::elevation)
  ;
  emscripten::value_object<OSMap>("OSMap")
    .field("num", &OSMap::num)
    .field("emin", &OSMap::emin)
    .field("nmin", &OSMap::nmin)
    .field("emax", &OSMap::emax)
    .field("nmax", &OSMap::nmax)
  ;
  emscripten::value_object<LatLonDegMinSec>("LatLonDegMinSec")
    .field("lat", &LatLonDegMinSec::lat)
    .field("lon", &LatLonDegMinSec::lon)
    .field("elevation", &LatLonDegMinSec::elevation)
  ;
  emscripten::value_object<DegMinSec>("DegMinSec")
    .field("deg", &DegMinSec::deg)
    .field("min", &DegMinSec::min)
    .field("sec", &DegMinSec::sec)
    .field("westOrSouth", &DegMinSec::westOrSouth)
  ;

  emscripten::function("ETRS89EastingNorthingFromETRS89LatLon",           &ETRS89EastingNorthingFromETRS89LatLon);
  emscripten::function("OSGB36EastingNorthingFromETRS89EastingNorthing",  &OSGB36EastingNorthingFromETRS89EastingNorthing);
  emscripten::function("ETRS89EastingNorthingFromOSGB36EastingNorthing",  &ETRS89EastingNorthingFromOSGB36EastingNorthing);
  emscripten::function("ETRS89LatLonFromETRS89EastingNorthing",           &ETRS89LatLonFromETRS89EastingNorthing);
  
  emscripten::function("tetradFromOSGB36EastingNorthing",                 &stringWrapped_tetradFromOSGB36EastingNorthing);
  emscripten::function("gridRefFromOSGB36EastingNorthing",                &stringWrapped_gridRefFromOSGB36EastingNorthing);
  
  emscripten::function("degMinSecFromDecimal",                            &degMinSecFromDecimal);
  emscripten::function("decimalFromDegMinSec",                            &decimalFromDegMinSec);
  emscripten::function("latLonDecimalFromLatLonDegMinSec",                &latLonDecimalFromLatLonDegMinSec);
  emscripten::function("latLonDegMinSecFromLatLonDecimal",                &latLonDecimalFromLatLonDegMinSec);
  
  emscripten::function("OSTN02Test",                                      &test);
  
  emscripten::function("OSExplorerMapNextIndex",                          &nextOSExplorerMap);
  emscripten::function("OSExplorerMapDataForIndex",                       &OSExplorerMapDataForIndex);
  emscripten::function("OSExplorerMapNameUTF8ForIndex",                   &OSExplorerMapNameUTF8ForIndex);
  emscripten::function("OSExplorerMapSheetUTF8ForIndex",                  &OSExplorerMapSheetUTF8ForIndex);
  
  emscripten::function("gridConvergenceDegreesFromOSGB36EastingNorthing", &gridConvergenceDegreesFromOSGB36EastingNorthing);
  emscripten::function("gridConvergenceDegreesFromETRS89LatLon",          &gridConvergenceDegreesFromETRS89LatLon);

  emscripten::function("geoidNameForIndex",                               &geoidNameForIndex);
  emscripten::function("geoidRegionForIndex",                             &geoidRegionForIndex);
}

#endif

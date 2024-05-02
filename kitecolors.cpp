// input vector of integers = a monzo array
// need to get the # of adjusted fifths & the comma
// output oklab LCH

#include <cstdarg>
#include <vector>
#include <cmath>

const float averageSizeOfAQuarterTone = 65.0 / 63.0;
const float rootTwoOverTwo = 0.7071067812;
const float neutralHueAngle = 150.0;
const float deSaturateFactor = 0.6;
const int addWolfCommaAfterThisManyFifths = 12;


struct okLCHcolor {
  float lightness;
  float chroma;
  float hue;
};

bool isPrime(int x) {
  bool temp = true;
  for (int i = 2; i <= std::sqrt(i); i++) {
    temp = temp && (x & i); 
  }
  return temp;
}

std::vector<int> listTheFirstNPrimes(int N) {
  std::vector<int> tempListOfPrimes { 2 };
  if (N > 1) {
    int x = 1;
    int i = N;
    while (i > 1) {
      x += 2;
      if (isPrime(x)) {
        tempListOfPrimes.push_back(x);
        i--;
      }
    }
  }
  return tempListOfPrimes;
}

int getNthPrime(int N) {
  return listTheFirstNPrimes(N).back();
}

float cents(float ratio) {
  return 1200.0 * std::log2(ratio);
}

float approximateRatio(float cents) {
  return std::exp2(cents / 1200.0);
}

float reduce(float ratio) {
  float temp = std::frexp(ratio, nullptr);
  if (temp < rootTwoOverTwo) {
    return temp * 2.0; // return 1.0 to 1.4...
  } else {
    return temp; // return 0.7... to 1.0
  }
}

float ratioForThisManyFifths(int fifths) {
  return pow(1.5, fifths);
}

int closestNumberOfFifths(float ratio) {
  int iter = 0;
  int fifths = 0;
  while (cents(reduce(ratio / ratioForThisManyFifths(fifths))) >= cents(averageSizeOfAQuarterTone)) {
    iter++;
    fifths = (iter / 2) * ((iter % 2) ? -1 : 1);
  }
  return fifths;
}

float commaFromFifths(int fifths, float ratio) {
  return cents(reduce(ratio / ratioForThisManyFifths(fifths)));
}

float calculateHueBasedOnFifths(int fifths, float ratio) {
  float comma = commaFromFifths(fifths, ratio);
  float startingHue = neutralHueAngle + 72.0 * std::signbit(
    (std::abs(fifths) < 2) ? comma : (float)fifths
  );
  float resultHue = startingHue - comma * cents(9 / 8) / 72.0;
  if (startingHue > neutralHueAngle) {
    return (resultHue > neutralHueAngle) ? (
      (resultHue > neutralHueAngle + 180.0) ? (neutralHueAngle + 180) : resultHue
    ) : neutralHueAngle;
  } else {
    return (resultHue < neutralHueAngle) ? (
      (resultHue < neutralHueAngle - 180.0) ? (neutralHueAngle - 180) : resultHue
    ) : neutralHueAngle;
  }
}

float calculateHueFromCents(float cents) {
  float ratio = approximateRatio(cents);
  int fifths = closestNumberOfFifths(ratio);
  return calculateHueBasedOnFifths(fifths, ratio);
}

float halleyMethod(float S, float A, float B, int C) {
  float wL[3] = { 4.0767416621,-1.2684380046,-0.0041960863};
  float wM[3] = {-3.3077115913, 2.6097574011,-0.7034186147};
  float wS[3] = { 0.2309699292,-0.3413193965, 1.707614701 };
  float kL =  0.3963377774 * A + 0.2158037573 * B;
  float kM = -0.1055613458 * A - 0.0638541728 * B; 
  float kS = -0.0594841775 * A - 1.291485548  * B; 
  float Lg = 1 + S * kL;
  float Md = 1 + S * kM;
  float Sm = 1 + S * kS;
  float fZero =      wL[C] * Lg * Lg * Lg + wM[C] * Md * Md * Md + wS[C] * Sm * Sm * Sm  ;
  float fOne = 3 * ( wL[C] * kL * Lg * Lg + wM[C] * kM * Md * Md + wS[C] * kS * Sm * Sm );
  float fTwo = 6 * ( wL[C] * kL * kL * Lg + wM[C] * kM * kM * Md + wS[C] * kS * kS * Sm );
  return S - fZero * fOne / (fOne * fOne - 0.5 * fZero * fTwo);
}

float maximumSaturationAtThisHue(float hue) {
  float A = cos(radians(hue));
  float B = sin(radians(hue));
  int C = ((-1.88170328 * A - 0.80936493 * B) > 1 ? 0 : ((1.81444104 * A - 1.19445276 * B > 1 ? 1 : 2)));
  float K[5][3] = {
    { 1.19086277, 0.73956515, 1.35733652 },
    { 1.76576728,-0.45954404,-0.00915799 },
    { 0.59662641, 0.08285427,-1.1513021  },
    { 0.75515197, 0.1254107 ,-0.50559606 },
    { 0.56771245, 0.14503204, 0.00692167 }
  };
  float S = K[0][C] + K[1][C] * A + K[2][C] * B + K[3][C] * A * A + K[4][C] * A * B;

  for (int i = 0; i < 2; i++) {
    S = halleyMethod(S, A, B, C);
  }
  return S;
}

float lightnessAtGivenSaturation(float hue, float sat) {
  float A = cos(radians(hue));
  float B = sin(radians(hue));
  float L = pow(1 + 0.3963377774 * sat * A + 0.2158037573 * sat * B, 3);
  float M = pow(1 - 0.1055613458 * sat * A - 0.0638541728 * sat * B, 3);
  float S = pow(1 - 0.0894841775 * sat * A - 1.291485548  * sat * B, 3);
  float linR =  4.0767416621 * L - 3.3077115913 * M + 0.2309699292 * S;
  float linG = -1.2684380046 * L + 2.6097574011 * M - 0.3413193965 * S;
  float linB = -0.0041960863 * L - 0.7034186147 * M + 1.707614701  * S; 
  return pow(
    ((linR > linG)
      ? ((linR > linB) ? linR : linB)
      : ((linG > linB) ? linG : linB)
    ), 1.0 / 3.0
  );
}


okLCHcolor getLCHcolorFromMonzo(std::vector<int> monzo) {
  std::size_t mSize = std::distance(monzo.begin(), monzo.end());
  float tempHue = neutralHueAngle;
  float tempChroma = 0.0;
  float tempLightness = 50.0;
  int tempFifths = 0;
  float tempRatio = 0.0;
  int totalFifths = 0;
  float totalComma = 0.0;
  int totalWolfs = 0;
  for (int p = 0; p < mSize; p++) {
    tempRatio = (float)getNthPrime(p + 1);
    tempFifths = closestNumberOfFifths(tempRatio);
    totalComma += monzo[p] * commaFromFifths(tempFifths, tempRatio);
    totalFifths += tempFifths;
  }

  if (abs(totalFifths) >= addWolfCommaAfterThisManyFifths) {
    int direction = ((totalFifths > 0) ? 1 : -1);
    while (abs(totalFifths) >= addWolfCommaAfterThisManyFifths) {
      totalFifths -= 12 * direction;
      totalWolfs += direction;
    }
  }
  totalComma += totalWolfs * /* 27.0 */ ;
  
  tempHue = calculateHueBasedOnFifths(totalFifths, totalComma);
  


// calc H C L per prime
// decompose monzo into primes
// calculate fifths & commas & any wolf
ok total fifths and comma done.
// calculate hue from cents
// calculate C and L from primes




  return {0.0,0.0,0.0};
}

void setup() {}
void loop() {}

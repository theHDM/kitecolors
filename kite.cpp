// input vector of integers = a monzo array
// need to get the # of adjusted fifths & the comma
// output oklab LCH

#include <cstdarg>
#include <vector>
#include <cmath>
#include <iostream>

const float averageSizeOfAQuarterTone = 65.0 / 63.0;
const float rootTwoOverTwo = 0.7071067812;
const float neutralHueAngle = 150.0;
const float deSaturateFactor = 0.4;
const int addWolfCommaAfterThisManyFifths = 12;

struct okLCHcolor {
  float lightness;
  float chroma;
  float hue;
};

struct sRGBcolor {
  int R;
  int G;
  int B;
};

float clamp(float value, float bottom, float ceiling) {
  return ((value > bottom) ? ((value > ceiling) ? ceiling : value) : bottom);
}

float maxOfThree(float a, float b, float c) {
  return ((a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c));
}

void degreesToXY(float angle, float* x, float* y) {
  float rad = angle * 3.14159265 / 180.0;
  *x = cos(rad);
  *y = sin(rad);
}

int gamma(float linear) {
  float f = (linear >= 0.0031308) ? (1.055 * pow(linear, 1.0 / 2.4) - 0.055) : (12.92 * linear);
  return round(255.0 * clamp(f, 0.0, 1.0));
}

sRGBcolor toRGB(okLCHcolor givenOkLCH) {
  float A;
  float B;
  degreesToXY(givenOkLCH.hue, &A, &B);
  A *= givenOkLCH.chroma;
  B *= givenOkLCH.chroma;
  float L = pow(givenOkLCH.lightness + 0.3963377774 * A + 0.2158037573 * B, 3);
  float M = pow(givenOkLCH.lightness - 0.1055613458 * A - 0.0638541728 * B, 3);
  float S = pow(givenOkLCH.lightness - 0.0894841775 * A - 1.291485548  * B, 3);
  float linR =  4.0767416621 * L - 3.3077115913 * M + 0.2309699292 * S;
  float linG = -1.2684380046 * L + 2.6097574011 * M - 0.3413193965 * S;
  float linB = -0.0041960863 * L - 0.7034186147 * M + 1.707614701  * S;
  return {gamma(linR), gamma(linG), gamma(linB)};
}

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
  int discard;
  float temp = std::frexp(ratio, &discard);
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
  while (abs(cents(reduce(ratio / ratioForThisManyFifths(fifths)))) >= cents(averageSizeOfAQuarterTone)) {
    iter++;
    fifths = (iter / 2) * ((iter % 2) ? -1 : 1);
  }
  return fifths;
}

float commaFromFifths(int fifths, float ratio) {
  return cents(reduce(ratio / ratioForThisManyFifths(fifths)));
}

float calculateHueBasedOnComma(int fifths, float comma) {
  float startingHue = neutralHueAngle + 72.0 * (
    std::signbit((std::abs(fifths) < 2) ? comma : (float)fifths)
    ? 1 : -1
  );
  std::cout << "calculating hue...\nstarting hue: " << startingHue << "\n";
  float resultHue = startingHue - comma * cents(1.125) / 144.0;
  std::cout << "result hue: " << resultHue << "\n";
  if (startingHue > neutralHueAngle) {
    return clamp(resultHue, neutralHueAngle, neutralHueAngle + 180.0);
  } else {
    return clamp(resultHue, neutralHueAngle - 180.0, neutralHueAngle);
  }
}

float calculateHueBasedOnFifths(int fifths, float ratio) {
  return calculateHueBasedOnComma(fifths, commaFromFifths(fifths, ratio));
}

float calculateHueFromPrime(int prime, bool underTone) {
  float ratio;
  if (underTone) {
    ratio = 1.0 / prime;
  } else {
    ratio = (float)prime;
  }
  return calculateHueBasedOnFifths(closestNumberOfFifths(ratio), ratio);
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
  float A;
  float B;
  degreesToXY(hue, &A, &B);
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
  float A;
  float B;
  degreesToXY(hue, &A, &B);
  float L = pow(1 + 0.3963377774 * sat * A + 0.2158037573 * sat * B, 3);
  float M = pow(1 - 0.1055613458 * sat * A - 0.0638541728 * sat * B, 3);
  float S = pow(1 - 0.0894841775 * sat * A - 1.291485548  * sat * B, 3);
  float linR =  4.0767416621 * L - 3.3077115913 * M + 0.2309699292 * S;
  float linG = -1.2684380046 * L + 2.6097574011 * M - 0.3413193965 * S;
  float linB = -0.0041960863 * L - 0.7034186147 * M + 1.707614701  * S; 
  return pow( maxOfThree(linR, linG, linB), - 1.0 / 3.0 );
}

okLCHcolor getLCHcolorFromMonzo(std::vector<int> monzo) {
  std::size_t mSize = std::distance(monzo.begin(), monzo.end());
  std::cout << "monzo dimension: " << mSize << "\n";
  float tempHue = neutralHueAngle;
  float tempChroma = 0.0;
  float tempLightness = 0.0;
  
  float tempRatio = 0.0;
  int tempFifths = 0;
  float tempComma = 0.0;
  int totalFifths = 0;
  float totalComma = 0.0;
  int totalWolfs = 0;
  
  for (int p = 0; p < mSize; p++) {
    tempRatio = (float)getNthPrime(p + 1);
    tempFifths = closestNumberOfFifths(tempRatio);
    tempComma = commaFromFifths(tempFifths, tempRatio);
    std::cout << "prime " << tempRatio << " fifths: " << tempFifths<< " comma: " << tempComma << "\n";
    totalComma += monzo[p] * tempComma;
    totalFifths += monzo[p] * tempFifths;
  }

  if (abs(totalFifths) >= addWolfCommaAfterThisManyFifths) {
    int direction = ((totalFifths > 0) ? 1 : -1);
    while (abs(totalFifths) >= addWolfCommaAfterThisManyFifths) {
      totalFifths -= 12 * direction;
      totalWolfs += direction;
    }
  }
  totalComma += totalWolfs * cents( pow(3,12) / pow(2,19) );
  std::cout << "wolfs: " << totalWolfs << "\n" << "total fifths: " << totalFifths << " totalComma: " << totalComma << "\n";
  tempHue = calculateHueBasedOnComma(totalFifths, totalComma);
  if (mSize < 3) {
    tempChroma = 0.0;
    tempLightness = 1.0;
  } else {
    int iterator = 0;
    for (int p = 2; p < mSize; p++) {
      if (monzo[p] != 0) {
        float tempHueThisPrime = calculateHueFromPrime(getNthPrime(p + 1), (monzo[p] < 0));
        float tempSaturationMax = maximumSaturationAtThisHue(tempHueThisPrime);
        float tempLightnessCusp = lightnessAtGivenSaturation(tempHueThisPrime, tempSaturationMax);
        float tempLightnessClamped = clamp(tempLightnessCusp,
          pow(2.0 / getNthPrime(p + 1), 1.0 / 3.0),
          (p == 2) ? 1.0 : pow(2.0 / getNthPrime(p), 1.0 / 3.0)
        );
        float tempChromaThisPrime = tempSaturationMax * (
          ( tempLightnessClamped <= tempLightnessCusp ) ? tempLightnessClamped :
          ( tempLightnessCusp * ( 1 - tempLightnessClamped ) / ( 1 - tempLightnessCusp ) )
        );
        std::cout << "sat max " << tempSaturationMax << " light " << tempLightnessCusp << "\n";
        std::cout << "final light " << tempLightnessClamped << " chroma " << tempChromaThisPrime << "\n";
        tempLightness += tempLightnessClamped;
        tempChroma += tempChromaThisPrime;
        iterator++;
      }    
    }
    tempLightness /= iterator;
    tempChroma /= iterator;
    std::cout << "chroma before desaturate: " << tempChroma << "\n";
    tempChroma *= pow(deSaturateFactor, iterator - 1);
  }
  return {tempLightness, tempChroma, tempHue};
}

sRGBcolor getRGBcolorFromMonzo(std::vector<int> monzo) {
  return toRGB(getLCHcolorFromMonzo(monzo));
}

int main() {
    sRGBcolor result = getRGBcolorFromMonzo({1,0,1,-1});
    std::cout << "R: " << result.R << " G: " << result.G << " B: " << result.B << "\n";
    return 0;
}

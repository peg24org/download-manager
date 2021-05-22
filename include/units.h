#ifndef _UNITS_H
#define _UNITS_H

#include <cmath>

constexpr size_t operator""_KB(unsigned long long input)
{
  return pow(2, 10) * input;
}

constexpr size_t operator""_MB(unsigned long long input)
{
  return pow(2, 20) * input;
}

constexpr size_t operator""_GB(unsigned long long input)
{
  return pow(2, 30) * input;
}

#endif

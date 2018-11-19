#pragma once

#include <cstdint>

class SecondsCounter
{
public:
  SecondsCounter(double runningFrequency);
  ~SecondsCounter() = default;

  double Value() { return _counterValue; }

  double operator++();    // prefix ++
  double operator++(int); // postfix ++

  void operator=(const double &value);
  bool operator<(const double &value);
  bool operator>(const double &value);
  bool operator<=(const double &value);
  bool operator>=(const double &value);
  bool operator==(const double &value);

private:
  double _counterValue = 0;
  double _runningFrequency = 1; // default frequency
  double _increment = 0;
};

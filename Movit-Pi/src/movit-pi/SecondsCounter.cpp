#include "SecondsCounter.h"

SecondsCounter::SecondsCounter(double runningFrequency) : _runningFrequency(runningFrequency)
{
    if (_runningFrequency == 0)
    {
        _increment = 0;
    }
    else
    {
        _increment = 1 / _runningFrequency;
    }
}

double SecondsCounter::operator++() // prefix ++
{
    _counterValue = _counterValue + _increment;
    return _counterValue;
}

double SecondsCounter::operator++(int) // postfix ++
{
    double value = _counterValue;               // make a copy for result
    _counterValue = _counterValue + _increment; // Now use the prefix version to do the work
    return value;                               // return the copy (the old) value.
}

void SecondsCounter::operator=(const double &value)
{
    _counterValue = value;
}

bool SecondsCounter::operator<(const double &value)
{
    return _counterValue < value;
}

bool SecondsCounter::operator>(const double &value)
{
    return _counterValue > value;
}

bool SecondsCounter::operator<=(const double &value)
{
    return _counterValue <= value;
}

bool SecondsCounter::operator>=(const double &value)
{
    return _counterValue >= value;
}

bool SecondsCounter::operator==(const double &value)
{
    return _counterValue == value;
}

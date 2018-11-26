#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

#define WINDOW_SIZE_BUFFER 256

#include <numeric>

#include <iostream>

template <class T>
class MovingAverage
{
  public:
    MovingAverage(uint8_t windowSize) : _windowSize(windowSize)
    {
    }

    void AddSample(T sample)
    {
        _values[_pos] = sample;
        _pos++;

        if (_pos >= _windowSize)
        {
            _pos = 0;
            _windowFilled = true;
        }
    }

    double GetAverage()
    {
        if (_windowFilled)
        {
            return static_cast<double>(std::accumulate(_values, _values + _windowSize, 0) / _windowSize);
        }
        else
        {
            if (_pos == 0)
            {
                return 0.0f;
            }
            return static_cast<double>(std::accumulate(_values, _values + _pos, 0) / _pos);
        }
    }

    void Reset()
    {
        _pos = 0;
        _windowFilled = false;
        for (int i = 0; i < WINDOW_SIZE_BUFFER; i++)
        {
            _values[i] = static_cast<T>(0);
        }
    }

  private:
    uint8_t _windowSize = 0;
    uint8_t _pos = 0;
    T _values[WINDOW_SIZE_BUFFER];
    bool _windowFilled = false;
};

#endif //MOVING_AVERAGE_H
#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer
{
  public:
    Timer() : m_beg(clock_::now())
    {
    }
    
    void Reset()
    {
      m_beg = clock_::now();
    }

    // Retourne le nombre de miliseconde passé depuis le dernier reset
    double Elapsed() const
    {
      return std::chrono::duration_cast<std::chrono::milliseconds>(clock_::now() - m_beg).count();
    }

  private:
    typedef std::chrono::high_resolution_clock clock_;
    typedef std::chrono::duration<double, std::ratio<1>> second_;
    std::chrono::time_point<clock_> m_beg;
};

#endif // TIMER_H

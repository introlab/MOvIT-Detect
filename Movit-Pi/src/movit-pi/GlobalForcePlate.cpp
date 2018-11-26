#include "GlobalForcePlate.h"

GlobalForcePlate::GlobalForcePlate()
{
    _fx12 = 0.0f;
    _fx34 = 0.0f;
    _fy14 = 0.0f;
    _fy23 = 0.0f;
    _fz1 = 0.0f;
    _fz2 = 0.0f;
    _fz3 = 0.0f;
    _fz4 = 0.0f;
    _fx = 0.0f;
    _fy = 0.0f;
    _fz = 0.0f;
    _mx = 0.0f;
    _my = 0.0f;
    _mz = 0.0f;
    _mx1 = 0.0f;
    _my1 = 0.0f;

    _centerOfPressure = {0.0f, 0.0f};
}

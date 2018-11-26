#include "Sensor.h"

bool Sensor::IsStateChanged()
{
    if (_isConnected != _prevIsConnected)
    {
        _prevIsConnected = _isConnected;
        return true;
    }

    _prevIsConnected = _isConnected;
    return false;
}


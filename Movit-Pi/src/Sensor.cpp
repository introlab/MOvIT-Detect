#include "Sensor.h"

bool Sensor::IsStateChanged()
{
    const bool isConnected = IsConnected();

    if (_isConnected != isConnected)
    {
        _isConnected = isConnected;
        return true;
    }

    return false;
}


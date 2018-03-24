#include "ChairManager.h"

ChairManager::ChairManager(MosquittoBroker *mosquittoBroker, DeviceManager *devicemgr)
    : _mosquittoBroker(mosquittoBroker), _devicemgr(devicemgr)
{
    _copCoord.x = 0;
    _copCoord.y = 0;
}

void ChairManager::UpdateDevices()
{
    _devicemgr->update();
    _currentDatetime = _devicemgr->getDateTime();
    _isSomeoneThere = _devicemgr->isSomeoneThere();
    _copCoord = _devicemgr->getCenterOfPressure();
    _prevChairAngle = _currentChairAngle;
    _currentChairAngle = _devicemgr->GetBackSeatAngle();

    printf("getDateTime = %s\n", _currentDatetime.c_str());
    printf("isSomeoneThere = %i\n", _devicemgr->isSomeoneThere());
    printf("getCenterOfPressure x = %i, y = %i\n", _copCoord.x, _copCoord.y);
    printf("GetBackSeatAngle = %i\n\n", _currentChairAngle);
}

void ChairManager::ReadFromServer()
{
    if (_mosquittoBroker->isSetAlarmOnNew())
    {
        setAlarmOn = _mosquittoBroker->getSetAlarmOn();
        printf("Something new for setAlarmOn = %i\n", setAlarmOn);
    }
    if (_mosquittoBroker->isRequiredBackRestAngleNew())
    {
        requiredBackRestAngle = _mosquittoBroker->getRequiredBackRestAngle();
        printf("Something new for requiredBackRestAngle = %i\n", requiredBackRestAngle);
    }
    if (_mosquittoBroker->isRequiredPeriodNew())
    {
        requiredPeriod = _mosquittoBroker->getRequiredPeriod();
        printf("Something new for requiredPeriod = %i\n", requiredPeriod);
    }
    if (_mosquittoBroker->isRequiredDurationNew())
    {
        requiredDuration = _mosquittoBroker->getRequiredDuration();
        printf("Something new for requiredDuration = %i\n", requiredDuration);
    }
}

void ChairManager::CheckNotification(uint8_t &state)
{
    if (_isSomeoneThere && requiredDuration != 0 && requiredPeriod != 0)
    {
        switch (state)
        {
        // Vérifie que le patient est assis depuis 5 secondes
        case 1:
            _secondsCounter++;
            if (_secondsCounter >= 5)
            {
                state = 2;
                _secondsCounter = 0;
            }
            break;
        // Une bascule est requise
        case 2:
            _secondsCounter++;
            if (_secondsCounter >= requiredPeriod)
            {
                state = 3;
                _secondsCounter = 0;
            }
            break;
        // Genere un alarme pour initier la bascule
        case 3:
            // L'ancienne équipe attendait un autre 60 secondes sans bougé soit passé
            // Faudrais considerer un autre solution car ca me semble wack
            // if (notmoving)
            // {
            //     //genere un alarm
            // }
            // else
            // {
            //     //faire un ptit bzz bzz pour dire d'arreter de bouger ?
            // }

            //TODO: Valider que c'est le bon genre d'alarme
            _devicemgr->getAlarm()->TurnOnDCMotor();
            _devicemgr->getAlarm()->TurnOnRedLed();
            state = 4;
            break;
        // Gestion de la monté de la bascule
        case 4:
            //Quand une positive rate est détecté sur l'angle, il faut l'envoyer
            if (_currentChairAngle - _prevChairAngle >= 1)
            {
                //TODO: Peut-être implémenté quelque chose qui valide qu'on a deux
                // échantillion de suite qui ont un positive rate ?
                _mosquittoBroker->sendBackRestAngle(_currentChairAngle);
            }

            // Si le patient est rendu a l'angle +- 10 deg et il s'arrête
            // TODO reconsiderer le threshold de 10 deg et handle si le patient dépasse l'angle
            if (abs(requiredBackRestAngle - _currentChairAngle) < 10 && _prevChairAngle == _currentChairAngle)
            {
                // Quand on s'arrête, on envoi l'angle au back end
                _mosquittoBroker->sendBackRestAngle(_currentChairAngle);

                _devicemgr->getAlarm()->TurnOffDCMotor();
                _devicemgr->getAlarm()->TurnOffRedLed();
                _devicemgr->getAlarm()->TurnOnGreenLed();
                state = 5;
            }
            break;
        // Maintient de la bascule
        case 5:
            // Si l'angle est maintenu
            if (abs(requiredBackRestAngle - _currentChairAngle) < 10)
            {
                // On utilise l'alarme pour dire au patient de s'arrêter.
                // Temporairement, on alume les deux LED.
                // TODO: Il faudrait que les deux LEDs clignottent
                _devicemgr->getAlarm()->TurnOffDCMotor();
                _devicemgr->getAlarm()->TurnOnGreenLed();
                _devicemgr->getAlarm()->TurnOnRedLed();
                _secondsCounter++;
                if (_secondsCounter >= requiredDuration)
                {
                    state = 6;
                }
            }
            // Le patient ne maintient plus l'angle de bascule
            else
            {
                _devicemgr->getAlarm()->TurnOnDCMotor();
                _devicemgr->getAlarm()->TurnOnRedLed();
                _devicemgr->getAlarm()->TurnOffGreenLed();
                state = 4;
            }
            break;
        // Descente de la bascule
        case 6:
            _secondsCounter++;
            // On quitte quand l'angle requis - 10 n'est plus maintenue. Par contre, on laisse la possibilité de continuer
            if ((requiredBackRestAngle - _currentChairAngle) > 10)
            {
                state = 2;
                _secondsCounter = 0;
                _mosquittoBroker->sendBackRestAngle(_currentChairAngle);
                //TODO: Considerer envoyer le temps passé à l'angle aussi
            }
            break;
        }
    }
    else
    {
        state = 1;
        _secondsCounter = 0;
    }
}
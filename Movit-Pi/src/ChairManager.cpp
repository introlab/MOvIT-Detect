#include "ChairManager.h"

#define INITIAL_VALIDATING_TIME 5 // Temps initial d'attente avant de commencer la séquence de bascule
#define POSITIVE_ANGLE_RATE_THRESHOLD 3
#define ACCEPTABLE_ANGLE_RANGE 10

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

    // printf("getDateTime = %s\n", _currentDatetime.c_str());
    // printf("isSomeoneThere = %i\n", _devicemgr->isSomeoneThere());
    // printf("getCenterOfPressure x = %f, y = %f\n", _copCoord.x, _copCoord.y);
    printf("\n _currentChairAngle = %i\n", _currentChairAngle);
    printf("_prevChairAngle = %i\n\n", _prevChairAngle);
}

void ChairManager::ReadFromServer()
{
    if (_mosquittoBroker->isSetAlarmOnNew())
    {
        _overrideNotificationPattern = true;
        _setAlarmOn = _mosquittoBroker->getSetAlarmOn();
        printf("Something new for setAlarmOn = %i\n", _setAlarmOn);
    }
    if (_mosquittoBroker->isRequiredBackRestAngleNew())
    {
        _requiredBackRestAngle = _mosquittoBroker->getRequiredBackRestAngle();
        _secondsCounter = 0;
        // TODO valider que c'est le bon _state
        _state = 1;
        printf("Something new for _requiredBackRestAngle = %i\n", _requiredBackRestAngle);
    }
    if (_mosquittoBroker->isRequiredPeriodNew())
    {
        _requiredPeriod = _mosquittoBroker->getRequiredPeriod();
        _secondsCounter = 0;
        // TODO valider que c'est le bon _state
        _state = 1;
        printf("Something new for _requiredPeriod = %i\n", _requiredPeriod);
    }
    if (_mosquittoBroker->isRequiredDurationNew())
    {
        _requiredDuration = _mosquittoBroker->getRequiredDuration();
       _secondsCounter = 0;
        // TODO valider que c'est le bon _state
        _state = 1;
        printf("Something new for _requiredDuration = %i\n", _requiredDuration);
    }

    _mosquittoBroker->sendBackRestAngle(_currentChairAngle, _currentDatetime);
    _mosquittoBroker->sendCenterOfPressure(_copCoord.x, _copCoord.y, _currentDatetime);
    _mosquittoBroker->sendIsSomeoneThere(_isSomeoneThere, _currentDatetime);
    _mosquittoBroker->sendSpeed(0, _currentDatetime);
}

void ChairManager::CheckNotification()
{
    if (!_overrideNotificationPattern)
    {
        if (_isSomeoneThere && _requiredDuration != 0 && _requiredPeriod != 0 && _requiredBackRestAngle != 0)
        {
            switch (_state)
            {
            // Vérifie que le patient est assis depuis 5 secondes
            case 1:
                _secondsCounter++;
                printf("_state 1\t_secondsCounter: %i\n", _secondsCounter);
                if (_secondsCounter >= INITIAL_VALIDATING_TIME)
                {
                    _state = 2;
                    _secondsCounter = 0;
                }
                break;
            // Une bascule est requise
            case 2:
                _secondsCounter++;
                printf("_state 2\t_secondsCounter: %i\n", _secondsCounter);
                if (_secondsCounter >= _requiredPeriod)
                {
                    _state = 3;
                    _secondsCounter = 0;
                }
                break;
            // Genere un alarme pour initier la bascule
            case 3:
                // TODO
                // L'ancienne équipe attendait un autre 60 secondes sans bougé soit passé
                // Faudrais considerer un autre solution car ca me semble wack


                //TODO: Valider que c'est le bon genre d'alarme
                printf("_state 3\n");
                _devicemgr->getAlarm()->TurnOnDCMotor();
                _devicemgr->getAlarm()->TurnOnRedLed();
                _state = 4;
                break;
            // Gestion de la monté de la bascule
            case 4:
                printf("_state 4\t _currentChairAngle - _prevChairAngle: %i\n", _currentChairAngle - _prevChairAngle);
                //Quand une positive rate est détecté sur l'angle, il faut l'envoyer
                //TODO: ajouter une variable de threshol et la tuner
                if (_currentChairAngle - _prevChairAngle >= POSITIVE_ANGLE_RATE_THRESHOLD)
                {
                    //TODO: Peut-être implémenté quelque chose qui valide qu'on a deux
                    // échantillion de suite qui ont un positive rate ?
                    printf("_state 4 SEND ANGLE\t _currentChairAngle: %i\n", _currentChairAngle);
                    _mosquittoBroker->sendBackRestAngle(_currentChairAngle, _currentDatetime);
                }

                printf("_state 4\t abs(requiredBackRestAngle - _currentChairAngle): %i\n", abs(int(_requiredBackRestAngle) - int(_currentChairAngle)));
                // Si le patient est rendu a l'angle +- ACCEPTABLE_ANGLE_RANGE deg et il s'arrête
                // TODO reconsiderer si le patient dépasse l'angle
                if (abs(int(_requiredBackRestAngle) - int(_currentChairAngle)) < ACCEPTABLE_ANGLE_RANGE)
                {
                    // Quand on s'arrête, on envoi l'angle au back end
                    _mosquittoBroker->sendBackRestAngle(_currentChairAngle, _currentDatetime);

                    printf("_state 4 SEND ANGLE\t _currentChairAngle: %i\n", _currentChairAngle);

                    _devicemgr->getAlarm()->TurnOffDCMotor();
                    _devicemgr->getAlarm()->TurnOffRedLed();
                    _devicemgr->getAlarm()->TurnOnGreenLed();
                    _state = 5;
                }
                break;
            // Maintient de la bascule
            case 5:
                printf("_state 5\n");
                // Si l'angle est maintenu
                if (abs(_requiredBackRestAngle - _currentChairAngle) < ACCEPTABLE_ANGLE_RANGE)
                {
                    // On utilise l'alarme pour dire au patient de s'arrêter.
                    // Temporairement, on alume les deux LED.
                    // TODO: Il faudrait que les deux LEDs clignottent
                    _devicemgr->getAlarm()->TurnOffDCMotor();
                    _devicemgr->getAlarm()->TurnOnGreenLed();
                    _devicemgr->getAlarm()->TurnOnRedLed();
                    _secondsCounter++;
                    printf("_state 5\t_secondsCounter: %i\n", _secondsCounter);
                    if (_secondsCounter >= _requiredDuration)
                    {
                        _state = 6;
                    }
                }
                // Le patient ne maintient plus l'angle de bascule
                else
                {
                    printf("_state 5 il faut remonter\n");
                    _devicemgr->getAlarm()->TurnOnDCMotor();
                    _devicemgr->getAlarm()->TurnOnRedLed();
                    _devicemgr->getAlarm()->TurnOffGreenLed();
                    _state = 4;
                    _secondsCounter = 0;
                }
                break;
            // Descente de la bascule
            case 6:
                _secondsCounter++;
                printf("_state 6\t_secondsCounter: %i\n", _secondsCounter);
                _devicemgr->getAlarm()->TurnOffRedLed();
                // On quitte quand l'angle requis - ACCEPTABLE_ANGLE_RANGE n'est plus maintenue. Par contre, on laisse la possibilité de continuer
                if ((_requiredBackRestAngle - _currentChairAngle) > ACCEPTABLE_ANGLE_RANGE)
                {
                    _state = 2;
                    _secondsCounter = 0;
                    _mosquittoBroker->sendBackRestAngle(_currentChairAngle, _currentDatetime);
                    _devicemgr->getAlarm()->TurnOffDCMotor();
                    _devicemgr->getAlarm()->TurnOffRedLed();
                    _devicemgr->getAlarm()->TurnOffGreenLed();
                    //TODO: Considerer envoyer le temps passé à l'angle aussi
                }
                break;
            }
        }
        else
        {
            _state = 1;
            _secondsCounter = 0;
            // _devicemgr->getAlarm()->TurnOffDCMotor();
            // _devicemgr->getAlarm()->TurnOffRedLed();
            // _devicemgr->getAlarm()->TurnOffGreenLed();
        }
    }
    else
    {
        if (_setAlarmOn)
        {
            _devicemgr->getAlarm()->TurnOnDCMotor();
            _devicemgr->getAlarm()->TurnOnRedLed();
            _devicemgr->getAlarm()->TurnOnGreenLed();
        }
        else
        {
            _devicemgr->getAlarm()->TurnOffDCMotor();
            _devicemgr->getAlarm()->TurnOffRedLed();
            _devicemgr->getAlarm()->TurnOffGreenLed();
        }
        _overrideNotificationPattern = false;
    }
}
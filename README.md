# MOvIT-Detect
MOvIT-Detect est la partie capteur du projet MOvIT+ et elle prend en charge différents capteurs I²C. Ces capteurs servent à déterminer l'état du fauteuil et de son passager. Le code d'acquisitions des capteurs est en C++ et il communique via MQTT au backend. Les capteurs supportés actuellement sont les suivants:

| Capteur | Utilité | Communication | Adresse |
| --- | --- | --- | --- |
| Accéléromètre MPU6050 | À l'aide de deux accéléromètres, il est possible de connaitre l'angle de la chaise selon le fauteuil en entier, et ainsi connaitre la position du patient | I²C | 0x68-0x69 |
| I/O Extender PCA9536 | Utilisé pour la télécommande, permet le contrôle de deux LED, une rouge et une verte, d'un moteur pour des vibrations et la lecture d'un bouton poussoir | I²C | 0x41 |
| ADC MAX11611 | ADC de 10-bit a 12 canaux, les 9 premiers sont utilisés pour la lecture de 9 capteurs à pression, sous le siège du patient, permet alors de connaitre le centre de gravité du patient, et la détection du patient sur le fauteuil | I²C | 0x35 |
| RTC MCP79410 | Permet de garder la date et l'heure du raspberry pi en temps réel, et ce même lorsqu'il est déconnecté, le RTC ce met a jour si besoin avec le serveur NTP | I²C | 0x35 & 0x6f |
| ToF Ranging VL53L0X | Permet de calculer la distance entre le sol et le fauteuil, utiliser avec le capteur de Flow PWM3901, cela permet de calculer la vitesse du fauteuil | I²C | 0x29 |
| Optical Flow SensorPMW3901 | Calcul la distance en deltaX et deltaY de son changement de position, permet de savoir si le fauteuil est en mouvement, et de quelle distance celui-ci c'est déplacé | SPI | X |

> Le guide _Configuration du système_ devrait avoir été suivit avant de procéder à cette installation.
___
## Table des matières :
- [MOvIT-Detect](#movit-detect)
  - [Table des matières :](#table-des-mati%c3%a8res)
- [1. Configuration des capteurs](#1-configuration-des-capteurs)
  - [1.1. Activation des capteurs](#11-activation-des-capteurs)
      - [Activation I²C](#activation-i%c2%b2c)
      - [Activation SPI](#activation-spi)
      - [Expend FileSystem (Optionel)](#expend-filesystem-optionel)
  - [1.2. Connection au RTC](#12-connection-au-rtc)
      - [Installation de i2cdetect](#installation-de-i2cdetect)
      - [RTC MCP79410](#rtc-mcp79410)
      - [Vérification de la date et heure](#v%c3%a9rification-de-la-date-et-heure)
- [2. Installation de MOvIt-Detect](#2-installation-de-movit-detect)
  - [2.1. Installation de GitHub](#21-installation-de-github)
  - [2.2. Installation de librairies](#22-installation-de-librairies)
  - [2.3. Installation de MOvIT-Detect](#23-installation-de-movit-detect)
      - [Compilation de la librairie bcm2835](#compilation-de-la-librairie-bcm2835)
      - [Compilation de MOvIT-Detect](#compilation-de-movit-detect)
      - [Exécution de MOvIT-Detect](#ex%c3%a9cution-de-movit-detect)
  - [2.4. Cross-compilation](#24-cross-compilation)
      - [Utiliraire de cross-compilation](#utiliraire-de-cross-compilation)
- [3. Explication du code](#3-explication-du-code)
  - [3.1. Machines à états finis](#31-machines-%c3%a0-%c3%a9tats-finis)
      - [Machine à états des bascules](#machine-%c3%a0-%c3%a9tats-des-bascules)
      - [Machine à états des déplacements](#machine-%c3%a0-%c3%a9tats-des-d%c3%a9placements)
      - [Machine à états de détection de présence](#machine-%c3%a0-%c3%a9tats-de-d%c3%a9tection-de-pr%c3%a9sence)
      - [Machine à états des notifications](#machine-%c3%a0-%c3%a9tats-des-notifications)
___

<br>
<br>

# 1. Configuration des capteurs
## 1.1. Activation des capteurs
Il faut, pour commencer, activer le port I²C et le port SPI du Raspberry Pi. Pour ce faire, il faut utiliser l'utilitaire `raspi-config`
```bash
sudo raspi-config
```
#### Activation I²C
Activation du I²C avec raspa-config
- Option 5
- Option P5
- Choisir Yes

#### Activation SPI
Activation du SPI avec raspa-config
- Option 5
- Option P4
- Choisir Yes

#### Expend FileSystem (Optionel)
Expending FileSystem avec raspa-config
- Option 7
- Option A1

Finalement choisir Finish et accepté de redémarré

## 1.2. Connection au RTC
#### Installation de i2cdetect
_i2cdetect_ permet de savoir quels appareils I²C sont connectés au système, de façon à être sûr que l'appareil est bien connecté et que son adresse est la bonne. Il faut installer cet outil comme suit:
```bash
sudo apt-get install -y i2c-tools
```
Une fois l'outil installé, on peut afficher la carte des appareils connectés avec la commande suivante:
```bash
i2cdetect -y 1
```
Avec le capteur RTC connecté uniquement, on obtient la sortie suivante:
```bash
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- 57 -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 6f 
70: -- -- -- -- -- -- -- --    
```
Il y a donc deux appareils I²C a l'adresse 0x57 un EEPROM et a 0x6F le RTC lui-même
#### RTC MCP79410
Le RTC (_Real-Time clock_) doit être activé dans Linux afin de pouvoir garder l'heure adéquatement. Il faut donc activer le "_device tree overlay_" du système Linux afin qu'il active le RTC au démarrage:
```bash
sudo echo "rtc-mcp7941x" | sudo tee --append /etc/modules
sudo echo "dtoverlay=i2c-rtc,mcp7941x" | sudo tee --append /boot/config.txt 
sudo reboot now
```
On peut confirmer que le RTC est bel et bien connecté a l'aide de `i2cdetect -y 1` si on obtient la sortie suivante:
```bash
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- 57 -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- UU 
70: -- -- -- -- -- -- -- --    
```
Le RTC est utilisé par le système Linux, car 6f a été remplacé par UU, il faut ensuite modifier le fichier `/lib/udev/hwclock-set` et commenter certaines lignes:
```bash
sudo nano /lib/udev/hwclock-set
```
Les lignes à commenter sont les suivantes:
```bash
if [ -e /run/systemd/system ] ; then
    exit 0
fi
```
Il faut donc ajouter un "#" devant chacune des lignes de sorte à obtenir:
```bash
#if [ -e /run/systemd/system ] ; then
#    exit 0
#fi 
```

#### Vérification de la date et heure
On détermine la date et l'heure a l'aide de la commande `date` cette commande retourne la date et l'heure, voici la sortie de ce système:
```bash
Wed 16 Jan 14:43:42 EST 2019
```
Si la date est incorrecte, il faut changer de TimeZone, voici comment procédé, a l'aide de l'utilitaire `raspa-config` il faut:
- Choisir l'option 4
- Puis l'option I2
- Choisir ensuite le bon TimeZone selon votre location
- Choisir finish

Confirmer la date et l'heure avec `date`, si celle-ci est bonne, il faut l'écrire dans le RTC avec la commande suivante:
```bash
sudo hwclock -w
```

# 2. Installation de MOvIt-Detect
## 2.1. Installation de GitHub
Si _git_ n'est pas installé, il faut exécuter cette commande : `sudo apt-get install -y git`

## 2.2. Installation de librairies
Il faut installer les libraires de mosquitto a afin de pouvoir utiliser le MQTT comme moyen de communication:
```bash
wget http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key
sudo apt-key add mosquitto-repo.gpg.key
cd /etc/apt/sources.list.d/
sudo wget http://repo.mosquitto.org/debian/mosquitto-buster.list
sudo apt-get update
sudo apt-get install -y libkrb5-dev libzmq3-dev mosquitto-clients=1.6.4-0mosquitto1~buster1 libmosquitto1=1.6.4-0mosquitto1~buster1 mosquitto=1.6.4-0mosquitto1~buster1 libmosquitto-dev=1.6.4-0mosquitto1~buster1 libmosquittopp-dev=1.6.4-0mosquitto1~buster1 libmosquittopp1=1.6.4-0mosquitto1~buster1 --allow-downgrades
```
> Ancienne dernière commande : `sudo apt-get install -y libmosquitto-dev libmosquittopp-dev libssl-dev automake`
## 2.3. Installation de MOvIT-Detect
Il faut ensuite cloner ce répertoire et y accéder:
```bash
git clone https://github.com/introlab/MOvIT-Detect.git
cd MOvIT-Detect
```
#### Compilation de la librairie bcm2835
Il faut également la librairie _bcm2835_ pour communiquer avec les ports GPIO du processeur. Elle est disponible à la racine du projet :
```bash
cd bcm2835-1.60
./configure
make && sudo make check && sudo make install
```
> Il n'est pas recommendé de compiler cette partie de code directement sur l'appareil à cause du temps que le système prend pour accomplir cette tâche, soit plus de 10 minutes. Voir la section concernant la [cross-compilation](#cross-compilation)

#### Compilation de MOvIT-Detect
Pour compiler le projet, il suffit d'exécuter les lignes suivantes :
```bash
cd MOvIT-Detect/Movit-Pi
make -f MakefilePI all # Utilisation du Makefile pour compilation directment sur le PI
```
> La compilation de ce code peut prendre 2 minutes. Voir la section concernant la [cross-compilation](#cross-compilation)

#### Exécution de MOvIT-Detect
L'exécutable sera alors créé dans le dossier Executables, sous le nom movit-pi. Le programme doit obligatoirement être exécuté avec sudo, autrement le système ne démarrera pas. Il est possible de démarrer le programme compilé avec la commande suivante:
```bash
sudo ./movit-pi
```
La sortie en console affiche l'état de chacun des capteurs, et des machines à états finis. L'état de connexion de chacun des capteurs est aussi présent.

## 2.4. Cross-compilation
Le projet contient deux Makefiles, soit MakefilePI, le fichier à utiliser pour compiler le projet directement sur le Raspberry Pi Zero W, ainsi qu'un autre nommé Makefile. Ce second Makefile est utilisé pour la Cross-Compilation. Celui-ci contient des fichiers avec un lien absolu qu'il faut remplacer avant de pouvoir l'utiliser.
#### Utiliraire de cross-compilation
L'utilitaire de cross-compilation utilisé est [celui-ci](https://github.com/raspberrypi/tools/tree/master/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf), soit celui proposé dans le répertoire GitHub de RaspberryPi. Il est utilisé en conjonction avec les librairies mosquitto-broker libmosquitto et bcm2835. (**\*\*Origine inconnue !?\*\***)
___

<br>
<br>

# 3. Explication du code
## 3.1. Machines à états finis
Le système embarqué est régi par différente machine à états finis _(fsm / finite state machines)_. Voici le détail de chacune d'entre elle :

#### Machine à états des bascules
La machine à état fini de détection de bascule permet de générer des événements de bascules lorsque le fauteuil quitte la zone de détection de -5° à 10°. Voici le schéma de cette machine a état fini.
![images](images/AngleFSM.png)

#### Machine à états des déplacements
La machine à état fini de détection de déplacement détecte lorsque le fauteuil a subi un déplacement sur une certaine distance et généré un événement de déplacement. Voici le schéma de cette machine à état fini.
![images](images/TravelFSM.png)

#### Machine à états de détection de présence
La machine a états finis de détection de présence, détecte lorsqu'une personne est assise sur le fauteuil. Elle permet d'éviter au possible les fausses détections. Voici le schéma de cette machine à état fini.
![images](images/SeatingFSM.png)

#### Machine à états des notifications
La machine à état fini des notifications permet de générer des notifications de bascule après un certain temps que la personne est assise sur le fauteuil, elle gère aussi les mises en veille de bascule. Voici le schéma de cette machine à états fini.
![images](images/NotificationFSM.png)

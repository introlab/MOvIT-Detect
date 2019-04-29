# MOvIT-Detect
MOvIT-Detect est la partie capteur du projet MOvIT+, elle prend en charge différents capteurs i2c, ces capteurs servent à déterminer l'état du fauteuil et de son passager. Le code d'acquisitions des capteurs est en C++ et communique via MQTT au backend. Les capteurs pris en charge actuellement sont les suivants:

| Capteur | Utilité | Communication | Adresse |
| --- | --- | --- | --- |
| Accéléromètre MPU6050 | À l'aide de deux accéléromètres, il est possible de connaitre l'angle de la chaise selon le fauteuil en entier, et ainsi connaitre la position du patient | I²C | 0x68-0x69 |
| I/O Extender PCA9536 | Utilisé pour la télécommande, permet le contrôle de deux LED, une rouge et une verte, d'un moteur pour des vibrations et la lecture d'un bouton poussoir | I²C | 0x41 |
| ADC MAX11611 | ADC de 10-bit a 12 canaux, les 9 premiers sont utilisés pour la lecture de 9 capteurs à pression, sous le siège du patient, permet alors de connaitre le centre de gravité du patient, et la détection du patient sur le fauteuil | I²C | 0x35 |
| RTC MCP79410 | Permet de garder la date et l'heure du raspberry pi en temps réel, et ce même lorsqu'il est déconnecté, le RTC ce met a jour si besoin avec le serveur NTP | I²C | 0x35 & 0x6f |
| ToF Ranging VL53L0X | Permet de calculer la distance entre le sol et le fauteuil, utiliser avec le capteur de Flow PWM3901, cela permet de calculer la vitesse du fauteuil | I²C | 0x29 |
| Optical Flow SensorPMW3901 | Calcul la distance en deltaX et deltaY de son changement de position, permet de savoir si le fauteuil est en mouvement, et de quelle distance celui-ci c'est déplacé | SPI | X |


# Configuration des capteurs
Il faut pour commencé activé le port I²C et le port SPI du Raspberry Pipour ce faire on utilise l'utilitaire `raspa-config`
```bash
sudo raspi-config
```
##### Activation I²C
Activation du I²C avec raspa-config
- Option 5
- Option P5
- Choisir Yes

##### Activation SPI
Activation du SPI avec raspa-config
- Option 5
- Option P4
- Choisir Yes

##### Expend FileSystem (Optionel)
Expending FileSystem avec raspa-config
- Option 7
- Option A1

Finalement choisir Finish et accepté de redémarré

## Installation i2cdetect
i2cdetect permet de savoir quels appareils i2c sont connectés au système, de sorte a être sur que l'appareil est bien connecté et que son adresse est bel et bien la bonne, on installe cet outil comme suit:
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
## RTC MCP79410
Le RTC doit être activé dans Linux afin de pouvoir garder l'heure adéquatement, il faut activer le device tree overlay du système Linux afin qu'il active le RTC au démarrage:
```bash
sudo echo "rtc-mcp7941x" | sudo tee --append /etc/modules
sudo echo "dtoverlay=i2c-rtc,mcp7941x" | sudo tee --append /boot/config.txt 
sudo reboot now
```
On peut confirmer que le RTC est bel et bien connecté a l'aide de `i2cdetect -y 1` on obtient la sortie suivante:
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
Le RTC est utilisé par le système Linux, car 6f a été remplacé par UU, il faut ensuite modifier le fichier /lib/udev/hwclock-set et commenté certaines lignes:
```bash
sudo nano /lib/udev/hwclock-set
```
Les lignes a commenté sont les suivantes:
```bash
if [ -e /run/systemd/system ] ; then
    exit 0
fi
```
Il faut ajouter une # devant chacune des lignes de sorte à obtenir:
```bash
#if [ -e /run/systemd/system ] ; then
#    exit 0
#fi 
```
## Vérification de la date et heure
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

# Installation de MOvIT-Detect
Il faut installer les libraires de mosquitto a afin de pouvoir utiliser le MQTT comme moyen de communication:
```bash
wget http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key
sudo apt-key add mosquitto-repo.gpg.key
cd /etc/apt/sources.list.d/
sudo wget http://repo.mosquitto.org/debian/mosquitto-stretch.list
sudo apt-get update
sudo apt-get install -y libmosquitto-dev libmosquittopp-dev libssl-dev
```

Il faut ensuite cloner ce Repo et y accéder:
```bash
git clone https://github.com/introlab/MOvIT-Detect.git
cd MOvIT-Detect
```

Il faut également la librairie bcm2835 pour communiquer avec les ports GPIO du processeur la librairie a été modifié pour ajouter des timeouts, elle est disponible a la racine du projet:
```bash
cd MOvIT-Detect/bcm2835-1.58
./configure
make
sudo make check
sudo make install
```

Pour compiler le projet, il suffit d'aller dans le dossier Movit-pi et d'exécuté `make`:
```bash
cd MOvIT-Detect/Movit-Pi
make all
```
L'exécutable sera alors créé dans le dossier Executables, sous le nom movit-pi. Le programme doit obligatoirement être exécuté avec sudo, autrement le système ne démarrera pas. Il est possible de démarrer le programme compilé avec la commande suivante:
```bash
sudo ./movit-pi
```
La sortie en console affiche l'état de chacun des capteurs, et des machines à états finis. L'état de connexion de chacun des capteurs est aussi présent.

# Les Makefiles
Le projet contient deux Makefiles, soit MakefilePI, le fichier utiliser pour compiler le projet directement sur le Raspberry Pi Zero W, ainsi qu'un autre nommé Makefile. Ce second Makefile est utilisé pour la Cross-Compilation. Ce MakeFile contient des fichiers avec un lien absolu qu'il faut remplacer avant de pouvoir l'utiliser.

# Machines à états finis
Le système embarqué est régi par différente machine à états finis voici le détail de chacune d'elle

## Machine à états des bascules
La machine à état fini de détection de bascule permet de générer des événements de bascules lorsque le fauteuil quitte la zone de détection de -5° à 10°. Voici le schéma de cette machine a état fini.
![images](images/AngleFSM.png)

## Machine à états des déplacements
La machine à état fini de détection de déplacement détecte lorsque le fauteuil a subi un déplacement sur une certaine distance et généré un événement de déplacement. Voici le schéma de cette machine à état fini.
![images](images/TravelFSM.png)

## Machine à états de détection de présence
La machine a états finis de détection de présence, détecte lorsqu'une personne est assise sur le fauteuil. Elle permet d'éviter au possible les fausses détections. Voici le schéma de cette machine à état fini.
![images](images/SeatingFSM.png)

## Machine à états des notifications
La machine à état fini des notifications permet de générer des notifications de bascule après un certain temps que la personne est assise sur le fauteuil, elle gère aussi les mises en veille de bascule. Voici le schéma de cette machine à états fini.
![images](images/NotificationFSM.png)

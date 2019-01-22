# MOvIT-Detect
MOvIT-Detect est la partie capteur du projet MOvIT+, elle prends en charge différents capteurs i2c, ces capteurs servent a déterminé l'état du fauteuil et de sont passager. Le code d'acquisitions des capteurs est en C++ et communique via MQTT au backend. Les capteurs prit en charge présentement sont les suivant:
| Capteur | Utilité | Communication | Adresse |
| :------: | :------ | :------: | :------: |
| Accéléromètre MPU6050 | A l'aide de deux accéléromètre, il est possible de connaitre l'angle de la chaise selon le fauteil en entier, et ainsi connaitre la position du patient | I²C | 0x68-0x69 |
| I/O Extender PCA9536 | Utiliser pour la télécommande, permet le controle de deux leds, une rouge et une vert, d'un moteur pour des vibration et la lecture d'un bouton poussoir | I²C | 0x41 |
| ADC MAX11611 | ADC de 10-bit a 12 canal, les 9 premiers sont utilisé pour la lecture de 9 capteurs a pression, sous le siège du patient, permet alors de connaitre le centre de gravité du patient, et la détection du patient sur le fauteuil | I²C | 0x35 |
| RTC MCP79410 | Permet de garder la date et l'heure du raspberry pi en temps réel, et ce même lorsqu'il est deconnecté ce met a jour si besoin avec le serveur NTP | I²C | 0x35 & 0x6f |
| ToF Ranging VL53L0X | Permet de calculer la distance entre le sol et le fauteuil, utiliser avec le capteur de Flow PWM3901, cela permet de calculer la vitesse du fauteil | I²C | 0x29 |
| Optical Flow SensorPMW3901 | Calcul la distance en deltaX et deltaY de son changement de position, permet de savoir si le fauteuil est en mouvement, et de quel distance celui-ci c'est déplacé | SPI | X |

# Configuration des capteurs
Il faut pour commencé activé le port I²C et le port SPI du Raspberry Pipour ce faire on utilise l'utilitaire `raspi-config`
```bash
sudo raspi-config
```
##### Activation I²C
Activation du I²C avec raspi-config
- Option 5
- Option P5
- Choisir Yes

##### Activation SPI
Activation du SPI avec raspi-config
- Option 5
- Option P4
- Choisir Yes

##### Expend FileSystem (Optionel)
Expending FileSystem avec raspi-config
- Option 7
- Option A1

Finalement choisir Finish et accepté de redémarré

## Installation i2cdetect
i2cdetect permet de savoir quels appareils i2c sont connecté au système, de sorte a être sur que l'appareil est bien connecté et que son adresse est bel et bien la bonne, on installe cette outil comme suit:
```bash
sudo apt-get install -y i2c-tools
```
Une fois l'outil installé, on peut afficher la carte des appareils connecté avec la commande suivante:
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
Le RTC doit être activé dans linux afin de pouvoir garder l'heure adéquatement, faut activer le device tree overlay du systeme linux afin qu'il active le RTC au démarrage:
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
Le RTC est utilisé par le système linux, car 6f a été remplacé par UU, il faut ensuite modifier le fichier /lib/udev/hwclock-set et commenté certaines lignes:
```bash
sudo nano /lib/udev/hwclock-set
```
Les lignes a commenté sont les suivantes:
```bash
if [ -e /run/systemd/system ] ; then
    exit 0
fi
```
Il faut ajouter une # devant chacune des lignes de sorte a obtenir:
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
Si la date est incorrecte, il faut changer de TimeZone, voici comment procédé, a l'aide de l'utilitaire `raspi-config` il faut:
- Choisir l'option 4
- Puis l'option I2
- Choisir ensuite le bon TimeZone selon votre location
- Choisir finish

Confirmer la date et l'heure avec `date`, si celle-ci est bonne, il faut l'écrire dans le RTC avec la commande suivante:
```bash
sudo hwclock -w
```

# Installation de MOvIT-Detect
Il faut installer les libraires de mosquitto a afin de pouvoir utiliser le mqtt comme moyen de communication:
```bash
wget http://repo.mosquitto.org/debian/mosquitto-repo.gpg.key
sudo apt-key add mosquitto-repo.gpg.key
cd /etc/apt/sources.list.d/
sudo wget http://repo.mosquitto.org/debian/mosquitto-stretch.list
sudo apt-get update
sudo apt-get install -y libmosquitto-dev libmosquittopp-dev libssl-dev
```

Il faut également la librairie bcm2835 pour communiquer avec les ports GPIO du processeur voici comment l'installer:
```bash
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.58.tar.gz
tar zxvf bcm2835-1.58.tar.gz
cd bcm2835-1.58
./configure
make
sudo make check
sudo make install
cd .. && rm -r -f bcm2835*
```

Il faut ensuite cloner ce Repo et y accéder:
```bash
git clone https://github.com/introlab/MOvIT-Detect.git
cd MOvIT-Detect
```

Pour compiler le projet, il suffit d'aller dans le dossier Movit-pi et d'executé `make`:
```bash
cd MOvIT-Detect/Movit-Pi
make all
```
Deux executables seront alors créer dans le dossier Executables, soit movit-pi et movit-control, le premier s'occupe des capteurs, le second demarre movit-pi et le backend. Il est possible de démarrer le tout avec la commande suivante
```bash
sudo ./movit-control
```

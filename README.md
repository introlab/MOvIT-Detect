# MOvIT-Detect
The embedded system part of the MOvIT-Detect project.

## Getting started

- Télécharger [Ubuntu 18.04 64bit](http://releases.ubuntu.com/18.04/)
- Installer Ubuntu dans une machine virtuelle ou sur une machine physique
- Faire l'installation normale
- Une fois l'installation terminé, ouvrir un terminal et entrer la commande:
``` shell
    sudo apt-get install make git
```
- Télécharger le cross-compiler: [movit-cross-compiler](https://1drv.ms/u/s!AjyPHvJRDLOMrhWOt6g2g4qBiGZn)
- L'extraire dans votre Ubuntu:
``` shell
    cd /usr/local
    sudo tar -xzvf /path/to/file/movit-cross-compiler.tar.gz
```
- Déplacez vous où vous voulez travailler (Ex: dossier partagé avec votre host OS)
- Cloner le repo git sur la machine Ubuntu ou dans votre dossier partagé avec le host OS:
```shell
    git clone https://github.com/AustinDidierTran/MOvIT-Detect.git
```
- Se déplacer où le code est situé:
```shell
    cd MOvIT-Detect/Movit-Pi/
```
- Finalement compiler le code:
```shell
    make
```
- Note: Si vous buildé sur une VM à partir d'un dossier partagé et que vous voyez `Clock skew detected.  Your build may be incomplete.` dans le build, vous pouvez utiliser la commande suivante pour régler ce problème:
``` shell
find . -exec touch {} \;
```

## Exécuter le code sur le RaspberryPi

- Copier le fichier `output/movit-pi` sur le RaspberryPi
- Excécuter le fichier en faisant:
```shell
    sudo ./movit-pi
```
Note: Si le fichier est présent, mais qu'en essayant de l'exécuter ça ne trouve pas le fichier, c'est surement parce que le fichier n'a pas les permissions pour être exécuté. Pour régler ce problème, il suffit de faire:
```shell
    sudo chmod +x movit-pi
```

## Configurer la connexion à internet sur le RaspberryPi

### Si vous avez accès au système de fichier du RaspberryPi
- Ouvrir le fichier `wpa_supplicant.conf`
```shell
sudo nano /etc/wpa_supplicant/wpa_supplicant.conf
```
- Entrer les informations du WiFi auquel vous voulez vous connecter. Voici un exemple de a quoi devrais ressembler le contenu:
```
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1

network={
ssid="NOM_DU_WIFI"
psk="PASSWORD_DU_WIFI"
proto=RSN
key_mgmt=WPA-PSK
pairwise=CCMP
auth_alg=OPEN
}

```
- Redémarrer le RaspberryPi
```shell
sudo reboot
```

### Si vous n'avez pas accès au système de fichier du RaspberryPi
- Brancher la carte microSD de votre RaspberryPi dans votre ordinateur. Vous allez voir le drive `boot` apparaitre. Ouvrez-le. 
- Ajouter le fichier `wpa_supplicant.conf` sur ce drive avec le contenu suivant:
```
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1

network={
ssid="NOM_DU_WIFI"
psk="PASSWORD_DU_WIFI"
proto=RSN
key_mgmt=WPA-PSK
pairwise=CCMP
auth_alg=OPEN
}

```
- Sauvegardez le fichier et éjectez le drive
- Inserez la carte microSD dans votre RaspberryPi et branchez-le.
- Et voilà, le RaspberryPi devrait être connecté à votre Wifi.
Note: Ça ne fonctionnera pas à l'école

## Configurer  et utiliser le scanneur de SonarQube

### Configuration

- Pour pouvoir utiliser le scanneur de SonarQube, il faut préalablement avoir installé [java version 8](https://www.java.com/fr/download/).

- Ensuite, télécharger le scanneur de SonarQube.
[Windows 64 bit](https://sonarsource.bintray.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-3.2.0.1227-windows.zip),
[Linux 64 bit](https://sonarsource.bintray.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-3.2.0.1227-linux.zip),
[Mac OS X 64 bit](https://sonarsource.bintray.com/Distribution/sonar-scanner-cli/sonar-scanner-cli-3.2.0.1227-macosx.zip).

- Décompresser le fichier et le déposer à l'endroit de votre choix.
Ensuite il faut ajouter le dossier "bin" aux variables système.
Sous Windows cela équivaut à rajouter par exemple le dossier "C:\SonarScanner\bin" à la variable "Path" sous les variables système.

### Utilisation

- Une fois que la configuration est faite, pour utiliser le scanneur on doit ouvrir une invite de commande à la racine du projet et y taper la commande 

```shell
sonar-scanner
```
- Laisse le processus s’exécuter.
- Pour consulter les résultats, se rendre [ici](http://sonarqubemovitplus.ddns.net:9000).

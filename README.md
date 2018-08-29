# MOvIT-Detect
The embedded system part of the MOvIT-Detect project.

## Getting started

- Télécharger [Ubuntu 18.04 64bit](http://releases.ubuntu.com/18.04/)
- Installer Ubuntu dans une machine virtuelle ou sur une machine physique
- Faire l'installation normal
- Une fois l'installation terminé, ouvrir un terminal et entrez la commande:
``` shell
    sudo apt-get install make git
```
- Télécharger le cross-compiler: [movit-cross-compiler](https://1drv.ms/u/s!AjyPHvJRDLOMrhWOt6g2g4qBiGZn)
- L'extraire dans votre Ubuntu:
``` shell
    cd /usr/local
    sudo tar -xzvf /path/to/file/movit-cross-compiler.tar.gz
```
- Cloner le repo git sur la machine Ubuntu:
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

## Exécuter le code sur le RaspberryPi

- Copier le fichier `output/movit-pi` sur le RaspberryPi
- Excécuter le fichier en faisant:
```shell
    sudo ./movit-pi
```
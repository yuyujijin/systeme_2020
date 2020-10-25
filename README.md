RENDU N°1:

Le projet se teste sur la branche develop, dans le fichier src/ (ce n'est pas la meilleure option, c'est une erreur d'attention, ce sera changer). On le compile avec "make". Pour l'exécuter, on utilise la commande "./shell" qui ouvre un terminal.

Les commandes disponibles jusqu'à présent sont les commandes "pwd", "ls", "cat" et "mkdir".

La commande pwd ...

La commande ls ...

La commande cat ...

La commande mkdir fonctionne pour l'instant uniquement depuis l'éxterieur d'un ".tar", mais permet de créer des fichiers à la fois dans des ".tar" et dans des fichiers lambda. Comme la commande mkdir originelle, elle vérifie que le dossier ne soit pas deja présent dans le ".tar" et que ce ".tar" soit bien une tarball et pas un dossier appelé "xxx.tar".

Pour supprimer les executables ainsi que les fichier .o, on peut utiliser la commande "make cleanall".
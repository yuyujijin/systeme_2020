**RENDU N°1:**

Le projet se teste sur la branche rendu1, dans le fichier src/ (ce n'est pas la meilleure option, c'est une erreur d'attention, ce sera changer). On le compile avec "make". Pour l'exécuter, on utilise la commande "./shell" qui ouvre un terminal.

Les commandes disponibles jusqu'à présent sont les commandes "pwd", "ls", "cat", "mkdir" et "cd".

La commande pwd écrit le chemin actuel (arborescence tar incluses) dans la sortie standard.

La commande ls fonctionne normalement qu'on soit dans un tarball ou une arborescence normal. L'option -l marche pour les tarballs. On peut regarder a l'interieur d'un tar depuis l'exterieur

La commande cat fonctionne normalement qu'on soit dans un tarball ou une arborescence normal. On peut regarder a l'interieur d'un tar depuis l'exterieur.

La commande cd permet de parcourir tout types d'arborescences (normal, tarball, arborescence dans tarball). "cd ~" ramène au homedir.

La commande mkdir fonctionne normalement qu'on soit dans un tarball ou une arborescence normal et permet de créer des fichiers à la fois dans des ".tar" et dans des fichiers lambda. Comme la commande mkdir originelle, elle vérifie que le dossier ne soit pas deja présent dans le ".tar" et que ce ".tar" soit bien une tarball et pas un dossier appelé "xxx.tar".

Pour supprimer les executables ainsi que les fichier .o, on peut utiliser la commande "make cleanall".
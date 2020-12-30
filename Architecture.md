# Architecture :

## tar_manipulation et useful
Ce sont deux fichiers regroupant quelques fonctions utiles pour la suite du projet;

*  tar_manipulation permet plusieurs manipulations sur les tars, l'ajout de données
(en créant un nouveau fichier, ou en mode 'append') en lisant depuis STDIN, 
la suppression de données, la lecture de données (en les écrivant dans STDOUT)
(lire un fichier) ou même la récuperation précise d'un header.
*  useful implemente sa propre structure, le 'special_path', qui permet de stocker
au travers de 3 champs le path normal, le nom du tar, et le chemin dans ce dernier
par rapport a un argument (ou un chemin en général). Il possède des fonctions
pour créer ce dernier, et permet la simplification général des arguments.

## Shell
Le shell tourne en boucle jusqu'à ce qu'un signal d'arrêt vienne lui dire de
stopper ou que la commande 'exit' soit inscrite.

On commence avec une fonction `read_line()` qui vient lire une ligne dans STDIN,
puis une seconde fonction, `str_cut()` qui vient découper cette dernière en 
utilisant un token (ici `espace`) en un sous tableau d'arguments.

Nous avons donc maintenant deux variable, un tableau d'arguments (argv) et sa taille (argc).

On lance donc execute_redirection(argc,argv) :
On va d'abord parcourir notre tableau d'arguments et procéder de cette manière:
On check si argv[i] est égal soit a `<`, `>`, `>>`, `2>`, `2>>` ou `2>&1`.
Si c'est le cas :
* le prochain args est un fichier normal et donc nous faisons des redirections 
sur des fichier simple (dup2 et open).">" et ">>" sont différencier par 
l'ouverture du fichier en mode O_TRUNCou O_APPEND
* le prochain args est un fichier dans un tar. Pour < nous lisons les données 
de l'argument dans une pipe en utilisant `rdTar()`.

Pour `>`,`>>`,`2>` et `2>>`, on redirige la sortie (erreur pour 2> && 2>>) sur 
une pipe. `2>&1` redirige simplement la sortie erreur dans la sortie standard.
`>` et `>>` (ainsi qu'avec le `2`) se différencient dans le cas
d'un tar par le fait que `>` supprime le fichier puis rajoute du contenu dans le
tar à l'aide de `addTar()`, tandis que `>>` utilise `appendTar()` (qui copie le
fichier, supprime le premier, rajoute du contenu a la fin du tar et mets à jour
le header).

Si ce n'est pas le cas alors argv[i] est ajouter a argv_no_redirection
A la fin on lance `execute_pipe_cmd(nouveau_argc,argv_no_redirection)`

Pour faire simple, cmd > a > b ---> `execute_pipe_cmd(nbr_arg_cmd,cmd)`

`execute_pipe_cmd()` va lui gerer les cas de pipe `|`, et créer un nouveau
pipe pour chaque pipe rencontré, et créer un nombre de fils suffisant.
Chaque fils se voit rediriger son entrée sur la sortie du précedent (si il 
n'est pas premier), et sa sortie sur l'entrée du suivant (si il n'est pas dernier).

puis enfin nous lançons execute_cmd() sur chaque commande entre les `|`

Deux cas :
- Soit nous exécutons sur aucun tar alors on utilise exec standard
- Soit nous avons des tar, fichier dans tar etc et nous utilisons nos propres 
commandes en faisant exec dessus


### ls

  On utilise special_path_maker et qui nous donne donc l'emplacement de notre argument.

  Si c'est un fichier normal, on fait un simple ls en prenant en compte des options.

  Si c'est un tar, ou dossier dans un tar, on boucle avec un read, et on affiche soit juste les nom des fichier du dossier ou soit les informations du fichier suivant l'option -l

### cat
  On utilise special_path_maker et qui nous donne donc l'emplacement de notre argument.

  Si c'est un fichier normal, on fait un simple cat

  Si c'est un tar, on applique rdTar() qui affiche les données du fichier

### cd
cd utilise les fonctions de useful pour permettre de 'tester' la possibilitée
d'un chemin : a partir du chemin qui lui est donné en argument, il récupère
donc une structure avec 3 champs.
Si le champs renseignant le 'véritable' chemin n'est pas accessible, il retourne
une erreur.
Si le champs renseignant le nom du tar n'est pas accessible (le tar n'existe pas),
il retourne aussi une erreur.
Enfin, si le champs renseignant le chemin dans le tar contient '.tar' ou que le
fichier n'existe pas dans ce tar, il retourne... `une erreur`.

Si tout les tests sont passés, cd met a jour les deux variables d'environnement
`TARNAME` et `TARPATH`.

### pwd
pwd récupère le cwd a l'aide de la fonction `getcwd`, le nom du tar ("" si pas dans
un tar) a l'aide de la variable d'env. `TARNAME` et de même pour le chemin dans
le tar, concatene ces 3 variables et écrit dans STDIN ce chemin

### cp
`cp`, `cp -r` et `cp avec plus de 2 arguments`, utilisent tous la fonction de cp à 2
arguments qui fonctionne comme ceci :
* Si ses 2 arguments renseignent des fichier qui sont en fait pas dans un tar,
elle exec cp.
Sinon, elle vérifie l'existence de son premier argument (et qu'il n'est pas un
dossier), la non existence de son second et si ces conditions sont vérifiées,
et créer un fils qui va lire dans le premier argument et écrire dans un pipe,
et le père lit dans le pipe et écrit dans le second argument.
(Ces 2 fonctions gèrent elle même la différence fichier tar / non tar)
* cp -r, elle regarde si son premier argument est un dossier, et si ce n'est pas
le cas, elle rapelle simplement cp sur ses 2 arguments.
Si c'est le cas, elle tente de créer le dossier du nom du 2ème argument, puis
liste tout les noms de fichier dans le premier arguments, et rapelle cp -r
avec comme premier argument, le premier argument + le nom du fichier, et en second
argument le second argument + le nom du fichier.
* cp avec plusieurs argument parcours juste tout les arguments, et copie
l'argument i sur l'argument n - 1


### mkdir
On commence par récupérer le vrai chemin, qui prend en compte les . et les .. .
Si on est dans un tar, on renvoie une exception si jamais on veut créer un tar imbriqué. Si ça n'est pas le cas, on appelle `addDirTar(tarloction,tarpath)`, où tarlocation est l'endroit où se situe le tar et tarpath le chemin du repertoire à supprimer dans le tar. La fonction vérifie si le chemin existe déjà, et l'ajoute si ça n'est pas le cas.

### rmdir
Comme précedemment, on commence par récupérer le vrai chemin et on utilise `special_path_maker ` pour récupérer les informations relatives au tar.
Dans le cas ou rmdir pointe un .tar, qu'il sagisse d'un fichier .tar ou d'un répertoire (vide), on cherche à le supprimer. On fait donc appel à `rm -r`.
Si le fichier n'est pas vide on renvoie une erreur.
Dans le cas où le chemin ne pointe pas un .tar mais utilise un chemin qui passe par un .tar, on cherche à gérer les exceptions : si le chemin vise un fichier au lieu d'un répertoire, si le chemin n'existe pas, ou si le répertoire n'est  pas vide. Puis on supprime à l'aide de `rmTar`.

### rm
On commence par récupérer le vrai chemin et on utilise `special_path_maker ` pour récupérer les informations relatives au tar.
Si le chemin pointe un tar, et que l'option `-r` est précisée, on supprime le tar, sinon on renvoie une erreur.
Dans `rm_tar`, si l'option `-r` n'est pas précisée, on fait appel à `rmTar`, sinon on recherche, dans les fichiers du tar, ceux qui ont un préfixe identique au nom de répertoire que l'on veut supprimer. Si oui, on les supprime.

### mv
  Si on a un fichier normal on applique le vrai mv dessus

  On appelle cmds/cp -r [argv] , ce qui va copier nos fichier ou dossier dans la destination( dernier arguments)

  Puis on apelle cmds/rm -r [argv -1], ce qui va supprimer les anciers fichiers ou dossier sauf la destination

# Architecture :


## Shell
On commence avec une fonction `read_line()` qui nous découpe notre ligne en plusieurs arguments séparés par des espaces.

Nous avons donc maintenant deux variable, un tableau d'arguments (argv) et sa taille (argc).

On lance donc execute_redirection(argc,argv) :

On va d'abord parcourir notre tableau d'arguments et procéder de cette manière:

On check si argv[i] est égal soit a "<", ">", ">>", "2>" ou "2>>".
Si c'est le cas.

 -le prochain args est un fichier normal et donc nous faisons des redirections sur des fichier simple (dup2 et open).">" et ">>" sont différencier par l'ouverture du fichier en mode O_TRUNCou O_APPEND

 -le prochain args est un fichier dans un tar. Pour < nous lisons les données de l'argument dans une pipe en utilisant `rdTar()`.
   Pour > >> 2> >>, on redirige la sortie (sortie erreur pour 2>) sur une pipe. > >> sont différencier par le fait que pour > on va supprimer le fichier en question puis ne rajouter que son posix_header. Avec >> on va utiliser la fonction `appendTar()`

   Si il n'y a pas de > alors argv[i] est ajouter a argv_no_redirection


   A la fin on lance `execute_pipe_cmd(nouveau_argc,argv_no_redirection)`


   Pour faire simple cat > a > b c ---> `execute_pipe_cmd(2,{"cat","c"})`

  `execute_pipe_cmd()` va lui gerer les cas de pipe '|', pour chaque fois qu'on | on crée une nouvelle pipe et redirigeons la sortie dessus.


   puis enfin nous lançons execute_cmd() sur notre nouvelle liste d'arguments.

   Deux cas :
    - Soit nous exécutons sur aucun tar alors on utilise exec standard
    - Soit nous avons des tar, fichier dans tar etc et nous utilisons nos propres commandes en faisant exec dessus


### ls

  On utilise special_path_maker et qui nous donne donc l'emplacement de notre argument.

  Si c'est un fichier normal, on fait un simple ls en prenant en compte des options.

  Si c'est un tar, ou dossier dans un tar, on boucle avec un read, et on affiche soit juste les nom des fichier du dossier ou soit les informations du fichier suivant l'option -l

### cat
  On utilise special_path_maker et qui nous donne donc l'emplacement de notre argument.

  Si c'est un fichier normal, on fait un simple cat

  Si c'est un tar, on applique rdTar() qui affiche les données du fichier

### cd

### pwd

### cp

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

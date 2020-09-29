SYSTÈME
==================

**L3 Informatique**


Il est important de bien lire le sujet jusqu'au bout et de bien y réfléchir
avant de se lancer dans la programmation du projet.


## Sujet : `tsh`, un shell pour les tarballs

Le but du projet est de faire tourner un shell qui permet à
l'utilisateur de traiter les tarballs comme s'il s'agissait de
répertoires, **sans que les tarballs ne soient désarchivés**. Le
format précis d'un tarball est décrit sur
[https://fr.wikipedia.org/wiki/Tar_(informatique)](https://fr.wikipedia.org/wiki/Tar_%28informatique%29).

Le shell demandé doit avoir les fonctionnalités suivantes :

* les commandes `cd` et `exit` doivent exister (avec leur comportement habituel)
* toutes les commandes externes doivent fonctionner normalement si leur déroulement n'implique pas l'utilisation d'un fichier (au sens large) dans un tarball
* `pwd` doit fonctionner y compris si le répertoire courant passe dans un tarball
* `mkdir`, `rmdir` et `mv` doivent fonctionner y compris avec des chemins impliquant des tarball quand ils sont utilisés sans option
* `cp` et `rm` doivent fonctionner y compris avec des chemins impliquant des tarball quand ils sont utilisés sans option ou avec l'option `-r`
* `ls` doit fonctionner y compris avec des chemins impliquant des tarball quand il est utilisé sans option ou avec l'option `-l`
* `cat` doit fonctionner y compris avec des chemins impliquant des tarball quand il est utilisé sans option
* les redirections de l'entrée, de la sortie et de la sortie erreur (y
  compris sur des fichiers d'un tarball) doivent fonctionner
* les combinaisons de commandes avec `|` doivent fonctionner

On ne vous demande pas de gérer les cas de tarballs imbriqués (un tarball dans un autre tarball).

Tous les processus lancés à partir de votre shell le seront en premier-plan.

## Modalités d'exécution (et de test)

Les projets seront testés sur la distribution [https://mirrors.ircam.fr/pub/mx/isos/ANTIX/Final/antiX-19/antiX-19.2.1_386-base.iso](https://mirrors.ircam.fr/pub/mx/isos/ANTIX/Final/antiX-19/antiX-19.2.1_386-base.iso). À vous de faire en sorte que ça fonctionne (nous n'accepterons pas d'argument de type "ça fonctionne chez moi").

La présence de tests dans le projet sera grandement appréciée.

## Modalités de rendu

Le projet est à faire par équipes de 2 ou 3 étudiants. Aucune exception ne sera
tolérée et nous avons une nette préférence pour les équipes de 3 (les équipes de 2 ne pourront pas utiliser le fait qu'elles ont moins de membres pour demander une quelconque indulgence car c'est elles qui auront fait ce choix-là). La composition de
chaque équipe devra être envoyée par mail à l'enseignante responsable
du cours de systèmes **au plus tard le 1er octobre 2020**, avec copie à
chaque membre de l'équipe.

Chaque équipe doit créer un dépôt `git` privé sur
le [gitlab de l'UFR](https://gaufre.informatique.univ-paris-diderot.fr/) **dès
la lecture du sujet** (maintenant!) et y donner accès en tant que `Reporter` à
tous les enseignants du cours de Système : Edwin Hamel-de le Court, Ines
Klimann, Anne Micheli et Dominique Poulalhon. Le dépôt devra contenir un
fichier `AUTHORS` donnant la liste des membres de l'équipe (nom, prénom,
numéro étudiant et pseudo(s) sur le gitlab).

En plus du programme demandé, vous devez fournir un `Makefile` utilisable, un
mode d'emploi, et un fichier `ARCHITECTURE` (idéalement en format Markdown,
donc avec extension `.md`) expliquant la stratégie adoptée pour répondre au
sujet (notamment l'architecture logicielle, les structures de
données et les algorithmes implémentés).

En cas de question et si la réponse n'est pas contenue dans le présent
document, merci de poser la question sur le forum `moodle` dédié du
cours de systèmes ou dans le salon quesions-projet du serveur discord du cours. Seules les réponses postées sur le forum et le salon feront foi
au moment de la soutenance. 

Les seules interdictions strictes sont les suivantes : plagiat (d'un
autre projet ou d'une source extérieure à la licence), utilisation de
la fonction `system` de la `stdlib` et de la commande `tar` (vous
pouvez utiliser la commande `tar` pour créer des archives afin de
tester le projet, mais pas dans le code du projet).

Pour rappel, l'emprunt de code sans citer sa source est un
plagiat. L'emprunt de code en citant sa source est autorisé, mais bien
sûr vous n'êtes notés que sur ce que vous avez effectivement produit.
Donc si vous copiez l'intégralité de votre projet en le spécifiant
clairement, vous aurez quand même 0 (mais vous éviterez une demande de
convocation de la section disciplinaire).


## MCC Session 1
La matière étant en contrôle continu intégral, ce projet donnera lieu
à 3 notes, correspondant à 2 rendus et 1 épreuve.

* un premier rendu aura lieu à mi-semestre, sans soutenance ; l'équipe
  pédagogique vous fera un retour circonstancié sur ce rendu ; il vous
  permettra entre autre de ne pas vous fourvoyer sur le sujet ; **ce rendu est à faire pour le 25 octobre au plus tard, il prendra la forme d'un tag `rendu1` sur votre dépôt git distant** (attention : les tags ne sont pas transmis automatiquement par l'action push sur le serveur distant, assurez-vous que le tag est bien mis à la date demandée, sinon le rendu sera considéré vide; voir par exemple [https://git-scm.com/book/en/v2/Git-Basics-Tagging](https://git-scm.com/book/en/v2/Git-Basics-Tagging))
* un deuxième rendu aura lieu en fin de semestre et sera accompagné
  d'une soutenance si les circonstances sanitaires le permettent
  (sinon rendu seul) ;
* l'épreuve sera personnalisée et portera sur votre projet ; elle sera
  à faire en temps limité sur machine (3h si les circonstances
  sanitaires le permettent, une plage plus longue en cas de confinement).

### Premier rendu
Le premier rendu est au choix de chaque groupe. Il est là pour vous
aider dans l'avancée de votre projet et vous éviter de passer à côté du
sujet. Si vous fournissez des documents explicatifs sur les structures
de données et les algorithmes que vous utiliserez, nous vous donnerons
notre avis dessus. Si vous fournissez du code en documentant ce qu'il
est censé faire, nous le testerons. Si vous n'avez pas bien compris le
sujet, nous vous expliquerons en quoi.

Nous nous attendons à un volume de travail représentant au moins le
tiers du travail final. À noter que le sujet porte à la fois sur le
SGF et les processus, mais qu'au moment du premier rendu nous n'aurons
pas traité en cours la partie processus, il semble donc plus logique
que votre travail porte de préférence sur la partie SGF. Ce travail
peut être constitué de fonctions qui seront utiles au projet, sans
pour autant prendre la forme d'un shell dès le départ ; par exemple un
ensemble de fonctions qui permettent de faire toutes les manipulations
qui seront nécessaires dans un tarball est un travail tout à fait
acceptable pour ce premier rendu.

> → note N1

### Deuxième rendu
Le deuxième rendu est le rendu final de tout le projet. Il doit
correspondre à la description globale du sujet et des attendus décrits
dans ce document. Il doit aussi tenir compte des remarques faites lors
du premier rendu (ou justifier clairement pourquoi vous n'avez pas
tenu compte de ces remarques).

> → note N2

Si N2>N1, on transformera N1 en N2. Nous sommes conscients que ça peut
en pousser certains à négliger le premier rendu : il s'agit pour nous
d'encourager les groupes qui sont capables de tenir compte
de nos remarques et de se remettre en selle après un mauvais premier
rendu. Si vous négligez le premier rendu, vous aurez peu de retours sur
votre travail et vous vous exposez au risque de passer à côté du
sujet, sans aucun garde-fou.

### Épreuve sur machine

Cette épreuve sera personnalisée : chaque étudiant recevra une demande
spécifique à son projet (un point à corriger ou à améliorer, ou un
point à ajouter). Il devra faire un `fork` du projet de son groupe
pour réaliser ce point (nous vous recommandons de regarder avant
l'épreuve comment faire un `fork` d'un dépôt), et mettre tous les
enseignants de la matière `Reporter` sur ce nouveau dépôt.

> → note N3

### Participation effective

La participation effective d'un étudiant au projet de son groupe sera
évaluée grâce, entre autres :

* aux statistiques du dépôt git d'origine ;
* aux réponses aux éventuelles questions portant sur le projet pendant
  la soutenance (si les circonstances sanitaires le permettent, sinon
  par mail ou visio) ;
* à la capacité à travailler sur le projet lors de l'épreuve sur machine.

À noter qu'un étudiant n'ayant aucun commit sur du code, incapable de
répondre aux questions sur le projet de son groupe ou incapable de
travailler sur ce même projet lors de l'épreuve sur machine sera
considéré comme n'ayant pas participé au projet. Sa note finale
sera 0 s'il a participé à l'épreuve sur machine et DEF sinon. Dans les
autres cas, sa note finale sera

> 50% max (N2, 50%N1 + 50%N2) + 50% N3.

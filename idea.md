# IDEAS
## How does <x> works?
**shell.c** : Read a line from STDIN, cut the words contained by this line, and put in a array of string. It then compare its first arguments _(so the cmd name)_ to every know functions, and call it if it matchs.
**pwd.c** : pwd just returns the actual cwd, so the path from root folder.

## How should we implement <x>?
**tarballs as _directory_** :
_(Eugene)_ : I think that we should store somewhere a _symbolic_ path, that complets the real path (the pwd one). If its **empty**, then we're in a _real_ directory, everything act as usual (unless we word on tarball in this directory). But if it contains an adress, then its the _extension_ of the actual pwd, meaning we're inside a tarball. Function should get args +  this path, so they know how to act (if its empty, normally, if not _do your thing_).
For example, if we're inside the directory **/home/<user>/A/B** then this symbolic path is empty. But if directory B contains, lets say a tarball named **C.tar**, as soon as we do **cd C.tar**, symbolic path now contains **"/C.tar"**, meaning that our shell path is now **/home/<user/A/B/C.tar**. Functions can now know if they're in a tarball or not.

##what we discussed about : 
we thought that cmds on tarball for these specifics function could work well :
- mkdir : just add an empty bloc that have the attributs of a folder, and thell the header that it size is now the same as before + the size of a bloc
**IN GENERAL**, every functions kinda works the same; you have to add, or remove something from a tar file, by adding (append) or removing to it a bloc, and tell the header.
**We should do a function for adding a file in an existing tarball, and a function for removing it.**
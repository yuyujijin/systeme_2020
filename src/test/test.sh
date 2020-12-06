#!/bin/bash
echo "fichier1" > fic1
echo "fichier2" > fic2
mkdir dir1
mkdir dir1/dir2
tar cvf test.tar fic1 fic2 dir1
rm -r fic1 fic2 dir1

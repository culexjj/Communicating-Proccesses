#!/bin/bash

cd ./Trabajo2

gcc -w fuente1.c -o Ej1
gcc -w fuente2.c -o Ej2
gcc -w fuente3.c -o Ej3

chmod +x Ej1
chmod +x Ej2
chmod +x Ej3

./Ej1

rm Ej1
rm Ej2
rm Ej3

exit 0


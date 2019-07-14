#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>



char *puntoDeMontajeDelFileSystem;
char directorio[200];

char* directorioNuevaTabla(char *carpeta) {

	scanf(directorio, "%s/%s", puntoDeMontajeDelFileSystem, carpeta);
	return directorio;
}



#ifndef FILESYSTEM_INICIARFILESYSTEM_H_
#define FILESYSTEM_INICIARFILESYSTEM_H_


struct ConfiguracionFS {
	int puertoEscucha;
	char *puntoDeMontaje; //EMPEZAR Y TERMINA CON "/"
	int retardo;
	int tamanioValue; //en bytes
	int tiempoDump;
};

struct metaDataBloques {
	int tamanioBloque;
	int bloques;
	char* magicNumber;
};


	struct ConfiguracionFS configuracionDePrueba;

#endif /* FILESYSTEM_INICIARFILESYSTEM_H_ */

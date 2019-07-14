#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <ctype.h>

int esNumero(char* lstNum);

int main(void) {
	struct sockaddr_in direccionServer;
	direccionServer.sin_family = AF_INET;
	direccionServer.sin_addr.s_addr = inet_addr("127.0.0.1");
	direccionServer.sin_port = htons(8080);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);
	if(connect(cliente, (void*) &direccionServer, sizeof(direccionServer)) != 0) {
		perror("No se pudo conectar");
		return 1;
	}


	char* tamanioPaqueteMsg = malloc(8);
	unsigned short tamanioPaquete;
	while(1) {
		 printf("ingrese tamanio del mensaje: ");
		 scanf("%s", tamanioPaqueteMsg);
		 while(!esNumero(tamanioPaqueteMsg)){
			 printf("\nValor incorrecto. Ingrese tamanio del mensaje: ");
			 scanf("%s", tamanioPaqueteMsg);
		 }
		 send(cliente, tamanioPaqueteMsg, sizeof(tamanioPaqueteMsg), 0);
		 tamanioPaquete = atoi(tamanioPaqueteMsg);
		 char* mensaje = malloc(tamanioPaquete);
		 printf("ingrese mensaje de tamanio <%d>: ", tamanioPaquete);
		 scanf("%s", mensaje);
		 send(cliente, mensaje, strlen(mensaje), 0);
		 free(mensaje);
	 }

	return 0;
}

int esNumero(char* listaNumeros){
	int longitud = strlen(listaNumeros);
	for(int i=0; i<longitud; i++){
		if(!isdigit(listaNumeros[i])){
			return 0;
		}
	}
	return 1;
}

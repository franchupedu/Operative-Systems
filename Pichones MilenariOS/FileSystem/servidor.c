#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>

int main(void) {

	struct sockaddr_in direccionServer;
	direccionServer.sin_family = AF_INET;
	direccionServer.sin_addr.s_addr =INADDR_ANY;
	direccionServer.sin_port = htons(8080);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if (bind(servidor, (void*)&direccionServer, sizeof(direccionServer)) != 0) {
		perror("fallo el bind");
		return 1;
	}

	printf("estoy escuchando\n");
	listen(servidor, 100);

	//------------------Aceptar Clientes-------------------

	struct sockaddr_in direccionCliente;
	unsigned int tamanioDireccion;
	int cliente = accept(servidor, (void*)&direccionCliente, &tamanioDireccion);

	printf("recibi una conexion en %d\n", cliente);
	fflush(stdout);

	send(cliente, "Hola Cliente! :)\n",17, 0);
	//-------------------------------------

	char* tamanioPaqueteMsg = malloc(7);
	int bytesDelTamanioPaq;
	char* buffer;

	while(1) {

		bytesDelTamanioPaq = recv(cliente, tamanioPaqueteMsg, 8, 0);
		if (bytesDelTamanioPaq < 0) {
			perror("Cantidad de datos del paquete incorrecta");
			return 1;
		}
		int tamanioPaquete = atoi(tamanioPaqueteMsg);

		buffer = malloc(tamanioPaquete+1);

//	while(1) {
		int bytesRecibidos = recv(cliente, buffer, tamanioPaquete, 0);
		if(bytesRecibidos <= 0) {
			perror("el chabon se desconecto o bla");
			return 1;
		}

		buffer[bytesRecibidos] = '\0';
		printf("Me llegaron %d de %d bytes con el mensaje: %s\n", bytesRecibidos, tamanioPaquete, buffer);
	}
	free(tamanioPaqueteMsg);
	free(buffer);

	return 0;
}

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void main () {
	int tamaniodireccion = sizeof (struct sockaddr_in);
	int servidor = socket (AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in direccionservidor;
	struct sockaddr_in direccioncliente;
	char *mensaje = malloc (1);
	int tamaniomensaje;
	int activado = 1;
	int cliente;
	int codigo;

	direccionservidor.sin_family = AF_INET;

	direccionservidor.sin_addr.s_addr = INADDR_ANY;

	direccionservidor.sin_port = htons (1024);

	setsockopt (servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof (activado));

	if (bind (servidor, (void*) &direccionservidor, sizeof (direccionservidor)) != 0) {
		perror ("Falló el bind.\n");

		return;
	}

	printf ("Estoy escuchando.\n");

	listen (servidor, SOMAXCONN);

	if ((cliente = accept (servidor, (void*) &direccioncliente, &tamaniodireccion)) == -1)
		perror ("Ocurrió un error.\n");

	else
		printf ("Nueva conexión en %i.\n", cliente);

	while (1) {
		recv (cliente, mensaje, 1, 0);

		codigo = *mensaje;

		if (codigo == 36) {
			printf ("%i 7 journal\n", codigo);

			continue;
		}

		recv (cliente, mensaje, 1, 0);

		tamaniomensaje = *mensaje;

		mensaje = realloc (mensaje, tamaniomensaje);

		recv (cliente, mensaje, tamaniomensaje, 0);

		mensaje [tamaniomensaje] = '\0';

		printf ("%i %i %s\n", codigo, tamaniomensaje, mensaje);

		mensaje = realloc (mensaje, 1);
	}
}

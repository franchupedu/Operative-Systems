#include <readline/readline.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int escuchar () {
	struct sockaddr_in direccioncliente;
	char *mensaje = malloc (1);
	int tamaniomensaje;
	fd_set temporal;
	fd_set general;
	int maximofd;
	int nuevofd;

	FD_ZERO (&general);

	FD_ZERO (&temporal);

	struct sockaddr_in direccionservidor;
	direccionservidor.sin_family = AF_INET;
	direccionservidor.sin_addr.s_addr = INADDR_ANY;
	direccionservidor.sin_port = htons (1024);

	int servidor = socket (AF_INET, SOCK_STREAM, 0);

	int activado = 1;

	setsockopt (servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof (activado));

	if (bind (servidor, (void*) &direccionservidor, sizeof (direccionservidor)) != 0) {
		perror ("Falló el bind.\n");

		return 1;
	}

	printf ("Estoy escuchando.\n");

	listen (servidor, SOMAXCONN);

	FD_SET (servidor, &general);

	maximofd = servidor;

	while (1) {
		temporal = general;

		if (select (maximofd + 1, &temporal, NULL, NULL, NULL) == -1) {
			perror ("Falló el select.\n");

			return 1;
		}

		for (int i = 0; i <= maximofd; i++) {
			if (FD_ISSET (i, &temporal)) {
				if (i == servidor) {
					int tamaniodireccion = sizeof (struct sockaddr_in);

					if ((nuevofd = accept (servidor, (void*) &direccioncliente, &tamaniodireccion)) == -1)
						perror ("Ocurrió un error.\n");

					else {
						FD_SET (nuevofd, &general);

						if (nuevofd > maximofd)
							maximofd = nuevofd;

						printf ("Nueva conexión en %i.\n", nuevofd);
					}
				}

				else {
					mensaje = realloc (mensaje, 2);

					if (recv (i, mensaje, 2, 0) <= 0) {
						printf ("El cliente %i se desconectó.\n", i);

						close (i);

						FD_CLR (i, &general);
					}

					else {
						tamaniomensaje = atoi (mensaje);

						mensaje = realloc (mensaje, tamaniomensaje);

						mensaje [tamaniomensaje] = '\0';

						if (recv (i, mensaje, tamaniomensaje, 0) <= 0) {
							printf ("El cliente %i se desconectó.\n", i);

							close (i);

							FD_CLR (i, &general);
						}

						else {
							for (int j = 0; j <= maximofd; j++) {
								if (FD_ISSET (j, &general)) {
									if (j != servidor && j != i) {
										if (send (j, mensaje, tamaniomensaje, 0) == -1)
											perror ("Error al enviar un mensaje.\n");
									}

									else {
										if (j == servidor)
											printf ("%s\n", mensaje);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	free (mensaje);

	close (servidor);

	return 0;
}

void castear (int numero, char *digitos) {
	digitos [1] = numero - numero % 10 * 10 + 48;

	digitos [0] = numero % 10 + 48;
}

int hablar () {
	char *tamaniomensajecasteado = malloc (2);
	char *caracter = malloc (1);
	char *mensaje = malloc (1);
	int tamaniomensaje = 0;

	struct sockaddr_in direccionservidor;
	direccionservidor.sin_family = AF_INET;
	direccionservidor.sin_addr.s_addr = INADDR_ANY;
	direccionservidor.sin_port = htons (1024);

	int cliente = socket (AF_INET, SOCK_STREAM, 0);

	if (connect (cliente, (void*) &direccionservidor, sizeof (direccionservidor)) != 0) {
		perror ("No me pude conectar.\n");

		return 1;
	}

	printf ("Estoy aquí.\nMensaje para el servidor: ");

	*caracter = getchar ();

	while (*caracter != '\n') {
		while (*caracter != '\n') {
			mensaje = realloc (mensaje, tamaniomensaje + 1);

			mensaje [tamaniomensaje] = *caracter;

			tamaniomensaje++;

			*caracter = getchar ();
		}

		castear (tamaniomensaje, tamaniomensajecasteado);

		send (cliente, tamaniomensajecasteado, 2, 0);

		printf ("Enviar.");

		scanf ("%c", caracter);

		send (cliente, mensaje, tamaniomensaje, 0);

		tamaniomensaje = 0;

		printf ("Mensaje para el servidor: ");

		*caracter = getchar ();
	}

	free (mensaje);

	tamaniomensaje = 20;

	castear (tamaniomensaje, tamaniomensajecasteado);

	send (cliente, tamaniomensajecasteado, 1, 0);

	printf ("Terminar.");

	scanf ("%c", caracter);

	send (cliente, "Adiós servidor ;).\n", 20, 0);

	close (cliente);

	return 0;
}

int main () {
	char *comando = malloc (10);
	char *basura = malloc (1);

	while (1) {
		printf ("####### COMANDOS #######");

		printf ("\n\n");

		printf ("escuchar");

		printf ("\n");

		printf ("hablar");

		printf ("\n");

		printf ("terminar");

		printf ("\n\n");

		printf ("Comando: ");

		scanf ("%s", comando);

		scanf ("%c", basura);

		if (! strcmp (comando, "escuchar"))
			escuchar ();
		else if (! strcmp (comando, "hablar"))
			hablar ();
		else if (! strcmp (comando, "terminar"))
			break;
		else
			printf ("Comando inválido.\n");
	}

	free (comando);

	free (basura);

	return 0;
}

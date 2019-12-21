#include "hilolay_internal.h"
#include "hilolay_alumnos.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int cantults = 0;

int servidor;

int cliente;

int suse_create (int tid) {
	char *tidcasteado = malloc (5);
	char *mensaje = malloc (7);
	char *aux = malloc (6);

	//printf ("Intentando crear un nuevo hilo.\n");

	if (cantults + 1 > MAX_ULTS) {
		printf ("Abortando creación de hilo, demasiados threads.\n");

		return ERROR_TOO_MANY_ULTS;
	}

	cantults++;

	//printf ("Ahora hay %i threads.\n", cantults);

	tidcasteado [0] = tid / 1000 + 48;

	tidcasteado [1] = tid / 100 - (tid / 1000) * 10 + 48;

	tidcasteado [2] = tid / 10 - (tid / 100) * 10 + 48;

	tidcasteado [3] = tid - (tid / 10) * 10 + 48;

	tidcasteado [4] = '\0';

	//printf ("El tid del nuevo thread es %s.\n", tidcasteado);

	sprintf (aux, "%c%s", 1, tidcasteado);

	free (tidcasteado);

	sprintf (mensaje, "%c%s", strlen (aux), aux);

	free (aux);

	send (cliente, mensaje, 7, 0);

	free (mensaje);

	return 0;
}

int suse_schedule_next () {
	char *mensaje = malloc (3);
	int tid;

	//printf ("Planificando el siguiente hilo.\n");

	mensaje [0] = 2;

	mensaje [1] = 2;

	mensaje [2] = '\0';

	send (cliente, mensaje, 3, 0);

	free (mensaje);

	mensaje = malloc (5);

	recv (cliente, mensaje, 5, 0);

	tid = (mensaje [0] - 48) * 1000 + (mensaje [1] - 48) * 100 + (mensaje [2] - 48) * 10 + mensaje [3] - 48;

	//printf ("El siguiente hilo es el %i.\n", tid);

	free (mensaje);

	return tid;
}

int suse_join (int tid) {
	char *tidcasteado = malloc (5);
	char *mensaje = malloc (7);
	char *aux = malloc (6);

	//printf ("Joineando al hilo %i.\n", tid);

	tidcasteado [0] = tid / 1000 + 48;

	tidcasteado [1] = tid / 100 - (tid / 1000) * 10 + 48;

	tidcasteado [2] = tid / 10 - (tid / 100) * 10 + 48;

	tidcasteado [3] = tid - (tid / 10) * 10 + 48;

	tidcasteado [4] = '\0';

	sprintf (aux, "%c%s", 3, tidcasteado);

	free (tidcasteado);

	sprintf (mensaje, "%c%s", strlen (aux), aux);

	free (aux);

	send (cliente, mensaje, 7, 0);

	free (mensaje);

	return 0;
}

int suse_close (int tid) {
	char *tidcasteado = malloc (5);
	char *mensaje = malloc (7);
	char *aux = malloc (6);

	//printf ("Cerrando el hilo %i.\n", tid);

	tidcasteado [0] = tid / 1000 + 48;

	tidcasteado [1] = tid / 100 - (tid / 1000) * 10 + 48;

	tidcasteado [2] = tid / 10 - (tid / 100) * 10 + 48;

	tidcasteado [3] = tid - (tid / 10) * 10 + 48;

	tidcasteado [4] = '\0';

	sprintf (aux, "%c%s", 4, tidcasteado);

	free (tidcasteado);

	sprintf (mensaje, "%c%s", strlen (aux), aux);

	free (aux);

	send (cliente, mensaje, 6, 0);

	free (mensaje);

	cantults--;

	//printf ("Ahora hay %i hilos.\n", cantults);

	return 0;
}

int suse_wait (int tid, char *sem) {
	char *mensaje = malloc (strlen (sem) + 6);
	char *tidcasteado = malloc (5);

	//printf ("Pidiendo el semáforo %s.\n", sem);

	tidcasteado [0] = tid / 1000 + 48;

	tidcasteado [1] = tid / 100 - (tid / 1000) * 10 + 48;

	tidcasteado [2] = tid / 10 - (tid / 100) * 10 + 48;

	tidcasteado [3] = tid - (tid / 10) * 10 + 48;

	tidcasteado [4] = '\0';

	sprintf (mensaje, "%c%c%s%c%s", strlen (sem) + 6, 5, tidcasteado, strlen (sem), sem);

	free (tidcasteado);

	send (cliente, mensaje, strlen (sem) + 7, 0);

	free (mensaje);

	return 0;
}

int suse_signal (int tid, char *sem) {
	char *mensaje = malloc (strlen (sem) + 2);

	//printf ("Signaleando el semáforo %s.\n", sem);

	sprintf (mensaje, "%c%c%c%s", strlen (sem) + 2, 6, strlen (sem), sem);

	send (cliente, mensaje, strlen (sem) + 3, 0);

	free (mensaje);

	return 0;
}

static struct hilolay_operations hiloops = {
		.suse_create = &suse_create,
		.suse_schedule_next = &suse_schedule_next,
		.suse_join = &suse_join,
		.suse_close = &suse_close,
		.suse_wait = &suse_wait,
		.suse_signal = &suse_signal
};

void hilolay_init () {
	struct sockaddr_in direccionservidor;
	direccionservidor.sin_family = AF_INET;
	direccionservidor.sin_addr.s_addr = inet_addr ("127.0.0.1");
	direccionservidor.sin_port = htons (1024);

	//printf ("Creando la conexión con SUSE.\n");

	cliente = socket (AF_INET, SOCK_STREAM, 0);

	if ((servidor = connect (cliente, (void*) &direccionservidor, sizeof (direccionservidor))) != 0) {
		perror ("No me pude conectar.\n");

		printf ("Error en la conexión, matando proceso...\n");

		exit (EXIT_FAILURE);
	}

	//printf ("Conexión exitosa.\n");

	init_internal (&hiloops);
}

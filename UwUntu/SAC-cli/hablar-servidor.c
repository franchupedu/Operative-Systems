#include "hablar-servidor.h"
#include <errno.h>

void datos_server() {

	config = config_create("config-cli.config");
	char* ip_valor = config_get_string_value(config, "IP_SERVER");

	strcpy(ip_server, ip_valor);
	puerto_server = config_get_int_value(config, "PUERTO_SERVER");
	config_destroy(config);
}


char* pedir_oper_sacServer(const char* pedido_oper, int codigo_oper) {

	struct sockaddr_in direccionservidor;
	direccionservidor.sin_family = AF_INET;
	direccionservidor.sin_addr.s_addr = inet_addr(ip_server); //Cambiar por ip del servdor

	direccionservidor.sin_port = htons(puerto_server); //cambiar por puerto
	memset(&(direccionservidor.sin_zero), '\0', 8);
	int cliente = socket(AF_INET, SOCK_STREAM, 0);

	log_info(loggerINFO, "ip del servidor : %s\n", inet_ntoa(direccionservidor.sin_addr));

	int servidor;
	if ((servidor = connect(cliente, (void*) &direccionservidor,
			sizeof(direccionservidor))) != 0) {
		log_info(loggerERROR, "No me pude conectar.\n");
	}


	//Preparar mensaje para el servidor


	log_info(loggerINFO, "Conectado a SAC-SERVER **** pedido operación: %s\n", pedido_oper);

	int tamanio_msj;
	int msj_a_servidor;
	char *mensaje;
	int tam_path;
	int tam_buf;

	switch (codigo_oper) {
		// modo del nodo + tamaño + path
		case TRUNCAR:
			tamanio_msj = strlen (pedido_oper + sizeof (int) + sizeof (off_t)) + sizeof (int) + sizeof (off_t);
			msj_a_servidor = 2 * sizeof (int) + tamanio_msj;
			mensaje = malloc (msj_a_servidor + 1);
			memcpy (mensaje, &codigo_oper, sizeof (int));
			memcpy (mensaje + sizeof (int), &tamanio_msj, sizeof (int));
			memcpy (mensaje + 2 * sizeof (int), pedido_oper, tamanio_msj);
			mensaje [msj_a_servidor] = '\0';
			break;
		case LEER_ARCHIVO:
			memcpy (&tam_path, pedido_oper, sizeof (int));
			tamanio_msj = tam_path + sizeof (int) + sizeof (int) + sizeof (int);
			msj_a_servidor = 2 * sizeof (int) + tamanio_msj;
			mensaje = malloc (msj_a_servidor + 1);
			memcpy (mensaje, &codigo_oper, sizeof (int));
			memcpy (mensaje + sizeof (int), &tamanio_msj, sizeof (int));
			memcpy (mensaje + 2 * sizeof (int), pedido_oper, tamanio_msj);
			mensaje [msj_a_servidor] = '\0';
			break;
		case ESCRIBIR_ARCHIVO:

			memcpy(&tam_path, pedido_oper, sizeof(int));
			memcpy(&tam_buf, pedido_oper + sizeof(int) + tam_path, sizeof(int));


			tamanio_msj = tam_path + tam_buf + 3 * sizeof (int);

			msj_a_servidor = 2 * sizeof (int) + tamanio_msj;
			mensaje = malloc (msj_a_servidor + 1);
			memcpy (mensaje, &codigo_oper, sizeof (int));
			memcpy (mensaje + sizeof (int), &tamanio_msj, sizeof (int));
			memcpy (mensaje + 2 * sizeof (int), pedido_oper, tamanio_msj);
			mensaje [msj_a_servidor] = '\0';
			break;
		case CREAR_NODO:
			tamanio_msj = strlen(pedido_oper + 2 * sizeof (int)) + 2 * sizeof (int); //tam msj + tam cod
			msj_a_servidor = sizeof(int)*2 + tamanio_msj;
			mensaje = malloc(msj_a_servidor + 1);
			memcpy(mensaje, &codigo_oper, sizeof(int));
			memcpy(mensaje + sizeof(int), &tamanio_msj, sizeof(int));
			memcpy(mensaje + sizeof(int)*2, pedido_oper, tamanio_msj);
			mensaje[msj_a_servidor] = '\0';
			break;
		case RENOMBRAR:
			memcpy (&tam_buf, pedido_oper, sizeof (int));
			memcpy (&tam_path, pedido_oper + sizeof (int), sizeof (int));
			tamanio_msj = 2 * sizeof (int) + tam_buf + tam_path;
			msj_a_servidor = 2 * sizeof (int) + tamanio_msj;
			mensaje = malloc (msj_a_servidor + 1);
			memcpy (mensaje, &codigo_oper, sizeof (int));
			memcpy (mensaje + sizeof (int), &tamanio_msj, sizeof (int));
			memcpy (mensaje + 2 * sizeof (int), pedido_oper, tamanio_msj);
			mensaje [msj_a_servidor] = '\0';
			break;
		// cod op + tam msj + msj  -  Armo el mensaje para el servdor
		case DESCRIBIR:

		case ABRIR_DIRECTORIO:
		case ELIMINAR_DIRECTORIO:
		case LISTAR_DIRECTORIO:
		case ABRIR_ARCHIVO:
		case ELIMINAR_ARCHIVO:
					tamanio_msj = strlen(pedido_oper); //tam msj + tam cod
					msj_a_servidor = sizeof(int)*2 + tamanio_msj;
					mensaje = malloc(msj_a_servidor + 1);
					memcpy(mensaje, &codigo_oper, sizeof(int));
					memcpy(mensaje + sizeof(int), &tamanio_msj, sizeof(int));
					memcpy(mensaje + sizeof(int)*2, pedido_oper, tamanio_msj);
					mensaje[msj_a_servidor] = '\0';
	}

	send(cliente, mensaje, msj_a_servidor + 1, 0);

	//Recibir la respuesta del servidor

	free(mensaje);
	char* mensaje_server;
	int estado;
	int tam_respuesta;
	int tam_total;

	pthread_mutex_lock (&sockete);
	switch (codigo_oper) {



	case RENOMBRAR:
	case ELIMINAR_ARCHIVO:
	case TRUNCAR:
	case CREAR_NODO:
		mensaje_server =  malloc(sizeof(int));
		recv(cliente, mensaje_server, sizeof(int), MSG_WAITALL);
		break;
	//Abrir archivo, abrir direcotorio y escribir tienen el mismo case
	case ABRIR_ARCHIVO:
	case ABRIR_DIRECTORIO:
	case ESCRIBIR_ARCHIVO:
							// estado + nro de nodo
		mensaje_server =  malloc(2*sizeof(int));
		recv(cliente, mensaje_server, sizeof(int), MSG_WAITALL);
		memcpy(&estado, mensaje_server, sizeof(int));
		if (estado == 1)
			break;
		recv (cliente, mensaje_server + sizeof (int), sizeof (int), MSG_WAITALL);
		break;

	case LEER_ARCHIVO:	// int estado + int tamanio + char* contenido

						mensaje_server =  malloc(sizeof(int));

						if (recv(cliente, mensaje_server, sizeof(int), MSG_WAITALL) == -1) {
							tam_respuesta = 0;

							break;
						}
						memcpy(&tam_respuesta, mensaje_server, sizeof(int));
						if (tam_respuesta == -1 || tam_respuesta == 0)
							break;
						mensaje_server = realloc(mensaje_server, sizeof(int) + tam_respuesta + 1);


						int bytes_totales_recv = 0;
						int bytes_recv;
						bytes_recv = recv(cliente, mensaje_server + sizeof(int), tam_respuesta, MSG_WAITALL);

						bytes_totales_recv += bytes_recv;

						while (bytes_totales_recv < tam_respuesta && bytes_recv > 0) {

							bytes_recv += recv(cliente, mensaje_server + sizeof(int) + bytes_totales_recv, tam_respuesta - bytes_totales_recv, MSG_WAITALL);
							bytes_totales_recv += bytes_recv;
						}

						break;

	case DESCRIBIR:
						mensaje_server = malloc(2*sizeof(int) + 2*sizeof(uint64_t));
						recv(cliente, mensaje_server, sizeof(int), MSG_WAITALL);
						memcpy(&estado, mensaje_server, sizeof(int));
						if (estado == 1)
							break;
						recv(cliente, mensaje_server + sizeof(int), 2*sizeof(uint64_t) + sizeof(uint32_t), MSG_WAITALL);
						break;

	case LISTAR_DIRECTORIO:
						recv(cliente, &tam_total, sizeof(int), MSG_WAITALL);
						if (tam_total == -3) {
							mensaje_server = malloc (sizeof (int));

							memcpy (mensaje_server, &tam_total, sizeof (int));

							int mensajito = 1;

							send (cliente, &mensajito, sizeof (int), 0);

							pthread_mutex_unlock (&sockete);

							return mensaje_server;
						}
						mensaje_server = malloc(tam_total);
						recv(cliente, mensaje_server, tam_total, MSG_WAITALL);
						break;
	}
	int mensajito = 1;
	send (cliente, &mensajito, sizeof (int), 0);
	pthread_mutex_unlock (&sockete);

	close(cliente);
	//close(servidor);

	return mensaje_server;
}

char* sac_leer(const char* path) {

	char* buffer = pedir_oper_sacServer(path, LEER_ARCHIVO);
	return buffer;
}


char* sac_crear_nodo(const char* path) {

	char* buffer = pedir_oper_sacServer(path, CREAR_NODO);
	return buffer;
}


char* sac_abrir(const char* path) {

	char* buffer = pedir_oper_sacServer(path, ABRIR_ARCHIVO);
	return buffer;
}

char* sac_abrir_directorio(const char* path) {

	char* buffer = pedir_oper_sacServer(path, ABRIR_ARCHIVO);
	return buffer;
}

char* sac_escribir(const char* path) {

	char* buffer = pedir_oper_sacServer(path, ESCRIBIR_ARCHIVO);
	return buffer;
}

char* sac_describir(const char* path) {

	char* buffer = pedir_oper_sacServer(path, DESCRIBIR);
	return buffer;
}

char* sac_listar (const char* path) {

	char* buffer = pedir_oper_sacServer(path, LISTAR_DIRECTORIO);
	return buffer;
}

char *sac_truncar (const char *path) {
	return pedir_oper_sacServer (path, TRUNCAR);
}

char *sac_borrar_archivo (const char *path) {
	return pedir_oper_sacServer (path, ELIMINAR_ARCHIVO);
}

char *sac_borrar_directorio (const char *path) {
	return pedir_oper_sacServer (path, ELIMINAR_DIRECTORIO);
}

char *sac_renombrar (const char *path) {
	return pedir_oper_sacServer (path, RENOMBRAR);
}

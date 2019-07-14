#include "Planificador.h"

// Esta función envia las requests al proceso memoria y las procesa.
void insertarrequest () {
	int puerto = config_get_int_value (config, "Puerto");
	struct sockaddr_in direccionservidor;
	char *request = malloc (1);
	char *clave = malloc (20);
	char *ip = malloc (11);
	int tamaniorequest = 0;
	char caracter;
	int codigo;

	// Se inicializa el socket cliente.
	cliente = socket (AF_INET, SOCK_STREAM, 0);

	// Clave para testeos de la cátedra.
	clave = config_get_string_value (config, "Clave");

	// IP para las conexiones con distintas máquinas.
	ip = config_get_string_value (config, "IP");

	// Aca se instancia la dirección del servidor Memoria.
	direccionservidor.sin_family = AF_INET;

	direccionservidor.sin_addr.s_addr = inet_addr (ip);

	direccionservidor.sin_port = htons (puerto);

	if (connect (cliente, (void*) &direccionservidor, sizeof (direccionservidor))) {
		log_info (logger, "No me pude conectar.\n");

		return;
	}

	// Inicia con un pedido del usuario para ejecutar una request.
	printf ("Inserte una request: ");

	caracter = getchar ();

	// Este ciclo guarda la request en un stream que luego será enviado con un código de operación.
	while (caracter != '\n') {
		while (caracter != '\n') {
			request = realloc (request, tamaniorequest + 1);

			request [tamaniorequest] = caracter;

			tamaniorequest++;

			caracter = getchar ();
		}

		request = realloc (request, tamaniorequest + 1);

		request [tamaniorequest] = '\0';

		// La función que parsea la request obtiene el código de operación.
		codigo = parsear (request);

		// El switch identifica la operación a ejecutar con el código obtenido anteriormente.
		switch (codigo) {
			case 6: ejecutarselect (request, tamaniorequest); break;
			case 7: log_info (logger, "Formato: select [tabla] [key]\n"); break;
			case 12: ejecutarinsert (request, tamaniorequest); break;
			case 13: log_info (logger, "Formato: insert [tabla] [key] \"[valor]\" [timestamp]\n"); break;
			case 18: ejecutarcreate (request, tamaniorequest); break;
			case 19: log_info (logger, "Formato: create [tabla] [consistencia] [particiones] [tiempo de compactación]\n"); break;
			case 26: ejecutardescribe (request, tamaniorequest); break;
			case 27: log_info (logger, "Formato: describe / describe [tabla]\n"); break;
			case 29: ejecutardrop (request, tamaniorequest); break;
			case 30: log_info (logger, "Formato: drop [tabla]\n"); break;
			case 36: ejecutarjournal (); break;
			case 37: log_info (logger, "Formato: journal\n"); break;
			case 46: ejecutaradd (request); break;
			case 47: log_info (logger, "Formato: add memory [numero] to [consistencia]\n"); break;
			case 49: ejecutarrun (request); break;
			case 50: log_info (logger, "Formato: run [path]\n"); break;
			case 56: ejecutarmetrics (); break;
			case 57: log_info (logger, "Formato: metrics\n"); break;
			default: log_info (logger, "Comando inválido.\n"); break;
		}

		tamaniorequest = 0;

		printf ("Inserte una request: ");

		caracter = getchar ();
	}

	free (request);

	close (cliente);
}

// Ciclo de logueo de las métricas cada 30 segundos.
void ciclodemetrics() {
	// Se ejecutará cada 30 segundos.
	sleep (30);

	// Manda la orden de ejecutar las métrics.
	ejecutarmetrics ();

	// Vuelve a hacerlo.
	ciclodemetrics ();
}

// Ciclo de ejecución de los scripts que se encuentren en ready.
void ejecutarready () {
	// Ejecuta el primero en la cola de ready.
	ejecutararchivo (*comienzoready);

	// Ejecuta el siguiente en la cola de ready.
	ejecutarready ();
}

// Se solicitan las memorias que componen el pool para la ejecución de requests.
void solicitarmemorias () {
	int tamaniodireccion = sizeof (struct sockaddr_in);
	struct sockaddr_in direccionservidor;
	char *mensaje = malloc (1);
	int tamaniomensaje;
	int activado = 1;

	configmemorias = config_create ("Memorias.config");

	cliente = socket (AF_INET, SOCK_STREAM, 0);

	// Se instancia la dirección de memoria como cliente.
	direccionservidor.sin_family = AF_INET;

	direccionservidor.sin_addr.s_addr = inet_addr (config_get_string_value (config, "IP"));

	direccionservidor.sin_port = htons (config_get_int_value (config, "Puerto"));

	// Se setea la dirección del puerto por si está en uso.
	setsockopt (servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof (activado));

	// Se enlaza la dirección del servidor con el puerto.
	if ((servidor = connect (cliente, (void*) &direccionservidor, sizeof (direccionservidor))) != 0) {
		log_info (logger, "No me pude conectar.\n");

		return;
	}

	// Se recibe el tamaño de la cantidad de memorias.
	if (recv (cliente, mensaje, 1, 0) <= 0) {
		log_info (logger, "Ocurrió un error.\n");

		close (cliente);

		return;
	}

	// Se guarda en una variable.
	tamaniomensaje = *mensaje;

	// Y se realoca la memoria necesaria para el mensaje.
	mensaje = realloc (mensaje, tamaniomensaje);

	// Por último se recibe el mensaje.
	if (recv (cliente, mensaje, tamaniomensaje, 0) <= 0) {
		log_info (logger, "Ocurrió un error.\n");

		close (cliente);

		return;
	}

	mensaje [tamaniomensaje] = '\0';

	// Y se setea el valor obtenido en el archivo de configuración.
	config_set_value (configmemorias, "Memorias", mensaje);

	free (mensaje);
}

// Función de configuración de quantum.
void configurarquantum () {
	char *quantum = malloc (2);
	char basura;

	printf ("Inserte el nuevo quantum: ");

	// Se obtiene el nuevo quantum.
	quantum [0] = getchar ();

	quantum [1] = '\0';

	basura = getchar ();

	// Se setea el nuevo valor.
	config_set_value (config, "Quantum", quantum);

	// Se guarda el archivo.
	config_save (config);
}

// Operación principal que inicializa el tiempo de inicio del programa para su posterior uso en las métricas, el ciclo de ejecución de la cola de ready, el de las métricas y llama a la función que inicializa el proceso de insertar una request.
void main () {
	char basura;
	char opcion;
	// TODO crear hilos y sincronizar.
	// TODO logs.

	// Se inicializa la varable del log.
	logger = log_create("Log.log", "Planificador.c", 1, LOG_LEVEL_INFO);

	// Y la variable del config.
	config = config_create ("Planificador.config");

	// Tiempo que despues se va a utilizar como referencia para conocer el tiempo trascurrido desde el inicio del programa.
	tiempodeinicio = time (NULL);

	// Primero que nada se solicitan las memorias que componen el pool.
	//solicitarmemorias ();

	// Ciclo de ejecución de la cola de ready.
	//ejecutarready ();

	// Ciclo de ejecución de las métricas.
	//ciclodemetrics ();

	// Menú de inicio.
	while (opcion != '\n') {
		printf ("1) Configurar quantum.\n2) Insertar request.\nInserte una opción: ");

		opcion = getchar ();

		basura = getchar ();

		switch (opcion) {
			case '1': configurarquantum (); break;
			case '2': insertarrequest (); break;
			default: break;
		}
	}
}

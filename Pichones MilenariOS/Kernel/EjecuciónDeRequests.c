#include "EjecuciónDeRequests.h"

// Ejecución de la request select.
void ejecutarselect (char *request, int tamaniorequest) {
	char *mensaje = malloc (tamaniorequest + 2);

	// El código se concatena como primer caracter del mensaje y el tamaño como segundo.
	mensaje [0] = aceptacionrequest [0];

	mensaje [1] = tamaniorequest;

	mensaje [2] = '\0';

	strcat (mensaje, request);

	// Se envía al proceso memoria.
	send (cliente, mensaje, tamaniorequest + 2, 0);

	free (mensaje);

	// Y se registra para las métrics.
	cantselect++;
}

// Ejecución de la request insert.
void ejecutarinsert (char *request, int tamaniorequest) {
	char *mensaje = malloc (tamaniorequest + 2);

	// El código se concatena como primer caracter del mensaje y el tamaño como segundo.
	mensaje [0] = aceptacionrequest [1];

	mensaje [1] = tamaniorequest;

	mensaje [2] = '\0';

	strcat (mensaje, request);

	// Se envía al proceso memoria.
	send (cliente, mensaje, tamaniorequest + 2, 0);

	free (mensaje);

	// Y se registra para las métrics.
	cantinsert++;
}

// Ejecución de la request create.
void ejecutarcreate (char *request, int tamaniorequest) {
	char *mensaje = malloc (tamaniorequest + 2);

	// El código se concatena como primer caracter del mensaje y el tamaño como segundo.
	mensaje [0] = aceptacionrequest [2];

	mensaje [1] = tamaniorequest;

	mensaje [2] = '\0';

	strcat (mensaje, request);

	free (mensaje);

	// Se envía al proceso memoria.
	send (cliente, mensaje, tamaniorequest + 2, 0);
}

// Ejecución de la request describe.
void ejecutardescribe (char *request, int tamaniorequest) {
	char *mensaje = malloc (tamaniorequest + 2);

	// El código se concatena como primer caracter del mensaje y el tamaño como segundo.
	mensaje [0] = aceptacionrequest [3];

	mensaje [1] = tamaniorequest;

	mensaje [2] = '\0';

	strcat (mensaje, request);

	free (mensaje);

	// Se envía al proceso memoria.
	send (cliente, mensaje, tamaniorequest + 2, 0);
}

// Ejecución de la request drop.
void ejecutardrop (char *request, int tamaniorequest) {
	char *mensaje = malloc (tamaniorequest + 2);

	// El código se concatena como primer caracter del mensaje y el tamaño como segundo.
	mensaje [0] = aceptacionrequest [4];

	mensaje [1] = tamaniorequest;

	mensaje [2] = '\0';

	strcat (mensaje, request);

	free (mensaje);

	// Se envía al proceso memoria.
	send (cliente, mensaje, tamaniorequest + 2, 0);
}

// Ejecución de la request journal.
void ejecutarjournal () {
	struct sockaddr_in direccionmemoria;
	char *codigo = malloc (1);
	struct Memoria *receptor;

	*codigo = aceptacionrequest [5];

	direccionmemoria.sin_family = AF_INET;

	// Se envía el código de la operación journal a la memoria asociada en la consistencia fuerte.
	if (consistenciafuerte.id) {
		direccionmemoria.sin_addr.s_addr = inet_addr (consistenciafuerte.ip);

		direccionmemoria.sin_port = htons (consistenciafuerte.puerto);

		if (connect (cliente, (void*) &direccionmemoria, sizeof (direccionmemoria)))
			log_info (logger, "No me pude conectar a la memoria %i.\n", consistenciafuerte.id);

		else
			send (cliente, codigo, 1, 0);
	}

	receptor = consistenciahashfuerte;

	// Se manda el código de operación a todas las memorias asociadas con la consistencia hash fuerte.
	while (receptor) {
		direccionmemoria.sin_addr.s_addr = inet_addr (receptor -> ip);

		direccionmemoria.sin_port = htons (receptor -> puerto);

		if (connect (cliente, (void*) &direccionmemoria, sizeof (direccionmemoria)))
			log_info (logger, "No me pude conectar a la memoria %i.\n", receptor -> id);

		else
			send (cliente, codigo, 1, 0);

		receptor = receptor -> siguiente;
	}

	receptor = consistenciaeventual;

	// Se manda el código de operación a todas las memorias asociadas con la consistencia eventual.
	while (receptor) {
		direccionmemoria.sin_addr.s_addr = inet_addr (receptor -> ip);

		direccionmemoria.sin_port = htons (receptor -> puerto);

		if (connect (cliente, (void*) &direccionmemoria, sizeof (direccionmemoria)))
			log_info (logger, "No me pude conectar a la memoria %i.\n", receptor -> id);

		else
			send (cliente, codigo, 1, 0);

		receptor = receptor -> siguiente;
	}

	// Por último se envia el código de operación a la memoria principal del proceso memoria.
	direccionmemoria.sin_addr.s_addr = inet_addr (config_get_string_value (config, "IP"));

	direccionmemoria.sin_port = htons (config_get_int_value (config, "Puerto"));

	send (cliente, codigo, 1, 0);

	free (codigo);
}

// Función que devuelve el número base elevado al número exponente.
int potencia (int base, int exponente) {
	if (! exponente)
		return 1;

	for (int i = 1; i < exponente; i++)
		base *= base;

	return base;
}

// Función que castea un stream de números a un int.
int castear (char *numero) {
	int numerocasteado = 0;

	for (int i = 0; i < strlen (numero); i++)
		numerocasteado += (numero [i] - 48) * potencia (10, strlen (numero) - i - 1);

	return numerocasteado;
}

// Ejecución de la request add.
void ejecutaradd (char *request) {
	char *numero = malloc (1);
	struct Memoria memoria;
	int numerocasteado;
	int actual = 0;
	int i = 11;
	int j = 0;

	// Este ciclo toma los caracteres que serán el número de id de la memoria.
	while (request [i] != ' ') {
		numero = realloc (numero, i - 10);

		numero [i - 11] = request [i];

		i++;
	}

	// Casteamos el número ya que lo queremos en tipo int.
	numerocasteado = castear (numero);

	i += 4;

	// Nos fijamos a que tipo de consistencia lo queremos agregar utilizando la tabla de transiciones de los criterios de consistencia.
	while (request [i]) {
		while (request [i] != caracteresconsistencia [j] && j < 4)
			j++;

		// Acá se usa la tabla de criterios.
		actual = transicionesconsistencia [actual] [j];

		i++;
	}

	// Y este switch agrega la memoria al criterio que corresponda dependiendo del estado de aceptación que quedo en la tabla.
	switch (actual) {
		case 2: memoria.id = numerocasteado; consistenciafuerte = memoria; break;
		case 4: memoria.id = numerocasteado; memoria.siguiente = consistenciahashfuerte; *consistenciahashfuerte = memoria; break;
		case 6: memoria.id = numerocasteado; memoria.siguiente = consistenciaeventual; *consistenciaeventual = memoria; break;
		default: break;
	}
}

// Función que borra una pcb si termina todas sus requests o se cancela su ejecución.
void sacarproceso (struct PCB pcb) {
	// Se actualiza el comiezo de la cola de ready.
	comienzoready = pcb.siguiente;

	// Se libera la memoria alocada para el path de su script.
	free (pcb.path);
}

// Función que recibe un nuevo proceso para encolarlo o recibirlo de nuevo por fin de quantum, de todas formas va a ir al final de la cola.
void enviaraready (struct PCB pcb) {
	// Caso especial si la cola esta vacía.
	if (! finready) {
		// El proceso va al comienzo de la cola porque es el primero.
		comienzoready = &pcb;

		// El siguiente ahora sera el último, que es NULL en este caso porque es el primero.
		pcb.siguiente = finready;

		// Y como es el único en la cola será también en último.
		finready = &pcb;
	}

	// Si no es el único en la cola.
	else {
		// Se encolará atras del último.
		finready -> siguiente = &pcb;

		// No hay nadie atras del último.
		pcb.siguiente = NULL;

		// Y ahora el último pasa a ser él.
		finready = &pcb;
	}
}

// Función que decide que request ejecutar.
void ejecutarrequest (char *request) {
	// La función que parsea la request obtiene el código de operación.
	int codigo = parsear (request);

	// El switch identifica la operación a ejecutar con el código obtenido anteriormente.
	switch (codigo) {
		case 6: ejecutarselect (request, strlen (request)); break;
		case 7: log_info (logger, "Formato: select [tabla] [key]\n"); break;
		case 12: ejecutarinsert (request, strlen (request)); break;
		case 13: log_info (logger, "Formato: insert [tabla] [key] \"[valor]\" [timestamp]\n"); break;
		case 18: ejecutarcreate (request, strlen (request)); break;
		case 19: log_info (logger, "Formato: create [tabla] [consistencia] [particiones] [tiempo de compactación]\n"); break;
		case 26: ejecutardescribe (request, strlen (request)); break;
		case 27: log_info (logger, "Formato: describe / describe [tabla]\n"); break;
		case 29: ejecutardrop (request, strlen (request)); break;
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
}

// Función que manda a parsear cada línea del script y la ejecuta.
void ejecutararchivo (struct PCB pcb) {
	char *linea = malloc (1);
	FILE *archivo;
	char caracter;
	int i = 0;
	int q = 0;

	// Se abre el archivo con el path del script en la pcb solo para lectura.
	archivo = fopen (pcb.path, "rb");

	if (! archivo) {
		log_info (logger, "El archivo no existe.");

		return;
	}

	// Se pocisiona en la última request a ejecutar, la primera si es un script virgen.
	fseek (archivo, pcb.caracteractual, 0);

	// Se comienza a leer la siguiente línea
	fread (&caracter, 1, 1, archivo);

	// Y se actualiza la posición de la última lectura del script rn la pcb.
	pcb.caracteractual++;

	// Ciclo que lee el script hasta que se termine el archivo o el quantum.
	while (! feof (archivo) && q < config_get_int_value (config, "Quantum")) {
		// Ciclo que lee hasta que se termine la linea del script o el script or si es la última request.
		while (! feof (archivo) && caracter != '\n') {
			linea = realloc (linea, i + 1);

			linea [i] = caracter;

			i++;

			fread (&caracter, 1, 1, archivo);

			pcb.caracteractual++;
		}

		// Si se termina el script se borra su pcb.
		if (feof (archivo)) {
			sacarproceso (pcb);

			break;
		}

		// Si se termina por quantum.
		if (q >= config_get_int_value (config, "Quantum")) {
			// Se envía a ready de nuevo.
			enviaraready (pcb);

			// Se actualiza el primero en ready.
			comienzoready = comienzoready -> siguiente;

			break;
		}

		linea [i] = '\0';

		ejecutarrequest (linea);

		i = 0;

		// Actualizo el quantum.
		q++;

		fread (&caracter, 1, 1, archivo);
	}

	free (linea);

	fclose (archivo);
}

// Ejecución de la request run.
void ejecutarrun (char *request) {
	char *path = malloc (1);
	struct PCB pcb;
	int i = 4;

	// Al crear la pcb del script se inicia por la request 0, obviamente, pero se tiene que ir acutualizando ya que si ese script es desalojado por fin de quantum, no iniciará con la request 0 cuando vuelva, iniciará con la request que se quedo.
	pcb.caracteractual = 0;

	// Este ciclo saca el path de la request para meterlo en la pcb.
	while (request [i]) {
		path = realloc (path, i - 3);

		path [i - 4] = request [i];

		i++;
	}

	// Se aloca la memoria necesaria para guardar el path en la pcb.
	pcb.path =  malloc (strlen (path));

	// Se guarda el path del script en la pcb.
	pcb.path = path;

	// Y se libera la memoria de esta variable auxiliar.
	free (path);

	// Por último se envia el proceso a la cola de ready.
	enviaraready (pcb);
}

// Ejecución de la request métrics.
void ejecutarmetrics () {

}

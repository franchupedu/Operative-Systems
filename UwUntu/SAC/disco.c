#include "estructuras-fs.h"
#include "disco.h"


size_t largo_archivo(char* nombre_archivo) {
	size_t largo;
	FILE* archivo = fopen(nombre_archivo, "r");
	fseek(archivo, 0, SEEK_END);
	largo = ftell(archivo);
	fseek(archivo, 0, SEEK_SET);

	return largo;
}


void iniciar_logs_disco() {
	loggerINFO_DISK = log_create("logs-serverINFO_DISK.log", "SAC-SERVER", 1,
			LOG_LEVEL_INFO);
	loggerERROR_DISK = log_create("logs-serverERROR_DISK.log", "SAC-SERVER", 1,
			LOG_LEVEL_ERROR);
}

void leer_config_disco() {

	t_config* config_disco = config_create("config-server.config");
	if (config_disco == NULL)
		log_info(loggerERROR_DISK, "Error al leer el archivo de configuración\n");
	nombre_disco = strdup(
			config_get_string_value(config_disco, "NOMBRE_DISCO"));
	bs = config_get_int_value(config_disco, "BS");
	count = config_get_int_value(config_disco, "COUNT");
	config_destroy(config_disco);
}

void crear_disco() {
	leer_config_disco();
	char* comando = string_from_format(
			"dd if=/dev/urandom iflag=fullblock of=%s bs=%i count=%i",
			nombre_disco, bs, count);
	printf("\nEjecutando el comando: %s\n\n", comando);
	printf("\nEsto puede demorar un poco...\n");
	if (system(comando) == -1) {
		log_info(loggerERROR_DISK, "No fue posible ejecutar el comando de creación del disco\n"); //si copio de dev/random no anda
		exit(-1);
	}
	if (largo_archivo(nombre_disco) != bs * count)
		log_info(loggerERROR_DISK, "Advertencia: El tamaño del disco es distinto al pedido\n");
	else
		log_info(loggerINFO_DISK, "El tamaño del disco es el solicitado\n");
	free(comando);
}

char* comando_formatear() {
	return string_from_format("./sac-format %s", nombre_disco);
}

char* comando_dump() {
	return string_from_format("./sac-dump %s", nombre_disco);
}

void formatear() {

	char* comando_formato = comando_formatear();
	int disco = open(nombre_disco, O_RDWR, 0);
	if (disco == -1 && errno == ENOENT) {
		log_info(loggerINFO_DISK, "El disco aún no existe. Creando\n");
		crear_disco();
	}
	if (system(comando_formato) == -1)
		printf("No se pudo formatear el disco\n");
	log_info(loggerINFO_DISK, "Disco formateado exitosamente\n");
	free(comando_formato);
}

void hacer_dump() {
	char* comando_dumpeo = comando_dump();
	printf("\n**** Mostrando el bitmap ****\n");
	if (system(comando_dumpeo) == -1)
		log_info(loggerERROR_DISK, "No se pudo hacer el dump del disco\n");
	free(comando_dumpeo);
}

void mostrar_nodo(Gfile nodo) {

	switch (nodo.estado) {
	case BORRADO:
		printf("\nEstado: Borrado");
		break;
	case DIRECTORIO:
		printf("\nEstado: Directorio");
		break;
	case OCUPADO:
		printf("\nEstado: Ocupado");
		break;
	}

	printf("\nNombre del archivo: %s\nBloque padre: %i\n"
			"Tamaño del archivo: %i\nFecha de creación: %llu\nFecha de "
			"modificación: %llu\n\n", nodo.nombre_archivo, nodo.bloque_padre,
			nodo.tam_archivo, nodo.fecha_creacion, nodo.fecha_modificacion);
}

Gfile crear_nodo(uint8_t estado, char* nombre_archivo, ptrGBloque bloque_padre,
		uint32_t tam_archivo) {
	Gfile nodo;
	nodo.estado = estado;
	char* nombre_max = string_substring_until(nombre_archivo,
			MAX_FILE_NAME_LEN - 1);
	strcpy(nodo.nombre_archivo, nombre_max);
	nodo.bloque_padre = bloque_padre;
	nodo.tam_archivo = tam_archivo;
	nodo.fecha_creacion = time(NULL);
	nodo.fecha_modificacion = time(NULL);

	for (int i = 0; i < CANT_INDIR_SIMPLE; i++) {
		nodo.simple_indirect[i] = 0;
	}

	free(nombre_max);

	return nodo;
}


int cant_bloques_necesaria(int tamanio_bytes) {
	if (tamanio_bytes % BLOCK_SIZE == 0)
		return tamanio_bytes / BLOCK_SIZE;
	else
		return tamanio_bytes / BLOCK_SIZE + 1;
}

typedef struct  {
	int indice_ptr_ind;
	int ind_ptr_dir;
	int offset_bloque_datos;
} t_datos_comienzo_lectura;


t_datos_comienzo_lectura indices_comienzo_operacion(int offset) {

	t_datos_comienzo_lectura indices_lectura;
	indices_lectura.indice_ptr_ind = offset / (CANT_DIRECTOS * BLOCK_SIZE);
	int resto = offset % (CANT_DIRECTOS * BLOCK_SIZE);
	indices_lectura.ind_ptr_dir = resto / BLOCK_SIZE;
	indices_lectura.offset_bloque_datos = resto % BLOCK_SIZE;


	return indices_lectura;
}


int buscar_nodo_libre(Gfile* tabla_nodos) {
	for (int i = 0; i < MAX_FILE_COUNT; i++)
		if (tabla_nodos[i].estado == BORRADO)
			return i;
	return -1;
}


int cant_archivos_en_path(char** lista_path) {
	int i = 0;
	while (lista_path[i] != NULL)
		i++;
	return i;
}


void free_string_split(char** lista_de_palabras) {
	int largo_lista = cant_archivos_en_path(lista_de_palabras);
	for (int i = 0; i < largo_lista - 1; i++) {
		free(lista_de_palabras[i]);
	}
	free(lista_de_palabras);
}

/*
 * Dado un path a un archivo o directorio, valida si el path hasta su padre existe. Útil para crear nuevos elementos
 * */
char* path_del_padre(char* path) {
	char** archivos_en_path = string_split(path, "/");
	int cant_archivos = cant_archivos_en_path(archivos_en_path);

	if (cant_archivos == 1) { //Significa que quiero crear un archivo en la raíz
		return "/";
	}
	char* elemento_nuevo = archivos_en_path[cant_archivos - 1];
	int len_path_padre = strlen(path) - strlen(elemento_nuevo);
	char* path_del_padre = string_substring_until(path, len_path_padre - 1);
	free_string_split(archivos_en_path);

	return path_del_padre;
}


bool path_valido(char* path) {
	return string_starts_with(path, "/");
}


char* nombre_nuevo_archivo(char* path) {

	char** archivos_en_path = string_split(path, "/");
	int cant_archivos = cant_archivos_en_path(archivos_en_path);

	if (cant_archivos == 1 && path_valido(path)) {
		return string_substring_from(path, 1);
	}
	free_string_split(archivos_en_path);
	return archivos_en_path[cant_archivos - 1];
}

/*Dado un puntero a nodo, calculo su posición en la tabla de nodos y devuelvo el índice*/
int indice_de_nodo(Gfile* nodo) {
	ptrdiff_t distancia_bytes = ((char*) nodo) - ((char*) tabla_nodos);

	if (distancia_bytes % BLOCK_SIZE == 0)
		return distancia_bytes / BLOCK_SIZE;
	else
		return -1;
}

/*
 * buscar() guarda todos los nodos que tienen el mismo nombre en
 *  lista_hijos, junto con el índicce del nodo dentro de la
 *  tabla de nodos */
int buscar(char* nombre, Gfile* tabla_nodos, t_list* lista_indices) {

	t_metadata_archivo* datos_archivo;
	//datos_archivo = malloc(sizeof(t_metadata_archivo));
	for (int i = 0; i < MAX_FILE_COUNT; i++) {
		if (strcmp(tabla_nodos[i].nombre_archivo, nombre) == 0) {
			datos_archivo = malloc(sizeof(t_metadata_archivo));
			datos_archivo->indice = i;
			datos_archivo->nodo = tabla_nodos[i];
			list_add(lista_indices, datos_archivo);
			//datos_archivo = malloc(sizeof(t_metadata_archivo));
		}
	}

	//free(datos_archivo);

	if (list_is_empty(lista_indices))
		return -1;
	else
		return 1;
}


Gfile* extraer_nodo(int indice, Gfile* tabla_nodos) {
	if (indice < MAX_FILE_COUNT && indice > 0)
		return &tabla_nodos[indice];
	else
		return NULL;
}


bool tiene_ese_nombre(t_metadata_archivo* datos_archivo, char* nombre) {
	if (strcmp((datos_archivo->nodo).nombre_archivo, nombre) == 0)
		return true;
	return false;
}


/*Busca los padre de archivos con el mismo nombre. Devuelve un puntero al nodo del padre
 * y agrega el nodo del hijo al lista_indices
 */
Gfile* buscar_archivo(char* path, Gfile* tabla_nodos) { //ERROR: Si el path es un solo archivo, anda mal


	int bloque_padre;
	t_list* lista_hijos = list_create();
	t_list* lista_posibles_padres = list_create();
	t_list* lista_padres_comprobados = list_create();
	//t_list* lista_no_eran_padres = list_create();

	void* datos_archivo_hijo;
	t_metadata_archivo* datos_posible_padre;// = malloc(
			//sizeof(t_metadata_archivo));
	Gfile* nodo_posible_padre;
	int indice_archivo_encontrado;

	if (strcmp(path, "/") == 0) {
		return tabla_nodos;
	}

	if (path_valido(path) == false) {

		log_info(loggerERROR_DISK, "No se pudo buscar el archivo. Path inválido\n");
		return NULL;
	}

	char** archivos_en_path = string_split(path, "/");
	int cant_archivos = cant_archivos_en_path(archivos_en_path);
	int a_revisar = cant_archivos - 1;
	char* archivo = archivos_en_path[a_revisar]; //ultimo archivo en el path, el que quiero ubicar
	char* archivo_buscado = archivos_en_path[a_revisar];
	char* padre_archivo;

	//paso 1) Busco todos los nodos con ese nombre de hijo
	int hijos_encontrados = buscar(archivo, tabla_nodos, lista_hijos);

	if (hijos_encontrados == -1) {
		//printf("No existe un elemento con el nombre \"%s\"\n", archivo);
		return NULL;
	}

	//paso 2) Busco los padres de esos nodos por número de bloque
	if (hijos_encontrados > 0 && cant_archivos > 1) {

		for (int k = 1; k <= a_revisar; k++) {

			padre_archivo = archivos_en_path[a_revisar - k];

			for (int i = 0; i < list_size(lista_hijos); i++) {
				datos_posible_padre = malloc(sizeof(t_metadata_archivo));
				datos_archivo_hijo = list_get(lista_hijos, i);
				bloque_padre =
						(((t_metadata_archivo*) datos_archivo_hijo)->nodo).bloque_padre;

				nodo_posible_padre = extraer_nodo(bloque_padre, tabla_nodos);
				datos_posible_padre->indice = bloque_padre;

				if (! nodo_posible_padre)
					return NULL;
				datos_posible_padre->nodo = *nodo_posible_padre;

				// si estoy tratando con el último archivo del path, guardo su índice con los datos del padre
				if (k == 1)
					datos_posible_padre->hijo =
							((t_metadata_archivo*) datos_archivo_hijo)->indice;
				else
					datos_posible_padre->hijo =
							((t_metadata_archivo*) datos_archivo_hijo)->hijo;
				//sino, hago que todos los "familiares" guarden su índice
				list_add(lista_posibles_padres, datos_posible_padre);
				//datos_posible_padre = malloc(sizeof(t_metadata_archivo));



			}
			//paso 3) Filtrar la lista de posibles padres comparando con el nombre del padre
			bool _tiene_ese_nombre(void* elemento) {
				return tiene_ese_nombre((t_metadata_archivo*) elemento,
						padre_archivo);
			}


			bool not_tiene_ese_nombre(void* elemento) {
							return !tiene_ese_nombre((t_metadata_archivo*) elemento,
									padre_archivo);
						}

			lista_padres_comprobados = list_filter(lista_posibles_padres,
					_tiene_ese_nombre);
			//lista_no_eran_padres = list_filter(lista_posibles_padres,
					//not_tiene_ese_nombre);
			list_clean_and_destroy_elements(lista_posibles_padres, free);
			//void list_clean(t_list *);


			//list_clean_and_destroy_elements(lista_posibles_padres, free);
			if (list_size(lista_padres_comprobados) > 0) {
				//list_destroy_and_destroy_elements(lista_hijos, free);
				lista_hijos = lista_padres_comprobados;
				archivo = padre_archivo;
			} else {

				log_info(loggerINFO_DISK, "No existe la carpeta con el nombre \"%s\"\n",
						padre_archivo);
				//free(datos_posible_padre); //libero el último que reservé
				//datos_posible_padre = NULL;
				free_string_split(archivos_en_path);
				list_destroy_and_destroy_elements(lista_hijos, free);
				list_destroy_and_destroy_elements(lista_padres_comprobados,
						free);
				list_destroy_and_destroy_elements(lista_posibles_padres, free);
				return NULL;
			}
		}


	}

	else if (hijos_encontrados) { //si el path está formado solo por un archivo
		void* raiz = list_get(lista_hijos, 0);
		int indice = ((t_metadata_archivo*) raiz)->indice;
		//free(datos_posible_padre);
		//datos_posible_padre = NULL;
		free_string_split(archivos_en_path);
		list_destroy_and_destroy_elements(lista_hijos, free);
		list_destroy_and_destroy_elements(lista_padres_comprobados,
				free);
		list_destroy_and_destroy_elements(lista_posibles_padres, free);

		return extraer_nodo(indice, tabla_nodos);
	}
	free_string_split(archivos_en_path);
	void* raiz;

	if (list_size(lista_hijos) == 1) {
		raiz = list_get(lista_hijos, 0);
		int padre = ((t_metadata_archivo*) raiz)->nodo.bloque_padre;

		if (padre != 0) {

			log_info(loggerINFO_DISK, "El path \"%s\" es inválido\n", path);
			//free(datos_posible_padre); //libero el último que reservé
			//datos_posible_padre = NULL;
			//list_destroy_and_destroy_elements(lista_padres_comprobados, free);
			list_destroy_and_destroy_elements(lista_posibles_padres, free);
			//list_destroy_and_destroy_elements(lista_hijos, free);

			return NULL;
		} else {
			//free(datos_posible_padre); //libero el último que reservé
			//datos_posible_padre = NULL;
			indice_archivo_encontrado = ((t_metadata_archivo*) raiz)->hijo;
			//list_destroy_and_destroy_elements(lista_hijos, free);
			//list_destroy_and_destroy_elements(lista_padres_comprobados, free);
			list_destroy_and_destroy_elements(lista_posibles_padres, free);

			return extraer_nodo(indice_archivo_encontrado, tabla_nodos);
		}
	}

	//free(datos_posible_padre); //libero el último que reservé
	//datos_posible_padre = NULL;
	//list_destroy_and_destroy_elements(lista_padres_comprobados, free);
	list_destroy_and_destroy_elements(lista_posibles_padres, free);
	//list_destroy_and_destroy_elements(lista_hijos, free);
	log_info(loggerINFO_DISK, "No se encontraron archivos con el nombre \"%s\"\n",
			archivo_buscado);
	return NULL;
}


int buscar_indice_nodo_por_path(char* path) {
	Gfile* nodo = buscar_archivo(path, tabla_nodos);
	if (nodo == NULL)
		return -1;
	return indice_de_nodo(nodo);
}


int crear_elemento(int estado, char* path, Gfile* tabla_nodos) {

	if (strcmp("/", path) == 0) { //No puedo reescribir el path raíz
		return -1;
	}

	Gfile* archivo_para_agregar = buscar_archivo(path, tabla_nodos);

	if (archivo_para_agregar == NULL) { //Si el archivo que quiero crear no existe
		char* path_padre = path_del_padre(path);
		Gfile* nodo_padre = buscar_archivo(path_padre, tabla_nodos);

		if (nodo_padre != NULL && nodo_padre->estado == DIRECTORIO) {

			int indice_nodo_padre = indice_de_nodo(nodo_padre);
			//if path == "/" padre  = 0
			char* nombre_archivo = nombre_nuevo_archivo(path);

			Gfile nodo_nuevo = crear_nodo(estado, nombre_archivo,
					indice_nodo_padre, 0);
			//Se asignan 0 bloques
			int indice_nodo = buscar_nodo_libre(tabla_nodos);
			if (indice_nodo == -1)
				return -1;
			tabla_nodos[indice_nodo] = nodo_nuevo;

			char* estado_f;
			if (estado == DIRECTORIO)
				estado_f = "Directorio";
			else
				estado_f = "Archivo";
			log_info(loggerINFO_DISK, " CREAR_ELEMENTO: Fue creado el %s \"%s\" en el path \"%s\"\n", estado_f,
					nombre_archivo, path_padre);
			return indice_nodo;
		}
		log_info(loggerINFO_DISK, " CREAR_ELEMENTO: El directorio indicado para crear el archivo no existe\n");
		return -1;
	}

	log_info(loggerINFO_DISK, " CREAR_ELEMENTO: El archivo ya existe en el File System\n");
	return -1;
}


void mostrar_metadata_archivo(void* datos_lista) {
	t_metadata_archivo* datos_archivo = (t_metadata_archivo*) datos_lista;

	mostrar_nodo(datos_archivo->nodo);
}


int buscar_bloque_libre(t_bitarray* bitmap) {
	for (int i = comienzo_bloques_datos; i < cant_bloques_totales; i++) {
		if (bitarray_test_bit(bitmap, i) == false)
			return i;
	}
	return -1;
}


Gbloque* extraer_dir_bloque(Gbloque* disco, int nro_bloque) {
	if (nro_bloque < cant_bloques_totales
			&& nro_bloque >= comienzo_bloques_datos) {
		Gbloque* dir_bloque = disco + nro_bloque;
		return dir_bloque;
	} else
		return NULL;
}


/*Paso por parámetro un índice del ptr indirecto (1000) en el que quiero agregar
 * un puntero a bloque directo (1024)
 * Esta operación agrega 1 bloque al bitmap, el otro se setea en "escribir"*/
int agregar_bloque_ptrDir(Gfile* nodo_archivo, int i) {


	/*Tengo que entrar al blqoue de punteros dir y revisar si hay espacio para más*/

	log_info(loggerINFO_DISK, "ESCRIBIR: Agregando nuevo bloque de punteros al archivo\n");

	nodo_archivo->simple_indirect[i] = buscar_bloque_libre(bitmap); //debería dar 1026

	if (nodo_archivo->simple_indirect[i] == -1) {
		log_info(loggerINFO_DISK, "ESCRIBIR: No hay bloques disponibles para agregar al archivo...\n");
		nodo_archivo->simple_indirect[i] = 0;
		return -1;
	} else {
		bitarray_set_bit(bitmap, nodo_archivo->simple_indirect[i]);

		Bloque_ptr* nuevo_bloq_dir = (Bloque_ptr*) extraer_dir_bloque(disco,
				nodo_archivo->simple_indirect[i]); //extraigo el bloque 1026


		//memset(nuevo_bloq_dir, 0, BLOCK_SIZE);
		for (int i = 0; i < CANT_DIRECTOS; i++) {
			nuevo_bloq_dir->ptr_direct[i] = 0;
		}


		nuevo_bloq_dir->ptr_direct[0] = buscar_bloque_libre(bitmap);


		if (nuevo_bloq_dir->ptr_direct[0] == -1 ) {
			log_info(loggerINFO_DISK, "ESCRIBIR: Se acabaron los bloques libres\n");
			return -1;
		} else {
			log_info(loggerINFO_DISK, "ESCRIBIR: Se agrego un bloque de punteros directos y se encontró un bloque libre para agregar datos\n");
			return nuevo_bloq_dir->ptr_direct[0];
		}
	}
}


t_list* listar_archivos_del_directorio(char* path) {

	Gfile* directorio = buscar_archivo(path, tabla_nodos);
	t_list* lista_archivos = list_create(); //Reservar_memoria
	int tam_lista_archivos = 0;
	int tam_info = 115;



	//lista_archivos = malloc(tam_info);
	//memset(lista_archivos, 0, tam_info);

	if (directorio != NULL && directorio->estado == DIRECTORIO) {
		int indice_nodo_dir = indice_de_nodo(directorio);

		for (int i = 0; i < MAX_FILE_COUNT; i++) {
			if (tabla_nodos[i].bloque_padre == indice_nodo_dir && tabla_nodos[i].estado != BORRADO && i != indice_nodo_dir) {

				tam_lista_archivos += tam_info;
				list_add(lista_archivos, &tabla_nodos[i]);
			}
		}

		log_info(loggerINFO_DISK, " LISTAR: Listando los archivos del directorio \"%s\"\n", path);
		return lista_archivos;
	}
	list_destroy (lista_archivos);
	log_info(loggerINFO_DISK, " LISTAR: El directorio no exste\n");
	return NULL;
}


/*Busca un bloque disponible para escribir y lo agrega a la lista de puntero*/
int bloque_indirecto_para_escribir(Gfile* nodo_archivo) {

	//quiero que devuelva el numero de bloque disponible para escribir
	//en el vector de ptr ind, revisa hasta encontrar el último puntero disinto de cero
	//entra a ese bloque y lee los ptr directos. Busca un bloque libre y lo agrega, si hay lugar
	//a la lista de punteros directos. Sino agrega otro indirecto (busca bloques libres), y a lo que apunte
	//le agrega un puntero a un bloque disponible para escribir.

	for (int i = 0; i < CANT_INDIR_SIMPLE; i++) {

		if (nodo_archivo->simple_indirect[i] == 0) {

			if (i > 0) {

				//if hay espacio en el bloque de ptr directos de i - 1, agreg un bloque de datos ahí
				Bloque_ptr* bloque_ptrDir = (Bloque_ptr*) extraer_dir_bloque(disco, nodo_archivo->simple_indirect[i - 1]);

				for (int j = 0; j < CANT_DIRECTOS; j++) { //si hay espacio para punteros disponible
					if (bloque_ptrDir->ptr_direct[j] == 0) {

						//todo Estoy agregando un nuevo bloque sin haber llenado el anterior
						/*  /(°o°)\ */

						int bloque_libre = buscar_bloque_libre(bitmap);

						if (bloque_libre != -1) { //si hay un bloque libre
							bloque_ptrDir->ptr_direct[j] = bloque_libre;
							log_info(loggerINFO_DISK, "ESCRIBIR: Se encontró un bloque libre y fue agregado al archivo, en el bloque de punteros %i\n",
									nodo_archivo->simple_indirect[i - 1]);
							return bloque_libre; //El bitmap lo seteo en escribir
						} else {
							log_info(loggerINFO_DISK, "ESCRIBIR: No hay bloques disponibles\n");
							return -1;
						}
					}
				}
				//Si salí del for es porque ya no hay espacio en el bloque de ptr dir.
				return agregar_bloque_ptrDir(nodo_archivo, i);
			}
			return agregar_bloque_ptrDir(nodo_archivo, i);
		}
	}
	log_info(loggerINFO_DISK, "ESCRIBIR: Se llenó la lista de punteros indirectos. El archvo llegó a su tamaño máximo\n");
	return -1;
}



int renombrar(char *path_viejo, char *path_nuevo) {

	Gfile* archivo = buscar_archivo(path_viejo, tabla_nodos);
	Gfile* padre_nuevo_archivo = buscar_archivo(path_del_padre(path_nuevo), tabla_nodos);

	if (archivo != NULL && padre_nuevo_archivo != NULL) {

			//Caso en el que solo quiero cambiar el nombre del archivo
			char** archivos_en_path = string_split(path_nuevo, "/");
			int cant_archivos = cant_archivos_en_path(archivos_en_path);

			char* nuevo_nombre = string_substring_until(
					archivos_en_path[cant_archivos - 1], MAX_FILE_NAME_LEN - 1);
//Recortar nombre a 70 caracteres. Puse  MAX_FILE_NAME_LEN - 1 porque creo 	que esta función no cuenta el \0 en len

			strcpy(archivo->nombre_archivo, nuevo_nombre);
			archivo->bloque_padre = indice_de_nodo(padre_nuevo_archivo);

			free(nuevo_nombre);
			free_string_split(archivos_en_path);

			return 0;
	}
	return -1;
}


typedef struct {
	char* contenido;
	int tam_contenido;
} contenido_escritura;

/*Permite agregar contenido al último bloque escrito, si aún no ha sido completado
 * Retorna resto del contenido que aún no ha sido escrito*/
contenido_escritura escribir_sobre_bloque_incompleto(Gfile* nodo_archivo, char* contenido, int tam_contenido) {

	contenido_escritura a_escribir;

	//Determina si el último bloque escrito fue llenado por completo o no a partit del tamaño actual del archivo
	t_datos_comienzo_lectura ult_escritura = indices_comienzo_operacion(nodo_archivo->tam_archivo);


	if (ult_escritura.offset_bloque_datos == 0) {

		a_escribir.contenido = contenido;
		a_escribir.tam_contenido = tam_contenido;

		return a_escribir;
	}

	Bloque_ptr* bloq_dir = (Bloque_ptr*) extraer_dir_bloque(disco, nodo_archivo->simple_indirect[ult_escritura.indice_ptr_ind]);

	Gbloque* bloque_espacio = extraer_dir_bloque(disco, bloq_dir->ptr_direct[ult_escritura.ind_ptr_dir]);
	int esp_libre = BLOCK_SIZE - ult_escritura.offset_bloque_datos;

	if (tam_contenido < esp_libre) {


		memset((void*) bloque_espacio + ult_escritura.offset_bloque_datos, 0, esp_libre);
		memcpy((void*) bloque_espacio + ult_escritura.offset_bloque_datos, contenido, tam_contenido);

		nodo_archivo->tam_archivo += tam_contenido;

		a_escribir.contenido = "";
		a_escribir.tam_contenido = 0;

		return a_escribir;
	}

	//contenido_parcial = malloc(esp_libre);
	//memcpy(contenido_parcial, contenido, esp_libre);
	//contenido_parcial[esp_libre - 1] = '\0';


	memcpy((void*) bloque_espacio + ult_escritura.offset_bloque_datos, contenido, esp_libre);


	nodo_archivo->tam_archivo += esp_libre;


	if (tam_contenido == 0) {//significa que mande cadena vacía en truncar
		a_escribir.contenido = contenido + esp_libre;
	}
	else {
		a_escribir.contenido = contenido + esp_libre;
	}
	a_escribir.tam_contenido = tam_contenido - esp_libre;



	return a_escribir;
}


int escribir_final_archivo(Gfile* nodo_archivo, char* contenido2, uint32_t tam_archivo) {


	int bloques_escritos = 0;
	int indice_bloque_libre;
	int bytes_a_copiar;
	int bytes_escritos = 0;
	Gbloque* bloque_libre;

	contenido_escritura a_escribir;



	if (tam_archivo == 0) {  //strlen(contenido2) == 0 ||
		return 0;
	}

	if (nodo_archivo != NULL && nodo_archivo->estado == OCUPADO) {

		a_escribir = escribir_sobre_bloque_incompleto(nodo_archivo, contenido2, tam_archivo);

		bytes_escritos += tam_archivo - a_escribir.tam_contenido;
		char* contenido = a_escribir.contenido;

		bytes_a_copiar = a_escribir.tam_contenido; //strlen(contenido);

		int bloques_necesarios = cant_bloques_necesaria(tam_archivo);

		while (bloques_escritos < bloques_necesarios && bytes_a_copiar > 0) {

			indice_bloque_libre = bloque_indirecto_para_escribir(nodo_archivo);

			if (indice_bloque_libre != -1 && nodo_archivo != NULL) {

				bitarray_set_bit(bitmap, indice_bloque_libre);

				msync(bitmap_en_disco, tamanio_bitmap_bytes, MS_SYNC);

				bloque_libre = extraer_dir_bloque(disco, indice_bloque_libre);

				if (bloque_libre != NULL) {

					if (bytes_a_copiar >= BLOCK_SIZE) {

						memcpy(bloque_libre, contenido, BLOCK_SIZE);
						nodo_archivo->tam_archivo += BLOCK_SIZE;
						bytes_escritos += BLOCK_SIZE;
						contenido += BLOCK_SIZE;
						bytes_a_copiar -= BLOCK_SIZE;
						bloques_escritos++;
						msync(disco, BLOCK_SIZE * bloques_escritos, MS_SYNC);

					} else {

						memset(bloque_libre, 0, BLOCK_SIZE);
						memcpy(bloque_libre, contenido, bytes_a_copiar);

						nodo_archivo->tam_archivo += bytes_a_copiar; //sin en \0


						bytes_escritos += bytes_a_copiar;
						bytes_a_copiar = 0;
						bloques_escritos++;
						msync(disco, BLOCK_SIZE * bloques_escritos, MS_SYNC);
					}
				}
			} else
				bytes_a_copiar = -1;
		}
		log_info(loggerINFO_DISK, "ESCRIBIR: Se han utilizado %i bloques\n", bloques_escritos);

		if (bloques_escritos > 0) {

			nodo_archivo->fecha_modificacion = time(NULL);
			if (bytes_a_copiar == -1) {
				log_info(loggerINFO_DISK, "ESCRIBIR: Se acabaron los bloques libres y fueron escritos %i bytes\n",
						bloques_escritos * BLOCK_SIZE);
				//free(contenido);
				return bloques_escritos * BLOCK_SIZE;//bloques_escritos * BLOCK_SIZE;
			}
			log_info(loggerINFO_DISK, "ESCRIBIR: Archivo escrito exitosamente. Fueron escritos %i bytes\n", bytes_escritos);
			//free(contenido);
			return bytes_escritos;//bytes_escritos;

		} else {

			if (a_escribir.tam_contenido == 0 && tam_archivo > 0) {
				log_info(loggerINFO_DISK, "ESCRIBIR: Se escribieron %i bytes dentro del último bloque del archivo\n", tam_archivo);
				//free(contenido);
				return tam_archivo;
			}

			log_info(loggerINFO_DISK, "ESCRIBIR: No se pudo escribir en el archivo\n");
			//free(contenido);
			return 0;
		}

	}
	log_info(loggerINFO_DISK, "ESCRIBIR: El archivo no existe o no es válido\n");
	return 0;
}



void escribir_dentro_archivo (Gfile* nodo_archivo, char* contenido2,  int cant_dentro_archivo, off_t offset) {

	t_datos_comienzo_lectura indices = indices_comienzo_operacion(offset);
	int cant_escrita = 0;
	Bloque_ptr* bloq_ptr_dir;
	Gbloque* bloq_datos;

	for (int i = indices.indice_ptr_ind; nodo_archivo->simple_indirect[i] != 0 && i < CANT_INDIR_SIMPLE && cant_escrita < cant_dentro_archivo; i++) {

		bloq_ptr_dir = (Bloque_ptr*) extraer_dir_bloque(disco, nodo_archivo->simple_indirect[i]);

		for (int j = i == indices.indice_ptr_ind ? indices.ind_ptr_dir : 0; bloq_ptr_dir->ptr_direct[j] != 0 && j < CANT_DIRECTOS; j++) {

			bloq_datos = extraer_dir_bloque(disco, bloq_ptr_dir->ptr_direct[j]);

			if (j == indices.ind_ptr_dir) {

				if (cant_dentro_archivo > BLOCK_SIZE - indices.offset_bloque_datos) {
					memcpy( (char*) bloq_datos + indices.offset_bloque_datos, contenido2, BLOCK_SIZE - indices.offset_bloque_datos);
					cant_escrita += BLOCK_SIZE - indices.offset_bloque_datos;
				}
				else {

					memcpy( (char*) bloq_datos + indices.offset_bloque_datos, contenido2, cant_dentro_archivo);
					i = CANT_INDIR_SIMPLE;
					break;
				}
			}

			else {
				if (cant_dentro_archivo - cant_escrita > BLOCK_SIZE) {
				memcpy(bloq_datos, contenido2 + cant_escrita, BLOCK_SIZE);
				cant_escrita += BLOCK_SIZE;
				}
				else {
				memcpy(bloq_datos, contenido2 + cant_escrita, cant_dentro_archivo - cant_escrita);
				cant_escrita = cant_dentro_archivo;
				}
			}
		}
	}
}


int escribir_archivo(char* path, char* contenido2, uint32_t size, off_t offset) {

	Gfile* nodo_archivo = buscar_archivo(path, tabla_nodos);

	if (nodo_archivo == NULL) {
		return -1;
	}

	int cant_dentro_archivo;
	int cant_fuera_archivo;
	int cant_agregada;

	if (offset + size > nodo_archivo->tam_archivo) {
		cant_dentro_archivo = nodo_archivo->tam_archivo - offset;
		cant_fuera_archivo = size - cant_dentro_archivo;

		cant_agregada = escribir_final_archivo(nodo_archivo, contenido2 + cant_dentro_archivo, cant_fuera_archivo);
		escribir_dentro_archivo(nodo_archivo, contenido2, cant_dentro_archivo, offset);


		if (cant_agregada == -1)
			return cant_dentro_archivo;

		printf("ESCRIBIR : tam retornado : %i\n\n*****\n", cant_dentro_archivo + cant_agregada);
		return cant_dentro_archivo + cant_agregada;
	}
	else {

		escribir_dentro_archivo(nodo_archivo, contenido2, size, offset);
		printf("ESCRIBIR : size retornado : %i\n\n*****\n", size);
		return size;
	}
}


/*Borro una parte del archivo. Útil para escribir a partir de un offset*/
int truncar_archivo_por_nodo (Gfile* nodo_archivo, off_t offset) {

	t_datos_comienzo_lectura indice_escritura;
	Bloque_ptr* bloq_ptr_dir;
	Gbloque* bloque_formatear;
	bool entro = true;


	int cant_bloques_archivo = cant_bloques_necesaria(nodo_archivo->tam_archivo);
	int cant_bloques_a_truncar = cant_bloques_archivo -  cant_bloques_necesaria(offset);


	if (offset > nodo_archivo->tam_archivo) {

		t_datos_comienzo_lectura indice_escritura = indices_comienzo_operacion(nodo_archivo->tam_archivo);

		int tam_escribir = offset - nodo_archivo->tam_archivo;

		int espacio_ult_bloque = BLOCK_SIZE - nodo_archivo->tam_archivo % 4096;


		if (offset < espacio_ult_bloque) {
			nodo_archivo->tam_archivo += offset;

			return 0;
		}


		else {
			int nro_bloque_libre_dir;
			int nro_bloque_libre;
			int bloques_agregar = cant_bloques_necesaria(tam_escribir - espacio_ult_bloque);
			Bloque_ptr* bloque_ptr_dir;
			Bloque_ptr* nuevo_bloque_ptr_dir;

			if (nodo_archivo->tam_archivo == 0) {
				nro_bloque_libre_dir = buscar_bloque_libre(bitmap);
				nodo_archivo->simple_indirect[0] = nro_bloque_libre_dir;

				nuevo_bloque_ptr_dir = (Bloque_ptr*) extraer_dir_bloque(disco, nro_bloque_libre_dir);

				memset(nuevo_bloque_ptr_dir, 0, BLOCK_SIZE);
				bitarray_set_bit(bitmap, nro_bloque_libre_dir);
			}

			for (int i = indice_escritura.indice_ptr_ind; i < CANT_INDIR_SIMPLE && bloques_agregar > 0; i++) {

				bloque_ptr_dir = (Bloque_ptr*) extraer_dir_bloque(disco, nodo_archivo->simple_indirect[i]);

			for (int j = i == indice_escritura.indice_ptr_ind ? indice_escritura.ind_ptr_dir : 0; j < CANT_DIRECTOS && bloques_agregar > 0; j++) {

				nro_bloque_libre = buscar_bloque_libre(bitmap);

				if(nro_bloque_libre == -1) {
					return -1;
				}

				bloque_ptr_dir->ptr_direct[j] = nro_bloque_libre;
				bitarray_set_bit(bitmap, nro_bloque_libre);


				bloques_agregar --;

				}

			if (bloques_agregar > 0) {
			nro_bloque_libre_dir = buscar_bloque_libre(bitmap);
			nodo_archivo->simple_indirect[i + 1] = nro_bloque_libre_dir;

			nuevo_bloque_ptr_dir = (Bloque_ptr*) extraer_dir_bloque(disco, nro_bloque_libre_dir);
			memset(nuevo_bloque_ptr_dir, 0, BLOCK_SIZE);

			bitarray_set_bit(bitmap, nro_bloque_libre_dir);

			}

			}
		}

		nodo_archivo->tam_archivo = offset;

		return 0;
	}

	indice_escritura = indices_comienzo_operacion(offset);
	int cant_bloques_datos_truncados = 0;


	for (int i = indice_escritura.indice_ptr_ind; i < CANT_INDIR_SIMPLE && nodo_archivo->simple_indirect[i] != 0 && cant_bloques_datos_truncados <= cant_bloques_a_truncar; i++ ) {

		bloq_ptr_dir = (Bloque_ptr*) extraer_dir_bloque(disco, nodo_archivo->simple_indirect[i]);

		for (int j = i > indice_escritura.indice_ptr_ind ? 0 : indice_escritura.ind_ptr_dir; j < CANT_DIRECTOS && bloq_ptr_dir->ptr_direct[j] != 0 && cant_bloques_datos_truncados <= cant_bloques_a_truncar; j++ ) {

			cant_bloques_datos_truncados++;

			if (j == indice_escritura.ind_ptr_dir &&  entro) {

				int nro_bloque_formatear = bloq_ptr_dir->ptr_direct[j];
				bloque_formatear = extraer_dir_bloque(disco, nro_bloque_formatear);

				if (BLOCK_SIZE - indice_escritura.offset_bloque_datos == 0)
					bitarray_clean_bit(bitmap, bloq_ptr_dir->ptr_direct[j]);
				//memset((char*) bloque_formatear + indice_escritura.offset_bloque_datos, 0, BLOCK_SIZE - indice_escritura.offset_bloque_datos);

				*((char*) bloque_formatear + indice_escritura.offset_bloque_datos) = '\0';
				entro = false;

			} else {


			bitarray_clean_bit(bitmap, bloq_ptr_dir->ptr_direct[j]);
			bloq_ptr_dir->ptr_direct[j] = 0;


			}
		}
		if (i > indice_escritura.indice_ptr_ind) {

			bitarray_clean_bit(bitmap, nodo_archivo->simple_indirect[i]);
			nodo_archivo->simple_indirect[i] = 0;
		}
	}


	nodo_archivo->tam_archivo = offset;

	return 0;
}


int truncar_archivo(char* path, off_t offset) {

	Gfile* archivo = buscar_archivo(path, tabla_nodos);

	if (archivo != NULL) {
		truncar_archivo_por_nodo(archivo, offset);

		return 0;
	}

	return -1;
}


int leer_archivo(char* path, char** contenido, int cant_leer, off_t offset) {

	Gfile* nodo_archivo = buscar_archivo(path, tabla_nodos);
	Bloque_ptr* bloque_ptrDir;
	Gbloque* bloque_datos;
//	char* contenido;

	if (nodo_archivo == NULL) {
		log_info(loggerINFO_DISK, "LEER: No se encontró el archivo \"%s\" para leer\n", path);
		return -1;
	}

	if (nodo_archivo->estado != OCUPADO) {
		log_info(loggerINFO_DISK, "LEER: \"%s\" no es un archivo o fue borrado\n", path);
		return -1;
	}

	int copiado = 0;
	int indice_bloque_datos;
	int a_copiar;
	int bytes_en_bloque;
	t_datos_comienzo_lectura indices_lectura;

	if (offset > nodo_archivo->tam_archivo) {
		log_info(loggerINFO_DISK, "LEER: El offset excede el tamaño del archivo.\n", path);
		return 0;
	}

	indices_lectura = indices_comienzo_operacion(offset);

	if ( nodo_archivo->tam_archivo - offset > cant_leer ) { //Controlo que la porción a leer esté dentro del archivo
		a_copiar = cant_leer;
		printf("Entré al if y a_copiar : %i\n", a_copiar);
	}
	else {
		a_copiar = nodo_archivo->tam_archivo - offset;
		printf("entré al else de a_copiar = nodo_archivo->tam_archivo - offset.  entonces a_copiar es %i\n", a_copiar);
	}


	printf(">>>>>>>>>>>>>>     Cantidad a leer :%i   a_copiar : %i\n", cant_leer, a_copiar);

	int fin_archivo = a_copiar;
	*contenido = malloc(a_copiar + 1);

	bool entro = true;
	//Controlar que los punteros se llenen en orden
	for (int i = indices_lectura.indice_ptr_ind; i < CANT_INDIR_SIMPLE && nodo_archivo->simple_indirect[i] != 0 && a_copiar > 0; i++) {

		//Extraigo el bloque con 1024 punteros
		bloque_ptrDir = (Bloque_ptr*) extraer_dir_bloque(disco, nodo_archivo->simple_indirect[i]);
//ACÁ ESETABA EL PROBLEMAAAAAAAAAAAAAAAAAAAAAAA  ?:
		for (int j = i == indices_lectura.indice_ptr_ind ? indices_lectura.ind_ptr_dir : 0; j < CANT_DIRECTOS && bloque_ptrDir->ptr_direct[j] != 0  && a_copiar > 0; j++) {

			indice_bloque_datos = bloque_ptrDir->ptr_direct[j];

			bloque_datos = extraer_dir_bloque(disco, indice_bloque_datos);

			if (j == indices_lectura.ind_ptr_dir && entro) { // si es la lectura del primer bloque

				bytes_en_bloque = BLOCK_SIZE - indices_lectura.offset_bloque_datos;
				bloque_datos = (void*) bloque_datos + indices_lectura.offset_bloque_datos;

				entro = false;
			}
			else bytes_en_bloque = BLOCK_SIZE;

			if (a_copiar > bytes_en_bloque) { //bytes en bloque es tam - offset en la primer iteración y BLOCK_SIZE	en el resto

						memcpy(*contenido + copiado, bloque_datos, bytes_en_bloque);
						copiado += bytes_en_bloque;
						a_copiar -= bytes_en_bloque;

					} else {

						memcpy(*contenido + copiado, bloque_datos, a_copiar);
						copiado += a_copiar;
						a_copiar = 0;
					}
			printf("En leer: copiado: %i   a_copiar: %i\n", copiado, a_copiar);
		}
	}

	printf("Salí del for y a_copiar : %i\n", a_copiar);
	memset(*contenido + fin_archivo, 0, sizeof(char));
	//*contenido2 = contenido;

	printf("Strlen del cont copiado : %i\n", strlen(*contenido));
	printf("Contenido en leer: %s\n", *contenido);
	return copiado;
}


void borrar_archivo_por_nodo(Gfile* nodo_elemento) {

		Bloque_ptr* bloque_ptrDir;
		int indice_bloque_ptr;
		int indice_bloque_datos;

	if (nodo_elemento->estado == OCUPADO) {

			for (int i = 0;
					i < CANT_INDIR_SIMPLE && nodo_elemento->simple_indirect[i] != 0;
					i++) {

				indice_bloque_ptr = nodo_elemento->simple_indirect[i];
				bloque_ptrDir = (Bloque_ptr*) extraer_dir_bloque(disco,
						indice_bloque_ptr);

				for (int j = 0;
						j < CANT_DIRECTOS && bloque_ptrDir->ptr_direct[j] != 0;
						j++) {
					indice_bloque_datos = bloque_ptrDir->ptr_direct[j];
					bitarray_clean_bit(bitmap, indice_bloque_datos);
					bloque_ptrDir->ptr_direct[j] = 0;
				}
				bitarray_clean_bit(bitmap, indice_bloque_ptr);
				nodo_elemento->simple_indirect[i] = 0;
			}
		}
		memset(nodo_elemento->nombre_archivo, 0, MAX_FILE_NAME_LEN);
		nodo_elemento->estado = BORRADO;
		nodo_elemento->tam_archivo = 0;
		nodo_elemento->bloque_padre = 0;
		nodo_elemento->fecha_creacion = 0;
		nodo_elemento->fecha_modificacion = 0;
}


int borrar_archivo(char* path) {

	Gfile* nodo_elemento = buscar_archivo(path, tabla_nodos);
	if (nodo_elemento == NULL) {
		log_info(loggerINFO_DISK, "BORRAR: El archivo no existe\n");
		return -1;
	}
	borrar_archivo_por_nodo(nodo_elemento);
	log_info(loggerINFO_DISK, "Archivo \"%s\" borrado\n", path);

	return 1;
}


void borrar_directorio_por_nodo(Gfile* direcctorio_a_borrar) {

	int indice_nodo_dir = indice_de_nodo(direcctorio_a_borrar);
	for (int i = 0; i < MAX_FILE_COUNT; i++) {

		if (tabla_nodos[i].bloque_padre == indice_nodo_dir && tabla_nodos[i].estado == OCUPADO) {

			borrar_archivo_por_nodo(&tabla_nodos[i]);
		}

		if (tabla_nodos[i].bloque_padre == indice_nodo_dir && tabla_nodos[i].estado == DIRECTORIO) {
			borrar_directorio_por_nodo(&tabla_nodos[i]);
		}
	}
	borrar_archivo_por_nodo(direcctorio_a_borrar);
}


int borrar_directorio(char* path) {

	Gfile* direcctorio_a_borrar = buscar_archivo(path, tabla_nodos);
	if (direcctorio_a_borrar == NULL || direcctorio_a_borrar->estado != DIRECTORIO) {
		log_info(loggerINFO_DISK, "BORRAR: No existe el directorio\n");
		return -1;
	}
	borrar_directorio_por_nodo(direcctorio_a_borrar);
	log_info(loggerINFO_DISK, "BORRAR: Directorio \"%s\" borrado\n", path);
	return 1;
}


void crear_raiz() {

		tabla_nodos[0] = crear_nodo(DIRECTORIO, "/", 0, 0);
}



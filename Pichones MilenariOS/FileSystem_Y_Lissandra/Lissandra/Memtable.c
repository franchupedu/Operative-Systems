#include "Memtable.h"


char* enMayuscula(char* palabra) {
	char* MPalabra = string_new();
	MPalabra = string_from_format("%s", palabra);
	string_to_upper(MPalabra);
	return MPalabra;
}

int tamanio_max_value() {

	configuracion = config_create("../FileSystem/Config.config");
	return config_get_int_value(configuracion, "Tamanio_Value");
}

int agregarTablaEnMemtable(char* nombreTabla) {
	char* nombreTablaMayus = enMayuscula(nombreTabla);

	struct NodoTabla* nuevaTabla = malloc(sizeof(struct NodoTabla));
	nuevaTabla->nombreTabla = nombreTablaMayus;
	nuevaTabla->primero = list_create();
	int posicion = list_add(listaTablas, nuevaTabla);
	struct NodoTabla* resultado = list_get(listaTablas, posicion); //A partir de cero
	printf("la tabla %s fue agregada exitosamente\n", resultado->nombreTabla);
//aaaaaaaaaaaaaaaaaaaaaaaaaaa
	return posicion;
}

bool laTablaSeLlamaAsi(struct NodoTabla* tabla, char* nombreTabla) {
	return string_equals_ignore_case(nombreTabla, tabla->nombreTabla);
}

struct NodoTabla* tablaEnMemtable(char* nombreTabla) {
	bool _existeUnaTablaConEseNombre(void *elemento) {
		return laTablaSeLlamaAsi((struct NodoTabla*) elemento, nombreTabla);
	}

	struct NodoTabla* resultado = list_find(listaTablas,
			_existeUnaTablaConEseNombre);
	/*if (resultado != NULL)
		printf("\nLa tabla %s está en memtable\n", resultado->nombreTabla);
	else
		printf("La tabla %s no existe en la memtable\n", nombreTabla);*/
	return resultado;
}

int agregarRegistroEnLaTablaConTS(char* nombreTabla, u_int16_t key, char* value,
		u_int timestamp) {
	struct NodoTabla* tablaObtenida = tablaEnMemtable(nombreTabla);

	if (tablaObtenida != NULL) {
		int tamanioAnterior = list_size(tablaObtenida->primero);
		struct dato* nuevoDato = malloc(sizeof(struct dato));
		nuevoDato->key = key;
		nuevoDato->value = string_substring_until(value, tamanio_max_value());
		printf("Value con tamaño maximo de %i bytes:  %s\n", tamanio_max_value(), nuevoDato->value);
		nuevoDato->timestamp = timestamp;
		list_add(tablaObtenida->primero, nuevoDato);
		int tamanioActual = list_size(tablaObtenida->primero);
		if (tamanioActual > tamanioAnterior) {
			printf("El registro fue agregado existosamente a la tabla\n");
			return 0;
		} else {
			printf("El registro no fue agregado\n");
			return 1;
		}
	} else {
		printf("No se agregaron registros nuevos. La tabla %s no existe\n", nombreTabla);
		return -1;
	}
}

int agregarRegistroEnLaTablaSinTS(char* nombreTabla, u_int16_t key, char* value) {
	return agregarRegistroEnLaTablaConTS(nombreTabla, key, value,
			(unsigned) time(NULL));
}

bool elRegistroTieneLaKey(struct dato* dato, u_int16_t key) {
	return dato->key == key;
}

bool mayorKeyPrimerElemento(struct dato* primero, struct dato* segundo) {
return primero->key > segundo->key ? true : false;
}

struct dato* max_key_lista(t_list* listaRegistros) {
	bool _mayorKeyPrimerElemento(void* primero, void* segundo) {
		return mayorKeyPrimerElemento(primero, segundo);
	}
list_sort(listaRegistros, _mayorKeyPrimerElemento);
struct dato* resultado = list_get(listaRegistros, 0);
return resultado;
}

//Devuelve el registro con la key más actualizada
struct dato* obtenerRegistroDeTabla(char* tabla, u_int16_t key) {

	struct NodoTabla* tablaMemtable = tablaEnMemtable(tabla);
		if (tablaMemtable != NULL) {

			bool _existeUnRegistroConLaKey(void* elemento) {
				return elRegistroTieneLaKey((struct dato*) elemento, key);
			}

			if(tablaMemtable->primero != NULL) {

				t_list* listaKeys = list_filter(tablaMemtable->primero,
				_existeUnRegistroConLaKey);

				struct dato* resultado = max_key_lista(listaKeys);
				if(resultado != NULL) {
					printf("Se encontró la key %i en la tabla %s\n", key, tabla);
					return resultado;
				} else {
					printf("La key %i no se encuentra en la tabla %s\n", key, tabla);
				}
			} else {
				printf("La tabla %s no tiene registros en la memtable\n", tabla);
				return NULL;
			}
} else {}


void limpiarRegistrosDeTabla(struct NodoTabla* tabla) {
	list_clean(tabla->primero);
	tabla->primero = NULL;

}

void limpiarTodasLasTablasEnMemtable(t_list* lista) {
	int tamanioLista =list_size(lista);
	for(int i = 0; i < tamanioLista; i++) {
		struct NodoTabla* tabla = list_get(lista,i);
		limpiarRegistrosDeTabla(tabla);
	}
}

void eliminarTabla(char* nombreTabla, t_list* tablas ) {
	struct NodoTabla* resultado = tablaEnMemtable(nombreTabla);
	if(resultado != NULL) {
		list_destroy_and_destroy_elements(resultado->primero, free);
		list_remove_and_destroy_element(tablas, posicion, free);
	}
}

int main() {




printf("\n\n-------------------\n");
listaTablas = list_create(); //Como variable global o como parámetro?


agregarTablaEnMemtable("calico");
agregarTablaEnMemtable("michi");
agregarTablaEnMemtable("peludo");
agregarTablaEnMemtable("naranja");
agregarTablaEnMemtable("juni");


int listaVacia = list_is_empty(listaTablas);
if (listaVacia)
	printf("la LISTA está vacia\n");
else {
	printf("la LISTA no está vacia. Tiene %i elementos\n\n",
			list_size(listaTablas));
	struct NodoTabla* resultadoB = list_get(listaTablas, 2);
	printf("el 3er elemento de la LISTA es:\n %s\n\n", resultadoB->nombreTabla);
}

agregarRegistroEnLaTablaConTS("calico", 99, "tres colores", 1000);
agregarRegistroEnLaTablaConTS("calico", 55, "nocturna", 10101);
agregarRegistroEnLaTablaConTS("calico", 4, "ninja", 3);
agregarRegistroEnLaTablaSinTS("calico", 16, "ratoncito");
agregarRegistroEnLaTablaConTS("calico", 99, "tres colores", 1511);
agregarRegistroEnLaTablaSinTS("calico", 19, "caja de arena");


struct NodoTabla* calico = tablaEnMemtable("calico");
int estaVacio = list_is_empty(calico->primero);
if (estaVacio)
	printf("la cola está vacia\n");
else {
	printf("la cola no está vacia. Tiene %i elementos\n\n",
			list_size(calico->primero));
	struct dato* datoExtraido = list_get(calico->primero, 0);
	printf("el 1er elemento de la cola es:\n time: %i, key: %i, value: %s\n\n",
			datoExtraido->timestamp, datoExtraido->key, datoExtraido->value);
}


obtenerRegistroDeTabla("calico", 19);
obtenerRegistroDeTabla("calico", 99);
obtenerRegistroDeTabla("calico", 909);
limpiarTodasLasTablasEnMemtable(listaTablas);
obtenerRegistroDeTabla("calico", 19);


printf("\n----------------\n\n");

return 0;
}


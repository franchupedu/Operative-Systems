#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "Memtable.h"
#include <commons/string.h>

void pushTabla(struct NodoTabla* pila, char* nuevoNombreTabla) {
	struct NodoTabla* nuevo = malloc(sizeof(struct NodoTabla));
	nuevo->nombreTabla = nuevoNombreTabla;
	nuevo->sig = pila;
	pila = nuevo;
}

//Pasar la tabla donde deseo guardar el dato y la estructura del dato
void pushDato(struct NodoTabla* tabla, struct dato nuevoDato) {
	struct NodoDato* nuevo = malloc(sizeof(struct NodoTabla));
	nuevo->elDato = nuevoDato;
	nuevo->sig = tabla->primero;
	tabla->primero = nuevo;
}

bool buscarTablaBool(char* nombreTabla, struct NodoTabla* pila) {
	while (pila != NULL && strcmp(nombreTabla, pila->nombreTabla) != 0)
		pila = pila->sig;

	return pila != NULL && criterio(nombreTabla, pila->nombreTabla) == 0;
}

struct NodoTabla *buscarTablaPuntero(char* nombreTabla, struct NodoTabla* pila) {
	while (pila != NULL && strcmp(nombreTabla, pila->nombreTabla) != 0)
		pila = pila->sig;

	return pila != NULL && strcmp(nombreTabla, pila->nombreTabla) == 0 ?
			pila : NULL;
}

struct NodoTabla buscarDatoPuntero(char* key, struct NodoTabla* pila) {
	while (pila != NULL && pila->primero != NULL
			&& strcmp(key, pila->primero->elDato->key) != 0)
		pila->primero = pila->primero->sig;

	return pila != NULL && pila->primero != NULL
			&& strcmp(key, pila->nombreTabla) == 0 ? pila->primero : NULL;
}

struct dato popDato(struct NodoDato* pila) {
	struct NodoDato *aux = pila;
	struct dato valor = pila->elDato;
	struct NodoDato* aux_elim = pila;
	pila = pila->sig;
	free(aux_elim);
	return valor;
}

char* borrarTabla(char* nombreTabla, struct NodoTabla *pila) {
	struct NodoTabla *aux = pila;
	struct NodoTabla *tablaAEliminar = buscarTablaPuntero(nombreTabla, pila);
	if (tablaAEliminar) {
		if (!pila)
			exit(EXIT_FAILURE);
		if (!strcmp(pila->nombreTabla, tablaAEliminar->nombreTabla)) {
			pila = pila->sig;
			free(tablaAEliminar);
			return nombreTabla;
		}
		while (aux->sig->nombreTabla != nombreTabla)
			aux = aux->sig;
		aux->sig = tablaAEliminar->sig;
		free(tablaAEliminar);
		return nombreTabla;
	}
	exit (EXIT_FAILURE);
}

int main() {

	return 0;
}

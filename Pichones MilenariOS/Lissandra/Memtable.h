/*
 * Memtable.h
 *
 *  Created on: 22 may. 2019
 *      Author: utnso
 */

#ifndef LISSANDRA_MEMTABLE_H_
#define LISSANDRA_MEMTABLE_H_

struct dato {
	int timestamp;
	u_int16_t key;
	char* value;
};


struct NodoTabla {
	char* nombreTabla; // valor que contiene el nodo
	struct NodoTabla* sig; // puntero al siguiente nodo
	struct NodoDato* primero;
};

struct NodoDato {
	struct dato elDato;
	struct NodoDato* sig;
};

void push(struct NodoTabla* pila, char* nuevoNombreTabla);

#endif /* LISSANDRA_MEMTABLE_H_ */

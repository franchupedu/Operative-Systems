#include "ParseoDeRequests.h"

// Función que sirve para parsear identificadores de tablas o de cualquier tipo.
int esalfanumerico (char caracter) {
	if (caracter >= '0' && caracter <= '9')
		return 1;

	else if (caracter >= 'A' && caracter <= 'Z')
		return 1;

	else if (caracter >= 'a' && caracter <= 'z')
		return 1;

	return 0;
}

// Función que parsea la reuest select individualmente.
int parsearselect (char* palabra) {
	int i = 7;

	// Primero se fija que el identificador de la tabla sea válido, sino devuelve el código de error de esta request que es igual a su estado de aceptación + 1.
	if (isdigit (palabra [i]))
		return aceptacionrequest [0] + 1;

	do {
		if (! esalfanumerico (palabra [i]))
			return aceptacionrequest [0] + 1;

		i++;
	} while (palabra [i] != ' ' && palabra [i]);

	i++;

	// Luego se fija que la key solicitada sea un número, sino devuelve el código de error.
	do {
		if (! isdigit (palabra [i]))
			return aceptacionrequest [0] + 1;

		i++;
	} while (palabra [i]);

	// Si pasa esas dos pruebas de parseo devuelve el código de operación de la request que es igual a su estado de aceptación (6).
	return aceptacionrequest [0];
}

// Función que parsea la request insert individualmente.
int parsearinsert (char *palabra) {
	int i = 7;

	// Primero se fija que el idntificador de la tabla sea válido, sino devuelve el código de error que es igual a su estado de aceptación + 1.
	if (isdigit (palabra [i]))
		return aceptacionrequest [1] + 1;

	do {
		if (! esalfanumerico (palabra [i]))
			return aceptacionrequest [1] + 1;

		i++;
	} while (palabra [i] != ' ' && palabra [i]);

	i++;

	// Segundo se fija que la key sea un número, sino devuelve el código de error.
	do {
		if (! isdigit (palabra [i]))
			return aceptacionrequest [1] + 1;

		i++;
	} while (palabra [i] != ' ' && palabra [i]);

	i++;

	// Si el valor a insertar no esta entre comillas devuelve el código de error.
	if (palabra [i] != '"')
		return aceptacionrequest [1] + 1;

	// Este ciclo se fija que la request no termine sin cerrar las comillas, osea que no se encuentre con el caracter '\0' antes.
	do {
		i++;

		if (! palabra [i])
			return aceptacionrequest [1] + 1;
	} while (palabra [i] != '"');

	i++;

	if (palabra [i] != ' ')
		return aceptacionrequest [1] + 1;

	i++;

	// Por último se fija que el timestamp sea un número, sino devuelve el código de error.
	do {
		if (! isdigit (palabra [i]))
			return aceptacionrequest [1] + 1;

		i++;
	} while (palabra [i]);

	// Si pasa todas las pruebas de parseo devuelve el código de operación que es igual a su estado de aceptación (12).
	return aceptacionrequest [1];
}

// Función que devuelve 1 si el estado actual es de aceptación en la tabla de criterios de consistencia y 0 si no lo es.
int esdeaceptacionconsistencia (int actual) {
	for (int i = 0; i < 3; i++) {
		if (actual == aceptacionconsistencia [i])
			return 1;
	}

	return 0;
}

// Función que parsea la request create individualmente.
int parsearcreate (char *palabra) {
	int actual = 0;
	int i = 7;
	int j;

	// Primero nos fijamos que el identificador de la tabla sea válido, sino devuelve el código de error que es igual a su estado de aceptación + 1.
	if (isdigit (palabra [i]))
		return aceptacionrequest [2] + 1;

	do {
		if (! esalfanumerico (palabra [i]))
			return aceptacionrequest [2] + 1;

		i++;
	} while (palabra [i] != ' ' && palabra [i]);

	i++;

	// Acá se parsea el tipo de consistencia que va a tener la tabla utilizando la tabla de transiciones de loa criterios de consistencia.
	do {
		j = 0;

		while (j < 4 && palabra [i] != caracteresconsistencia [j])
			j++;

		// Acá se utiliza la tabla de transiciones de los criterios de consistencia.
		actual = transicionesconsistencia [actual] [j];

		i++;
	} while (palabra [i] != ' ' && palabra [i]);

	i++;

	// Si no es un tipo de consistencia válido devualve el código de error.
	if (! esdeaceptacionconsistencia (actual))
		return aceptacionrequest [2] + 1;

	// Acá se fija que el número de particiones de la tabla sea efectivamente un número.
	do {
		if (! isdigit (palabra [i]))
			return aceptacionrequest [2] + 1;

		i++;
	} while (palabra [i] != ' ' && palabra [i]);

	i++;

	// Aca se fija que el tiempo de compactación de la tabla sea un número.
	do {
		if (! isdigit (palabra [i]))
			return aceptacionrequest [2] + 1;

		i++;
	} while (palabra [i]);

	// Si pasa todas las pruebas de parseo devuelve el código de operación que es igual a su estado de aceptación (18).
	return aceptacionrequest [2];
}

// Función que parsea la request describe individualmente.
int parseardescribe (char *palabra) {
	int i = 8;

	// Esta request puede venir escrita de dos maneras así que si el siguiente caracter es el '\0' manda el código de operación de esta request (26).
	if (! palabra [i])
		return aceptacionrequest [3];

	i++;

	// Acá se contempla la segunda manera que tiene un identificador de tabla y se fija que sea válido, si no lo es devuelve el código de error de esta request que es igual a su estado de aceptación + 1.
	if (isdigit (palabra [i]))
		return aceptacionrequest [3] + 1;

	do {
		if (! esalfanumerico (palabra [i]))
			return aceptacionrequest [3] + 1;

		i++;
	} while (palabra [i]);

	// Si es válido devuelve el código de operación que es igual a su estado de aceptación (26).
	return aceptacionrequest [3];
}

// Función que parsea la request drop individualmente.
int parseardrop (char *palabra) {
	int i = 5;

	// Acá ae fija que sea un identificador de tabla válido, sino devuelve el código de error que es igual al estado de aceptación de esta request + 1.
	if (isdigit (palabra [i]))
			return aceptacionrequest [4] + 1;

	do {
		if (! esalfanumerico (palabra [i]))
			return aceptacionrequest [4] + 1;

		i++;
	} while (palabra [i]);

	// Si es válido devuelve el código de operación que es igual al estado de aceptación de esta request (29).
	return aceptacionrequest [4];
}

// Función que parsea la request journal individualmente.
int parsearjournal (char *palabra) {
	// Esta request se escribe solo "journal" asi que si el proximo caracter no es '\0' ya está mal escrita y envia el código de error de esta request que es igual a su estado de aceptación + 1.
	if (palabra [7])
		return aceptacionrequest [5] + 1;

	// Si esta bien escrita manda el código de operación de esta request ques es igual a su estado de aceptación (36).
	return aceptacionrequest [5];
}

// Función que parsea la request add individualmente.
int parsearadd (char *palabra) {
	int actual = 0;
	int i = 11;
	int j;

	// Este ciclo se fija que el identificador de la memoria a agregar al criterio sea un número, sino tira el código de error de esta request que es igual a su estado de aceptación + 1.
	do {
		if (! isdigit (palabra [i]))
			return aceptacionrequest [6] + 1;

		i++;
	} while (palabra [i] != ' ' && palabra [i]);

	i++;

	// Me fijo que la siguiente palabra sea "to".
	if (palabra [i] != 't')
		return aceptacionrequest [6] + 1;

	i++;

	if (palabra [i] != 'o')
		return aceptacionrequest [6] + 1;

	i++;

	if (palabra [i] != ' ')
		return aceptacionrequest [6] + 1;

	i++;

	// Este ciclo parsea los criterios de consistencia a los que van a ser agregadas las memorias, estas tienen una tabla de transiciones especial ya que solo pueden ser ciertas palabras específicas.
	while (palabra [i]) {
		j = 0;

		if (palabra [i] == ' ')
			return aceptacionrequest [6] + 1;

		while (j < 4 && palabra [i] != caracteresconsistencia [j])
			j++;

		// Acá se usa la tabla de trancisiones de los criterios de consistencia.
		actual = transicionesconsistencia [actual] [j];

		i++;
	}

	// Si no es un criterio válido tira el código de error.
	if (! esdeaceptacionconsistencia (actual))
		return aceptacionrequest [6] + 1;

	// Si pasa todas las pruebas del parseo envia el código de operación que es igual a su estado de aceptación (46).
	return aceptacionrequest [6];
}

// Función que parsea la request run individualmente.
int parsearrun (char *palabra) {
	int i = 3;

	// Si no hay path envia el código de error de esta request que es igual a su estado de aceptación + 1.
	if (! palabra [i])
		return aceptacionrequest [7] + 1;

	i++;

	if (! palabra [i])
		return aceptacionrequest [7] + 1;

	// Si hay algún espacio en el path envia el código de error.
	do {
		if (palabra [i] == ' ')
			return aceptacionrequest [7] + 1;

		i++;
	} while (palabra [i]);

	// Si pasa todas las pruebas del parseo manda el código de operación que es igual a su estado de aceptación (49).
	return aceptacionrequest [7];
}

// Función que parsea la request métrics.
int parsearmetrics (char *palabra) {
	// Esta request se ecribe sensillamente "metrics" asi que si el caracter que le sigue a la palabra no es el '\0' ya esta mal escrita y devuelve un código de error.
	if (palabra [7])
		// El código de error es igual al estado de aceptación correspondiente a la request métrics + 1.
		return aceptacionrequest [8] + 1;

	// El código de operación es igual al estado de aceptación de esta request (56).
	return aceptacionrequest [8];
}

// Función que devuelve 1 si el estado actual del parser es de aceptación y 0 si no lo es.
int esdeaceptacionrequest (int estado) {
	for (int i = 0; i < 9; i++) {
		if (estado == aceptacionrequest [i])
			return 1;
	}

	return 0;
}

// Parseo la request y verifico que este bien escrita para devolver el código d operación.
int parsear (char *palabra) {
	int actual = 0;
	int i = 0;
	int j;

	// En este ciclo se parsea letra a letra la primera palabra de la request para saber a que operación nos enfrentamos y por lo tanto como hay que parsearla ya que cada request se parsea diferente.
	do {
		j = 0;

		while (j < 18 && palabra [i] != caracteresrequest [j]) {
			j++;
		}

		// Acá se hace uso de la tabla de transiciones para las requests como en cualquier parser.
		actual = transicionesrequest [actual] [j];

		// Si el estado actual del parser es de aceptación corta ya que el parseo minucioso se realiza una vez identificada la request.
		if (esdeaceptacionrequest (actual) && (! palabra [i + 1] || palabra [i + 1] == ' '))
			break;

		i++;
	} while (actual);

	// Este switch identifica la request por su código de operación y la parsea individualmente como debe ser parseada dicha request, si está mal escrita devuelve un código de error específico de cada request.
	switch (actual) {
		case 6: actual = parsearselect (palabra); break;
		case 12: actual = parsearinsert (palabra); break;
		case 18: actual = parsearcreate (palabra); break;
		case 26: actual = parseardescribe (palabra); break;
		case 29: actual = parseardrop (palabra); break;
		case 36: actual = parsearjournal (palabra); break;
		case 46: actual = parsearadd (palabra); break;
		case 49: actual = parsearrun (palabra); break;
		case 56: actual = parsearmetrics (palabra); break;
		default: break;
	}

	return actual;
}

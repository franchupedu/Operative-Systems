
#ifndef FILESYSTEM_Y_LISSANDRA_COMPACTADOR_COMPACTADOR_H_
#define FILESYSTEM_Y_LISSANDRA_COMPACTADOR_COMPACTADOR_H_

struct MetadataConfig {
	int consistencia;
	int particiones;
	int tiempoEntreCompactacion;
};

struct MetadataConfig configuracionMetadata;

#endif /* FILESYSTEM_Y_LISSANDRA_COMPACTADOR_COMPACTADOR_H_ */

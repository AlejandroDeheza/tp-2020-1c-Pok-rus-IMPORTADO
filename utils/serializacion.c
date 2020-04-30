#include "serializacion.h"

void* serializar_paquete(t_paquete* paquete, int *bytes)
{
	*bytes = paquete->buffer->size + sizeof(int) + sizeof(op_code);
	void * aEnviar = malloc(*bytes);
	int desplazamiento = 0;

	memcpy(aEnviar + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(aEnviar + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(aEnviar + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return aEnviar;
}


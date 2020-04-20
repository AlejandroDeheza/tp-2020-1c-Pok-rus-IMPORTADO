#include "serializacion.h"

void* serializar_paquete(t_paquete* paquete, int *bytes)
{
	*bytes = paquete->buffer->size + sizeof(int) + sizeof(op_code);
	printf("Hago malloc");
	void* aEnviar = malloc(*bytes);
	printf("Fin malloc");
	int offset = 0;

	memcpy(aEnviar + offset, &paquete->codigo_operacion, sizeof(op_code));//copiando codigo de operacion
	printf("SerializarPaquete -> Operación: %d (1 = MENSAJE).\n", *(int*)(aEnviar+offset));
	offset += sizeof(op_code);//desplazamiento

	memcpy(aEnviar + offset, &paquete->buffer->size, sizeof(int));//copia tamaño del stream (del contenido)
	printf("SerializarPaquete -> Size: %d.\n", *(int*)(aEnviar+offset));
	offset += sizeof(int);//desplazamiento


	memcpy(aEnviar + offset, paquete->buffer->stream, paquete->buffer->size);//copia el stream (el contenido)
	printf("SerializarPaquete -> Stream: \"%s\".\n", (char*)(aEnviar+offset));

	return aEnviar;
}

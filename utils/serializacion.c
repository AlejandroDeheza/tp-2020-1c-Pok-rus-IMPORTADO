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

void* serializar_new_pokemon(t_mensaje_new_pokemon* paquete, int *bytes)
{
	*bytes = paquete->buffer->size + sizeof(int) + sizeof(op_code) + sizeof(int) + sizeof(int) + sizeof(int);
	printf("Hago malloc \n");
	void* aEnviar = malloc(*bytes);
	printf("Fin malloc \n");
	int offset = 0;

	memcpy(aEnviar + offset, &paquete->codigo_operacion, sizeof(op_code));//copiando codigo de operacion
	printf("SerializarPaquete -> Operación: %d.\n", *(int*)(aEnviar+offset));
	offset += sizeof(op_code);//desplazamiento

	memcpy(aEnviar + offset, &paquete->buffer->size, sizeof(int));//copia tamaño del nombre (del contenido)
	printf("SerializarPaquete -> Size: %d.\n", *(int*)(aEnviar+offset));
	offset += sizeof(int);//desplazamiento


	memcpy(aEnviar + offset, paquete->buffer->nombre, paquete->buffer->size);//copia el stream (el contenido)
	printf("SerializarPaquete -> Nombre: \"%s\".\n", (char*)(aEnviar+offset));
	offset += sizeof(paquete->buffer->size);

	memcpy(aEnviar + offset, &paquete->buffer->posx, sizeof(int));//copia posicion x
	printf("SerializarPaquete -> PosX: %d.\n", *(int*)(aEnviar+offset));
	offset += sizeof(int);//desplazamiento

	memcpy(aEnviar + offset, &paquete->buffer->posy, sizeof(int));//copia posicion y
	printf("SerializarPaquete -> PosY: %d.\n", *(int*)(aEnviar+offset));
	offset += sizeof(int);//desplazamiento

	memcpy(aEnviar + offset, &paquete->buffer->cantidad, sizeof(int));//copia cantidad
	printf("SerializarPaquete -> Cantidad: %d.\n", *(int*)(aEnviar+offset));
	offset += sizeof(int);//desplazamiento

	return aEnviar;
}

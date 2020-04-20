#include "utils.h"


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

int crear_conexion(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;
}

//TODO
void enviar_mensaje(char* mensaje, int socket_cliente)
{
	int estado = 0;
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = TEAM;
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);
	printf("EnviarMensaje -> Mensaje Empaquetado: \"%s\".\n", (char*)paquete->buffer->stream);
	int bytes = 0;
	printf("voy a serializar");
	void* aEnviar = serializar_paquete(paquete, &bytes);
	printf("EnviarMensaje -> Paquete Serializado - Tamaño Total: %d Bytes.\n", bytes);
	estado = send(socket_cliente, aEnviar, bytes, 0);
	printf("pude hacer send");
	switch (estado) {
		case -1:
			printf("EnviarMensaje -> Error al enviar.\n");
			break;
		case 0:
			printf("EnviarMensaje -> No se pudo enviar nada.\n");
			break;
		default:
			printf("EnviarMensaje -> Paquete Enviado - %d Bytes transferidos.\n", estado);
			break;
	}
	printf("\n");
}

//TODO
char* recibir_mensaje(int socket_cliente)
{
	int codigo_operacion = 0;
		recv(socket_cliente, &codigo_operacion, sizeof(op_code), 0);
		int size;
		void* stream;
		char* string;
		switch (codigo_operacion) {
			case TEAM:
				printf("RecibirMensaje -> Operación: %d (1 = MENSAJE).\n", codigo_operacion);
				recv(socket_cliente,&size, sizeof(int), 0);
				printf("RecibirMensaje -> Size: %d Bytes.\n", size);
				stream = malloc(size);
				string = malloc(size);
				recv(socket_cliente,stream, size, 0);
				memcpy(string, stream, size);
				printf("RecibirMensaje -> Mensaje: \"%s\" - Longitud: %d.\n", string, strlen(string));
				break;
			default:
				printf("RecibirMensaje -> Error OpCode: %d.\n", codigo_operacion);
				break;
		}
		printf("\n");

		return string;
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

t_log* iniciar_logger(char* log_file)
{
	t_log* unLogger = log_create(log_file, "Team", true, LOG_LEVEL_INFO);

	return unLogger;
}

t_config* leer_config(char* file)
{
	return config_create(file);
}

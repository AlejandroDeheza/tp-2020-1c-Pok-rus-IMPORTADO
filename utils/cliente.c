#include "cliente.h"

#include "serializacion.h"

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

void enviar_mensaje(void* mensaje, int socket_cliente, op_code codigo_operacion)
{
	int estado = 0;

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo_operacion;
	paquete->buffer = malloc(sizeof(t_buffer));

	switch (codigo_operacion){
		case MENSAJE:
			printf("Creo un paquete para un MENSAJE");
			paquete->buffer->size = strlen(mensaje)+ 1;
			break;
		case NEW_POKEMON:
			printf("Creo un paquete para NEW_POKEMON");
			t_new_pokemon* new_pokemon = mensaje;
			paquete->buffer->size = sizeof(t_new_pokemon) +  new_pokemon->size;
			break;
	}

	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);
	printf("EnviarMensaje -> Mensaje Empaquetado: \"%s\".\n", (char*)paquete->buffer->stream);
	int bytes = 0;
	void* aEnviar = serializar_paquete(paquete, &bytes);
	printf("EnviarMensaje -> Paquete Serializado - Tamaño Total: %d Bytes.\n", bytes);

	estado = send(socket_cliente, aEnviar, bytes, 0);
	verificar_estado(estado);

	free(aEnviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	printf("\n");
}

t_new_pokemon* new_pokemon(char* nombre, int posx, int posy, int cantidad)
{
	t_new_pokemon* new_pokemon = malloc(sizeof(t_new_pokemon));
	new_pokemon->size = strlen(nombre) + 1;
	new_pokemon->nombre = malloc(new_pokemon->size);
	memcpy(new_pokemon->nombre, nombre, new_pokemon->size);
	new_pokemon->posicion.posx = posx;
	new_pokemon->posicion.posy = posy;
	new_pokemon->cantidad = cantidad;

	return new_pokemon;
}

void verificar_estado(int estado) {
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
}

char* recibir_mensaje(int socket_cliente) {
	int codigo_operacion = 0;
		recv(socket_cliente, &codigo_operacion, sizeof(op_code), 0);
		int size;
		void* stream;
		char* string;
		switch (codigo_operacion) {
			case MENSAJE:
				printf("RecibirMensaje -> Operación: %d (1 = MENSAJE).\n", codigo_operacion);
				recv(socket_cliente,&size, sizeof(int), 0);
				printf("RecibirMensaje -> Size: %d Bytes.\n", size);
				stream = malloc(size);
				string = malloc(size);
				recv(socket_cliente,stream, size, 0);

				memcpy(string, stream, size);
				printf("RecibirMensaje -> Mensaje: \"%s\" - Longitud: %d.\n", string, strlen(string));
				break;
			case NEW_POKEMON_RESPONSE:
				printf("Recibir Respuesta -> Operación: %s .\n", codigo_operacion);
				recv(socket_cliente,&size, sizeof(int), 0);
				printf("RecibirMensaje -> Size: %d Bytes.\n", size);
				stream = malloc(size);
				string = malloc(size);
				recv(socket_cliente,stream, size, 0);
				memcpy(string, stream, size);
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

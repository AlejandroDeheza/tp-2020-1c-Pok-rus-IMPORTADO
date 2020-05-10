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

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		printf("\n");
		error_show(" Error de conexion\n\n");
		exit(-1);
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(void* mensaje, int socket_cliente, op_code codigo_operacion)
{
	int estado = 0;
	int offset = 0;

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo_operacion;
	paquete->buffer = malloc(sizeof(t_buffer));

	switch (codigo_operacion){
		case IDENTIFICACION:
			printf("Creo un paquete para un identificarme\n");
			paquete->buffer->size = sizeof(int);
			paquete->buffer->stream = malloc(paquete->buffer->size);
			memcpy(paquete->buffer->stream + offset, mensaje, paquete->buffer->size);
			break;
		case MENSAJE:
			printf("Creo un paquete para un MENSAJE\n");
			paquete->buffer->size = strlen(mensaje)+ 1;
			paquete->buffer->stream = malloc(paquete->buffer->size);
			memcpy(paquete->buffer->stream + offset, mensaje, paquete->buffer->size);
			break;
		case NEW_POKEMON:
			printf("Creo un paquete para NEW_POKEMON\n");
			t_new_pokemon* new_pokemon = mensaje;
			paquete->buffer->size = sizeof(int)*4 + new_pokemon->size;
			paquete->buffer->stream = malloc(paquete->buffer->size);
			memcpy(paquete->buffer->stream + offset, &(new_pokemon->size), sizeof(int));
			offset += sizeof(int);
			memcpy(paquete->buffer->stream + offset, new_pokemon->nombre, new_pokemon->size);
			offset += new_pokemon->size;
			memcpy(paquete->buffer->stream + offset, &(new_pokemon->posicion.posx), sizeof(int));
			offset += sizeof(int);
			memcpy(paquete->buffer->stream + offset, &(new_pokemon->posicion.posy), sizeof(int));
			offset += sizeof(int);
			memcpy(paquete->buffer->stream + offset, &(new_pokemon->cantidad), sizeof(int));
			break;
		case APPEARED_POKEMON:
			printf("Creo un paquete para APPEARED_POKEMON\n");
			t_appeared_pokemon* appeared_pokemon = mensaje;
			paquete->buffer->size = sizeof(int)*3 + appeared_pokemon->size;
			paquete->buffer->stream = malloc(paquete->buffer->size);
			memcpy(paquete->buffer->stream + offset, &(appeared_pokemon->size), sizeof(int));
			offset += sizeof(int);
			memcpy(paquete->buffer->stream + offset, appeared_pokemon->nombre, appeared_pokemon->size);
			offset += appeared_pokemon->size;
			memcpy(paquete->buffer->stream + offset, &(appeared_pokemon->coordenadas.posx), sizeof(int));
			offset += sizeof(int);
			memcpy(paquete->buffer->stream + offset, &(appeared_pokemon->coordenadas.posy), sizeof(int));
			break;
		case CATCH_POKEMON:
			printf("Creo un paquete para CATCH_POKEMON\n");
			t_catch_pokemon* catch_pokemon = mensaje;
			paquete->buffer->size = sizeof(int)*3 + catch_pokemon->size;
			paquete->buffer->stream = malloc(paquete->buffer->size);
			memcpy(paquete->buffer->stream + offset, &(catch_pokemon->size), sizeof(int));
			offset += sizeof(int);
			memcpy(paquete->buffer->stream + offset, catch_pokemon->nombre, catch_pokemon->size);
			offset += catch_pokemon->size;
			memcpy(paquete->buffer->stream + offset, &(catch_pokemon->coordenadas.posx), sizeof(int));
			offset += sizeof(int);
			memcpy(paquete->buffer->stream + offset, &(catch_pokemon->coordenadas.posy), sizeof(int));
			break;
		case CAUGHT_POKEMON:
			printf("Creo un paquete para CAUGHT_POKEMON\n");
			paquete->buffer->size = sizeof(int);
			paquete->buffer->stream = malloc(paquete->buffer->size);
			memcpy(paquete->buffer->stream + offset, mensaje, sizeof(int));
			break;
		case GET_POKEMON:
			printf("Creo un paquete para GET_POKEMON\n");
			t_get_pokemon* get_pokemon = mensaje;
			paquete->buffer->size = sizeof(int) + get_pokemon->size;
			paquete->buffer->stream = malloc(paquete->buffer->size);
			memcpy(paquete->buffer->stream + offset, &(get_pokemon->size), sizeof(int));
			offset += sizeof(int);
			memcpy(paquete->buffer->stream + offset, get_pokemon->nombre, get_pokemon->size);
			break;
	}

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

t_appeared_pokemon* appeared_pokemon(char* nombre, int posx, int posy)
{
	t_appeared_pokemon* appeared_pokemon = malloc(sizeof(t_appeared_pokemon));
	appeared_pokemon->size = strlen(nombre) + 1;
	appeared_pokemon->nombre = malloc(appeared_pokemon->size);
	memcpy(appeared_pokemon->nombre, nombre, appeared_pokemon->size);
	appeared_pokemon->coordenadas.posx = posx;
	appeared_pokemon->coordenadas.posy = posy;

	return appeared_pokemon;
}

t_catch_pokemon* catch_pokemon(char* nombre, int posx, int posy)
{
	t_catch_pokemon* catch_pokemon = malloc(sizeof(t_catch_pokemon));
	catch_pokemon->size = strlen(nombre) + 1;
	catch_pokemon->nombre = malloc(catch_pokemon->size);
	memcpy(catch_pokemon->nombre, nombre, catch_pokemon->size);
	catch_pokemon->coordenadas.posx = posx;
	catch_pokemon->coordenadas.posy = posy;

	return catch_pokemon;
}

t_caught_pokemon* caught_pokemon(int resultado)
{
	t_caught_pokemon* caught_pokemon = malloc(sizeof(t_caught_pokemon));
	caught_pokemon->resultado= resultado;

	return caught_pokemon;
}

t_get_pokemon* get_pokemon(char* nombre)
{
	t_get_pokemon* get_pokemon = malloc(sizeof(t_get_pokemon));
	get_pokemon->size = strlen(nombre) + 1;
	get_pokemon->nombre = malloc(get_pokemon->size);
	memcpy(get_pokemon->nombre, nombre, get_pokemon->size);

	return get_pokemon;
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

void* recibir_mensaje(int socket_cliente) {
	int codigo_operacion = 0;
	recv(socket_cliente, &codigo_operacion, sizeof(op_code), 0);
	int size;
	void* stream;
	char* string;
	switch (codigo_operacion) {
		case IDENTIFICACION:
		case MENSAJE:
			printf("RecibirMensaje -> Operación: %d (1 = MENSAJE).\n", codigo_operacion);
			recv(socket_cliente,&size, sizeof(int), 0);
			printf("RecibirMensaje -> Size: %d Bytes.\n", size);
			stream = malloc(size);
		//	string = malloc(size);
			recv(socket_cliente,stream, size, 0);
		//	memcpy(string, stream, size);
		//	printf("RecibirMensaje -> Mensaje: \"%s\" - Longitud: %d.\n", string, strlen(string));
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

		return stream;
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

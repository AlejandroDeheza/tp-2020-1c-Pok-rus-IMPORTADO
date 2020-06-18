#include "cliente.h"

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
		/*
		printf("\n");
		error_show(" Error de conexion\n\n");
		exit(-1);
		*/
		return -1;
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void iniciar_conexion(int* conexion, t_config* config, t_log* logger, char *nombre_proceso){
	char* ip = NULL;
	char* puerto = NULL;

	leer_ip_y_puerto(&ip, &puerto, config, nombre_proceso);

	*conexion = crear_conexion( ip, puerto);

	if(*conexion > 0){
		log_info(logger, "Se realizo una conexion con %s", nombre_proceso);
	}
}


void enviar_mensaje(void* mensaje, int socket_cliente, op_code codigo_operacion, int id_mensaje, int id_correlativo)
{
	int estado = 0;

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo_operacion;
	paquete->id_correlativo = id_correlativo;	//este id lo puede settear el proceso que manda el mensaje
												//tambien lo puede dejar en 0 si no conoce el id
	paquete->id_mensaje = id_mensaje;	//EL ID_MENSAJE SIEMPRE LO SETEA EL BROKER
								//a menos que el gameboy envie un mensaje a gamecard
								//en ese caso lo seteamos nosotros desde consola
	paquete->buffer = malloc(sizeof(t_buffer));

	switch (codigo_operacion){
		case NEW_POKEMON:
			printf("Creo un paquete para NEW_POKEMON\n");
			serializar_new_pokemon(&paquete, mensaje);
			break;

		case APPEARED_POKEMON:
			printf("Creo un paquete para APPEARED_POKEMON\n");
			serializar_appeared_pokemon(&paquete, mensaje);
			break;

		case CATCH_POKEMON:
			printf("Creo un paquete para CATCH_POKEMON\n");
			serializar_catch_pokemon(&paquete, mensaje);

			break;
		case CAUGHT_POKEMON:
			printf("Creo un paquete para CAUGHT_POKEMON\n");
			serializar_caught_pokemon(&paquete, mensaje);

			break;
		case GET_POKEMON:
			printf("Creo un paquete para GET_POKEMON\n");
			serializar_get_pokemon(&paquete, mensaje);
			break;

		case LOCALIZED_POKEMON:
			printf("Creo un paquete para LOCALIZED_POKEMON\n");
			serializar_localized_pokemon(&paquete, mensaje);
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
	printf("\n");
}

//Es bastante parecido a enviar_mensaje, pero este no serializa, ya que el broker al enterarse de que recibio un mensaje,
// lo redistribuye y no pierde tiempo deserializando y serializando
void enviar_mensaje_a_suscriptores(void* mensaje, int size_mensaje, int socket_cliente, op_code codigo_operacion, int id_mensaje, int id_correlativo)
{
	int estado = 0;

	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo_operacion;
	paquete->id_correlativo = id_correlativo;	//este id lo puede settear el proceso que manda el mensaje
												//tambien lo puede dejar en 0 si no conoce el id
	paquete->id_mensaje = id_mensaje;	//EL ID_MENSAJE SIEMPRE LO SETEA EL BROKER
								//a menos que el gameboy envie un mensaje a gamecard
								//en ese caso lo seteamos nosotros desde consola
	paquete->buffer = malloc(sizeof(t_buffer));

	paquete->buffer->size = size_mensaje;
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
}

void enviar_new_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, int posx, int posy, int cantidad)
{
	t_new_pokemon* new_pokemon = malloc(sizeof(t_new_pokemon));
	new_pokemon->size = strlen(nombre) + 1;
	new_pokemon->nombre = malloc(new_pokemon->size);
	memcpy(new_pokemon->nombre, nombre, new_pokemon->size);
	new_pokemon->coordenadas.posx = posx;
	new_pokemon->coordenadas.posy = posy;
	new_pokemon->cantidad = cantidad;

	enviar_mensaje(new_pokemon, conexion, NEW_POKEMON, id_mensaje, id_correlativo);
	free(new_pokemon->nombre);
	free(new_pokemon);
}

void enviar_appeared_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, int posx, int posy)
{
	t_appeared_pokemon* appeared_pokemon = malloc(sizeof(t_appeared_pokemon));
	appeared_pokemon->size = strlen(nombre) + 1;
	appeared_pokemon->nombre = malloc(appeared_pokemon->size);
	memcpy(appeared_pokemon->nombre, nombre, appeared_pokemon->size);
	appeared_pokemon->coordenadas.posx = posx;
	appeared_pokemon->coordenadas.posy = posy;

	enviar_mensaje(appeared_pokemon, conexion, APPEARED_POKEMON, id_mensaje, id_correlativo);
	free(appeared_pokemon->nombre);
	free(appeared_pokemon);
}

void enviar_catch_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, int posx, int posy)
{
	t_catch_pokemon* catch_pokemon = malloc(sizeof(t_catch_pokemon));
	catch_pokemon->size = strlen(nombre) + 1;
	catch_pokemon->nombre = malloc(catch_pokemon->size);
	memcpy(catch_pokemon->nombre, nombre, catch_pokemon->size);
	catch_pokemon->coordenadas.posx = posx;
	catch_pokemon->coordenadas.posy = posy;

	enviar_mensaje(catch_pokemon, conexion, CATCH_POKEMON, id_mensaje, id_correlativo);
	free(catch_pokemon->nombre);
	free(catch_pokemon);
}

void enviar_caught_pokemon(int conexion, int id_mensaje, int id_correlativo, int resultado)
{
	t_caught_pokemon* caught_pokemon = malloc(sizeof(t_caught_pokemon));
	caught_pokemon->resultado= resultado;

	enviar_mensaje(caught_pokemon, conexion, CAUGHT_POKEMON, id_mensaje, id_correlativo);
	free(caught_pokemon);
}

void enviar_get_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre)
{
	t_get_pokemon* get_pokemon = malloc(sizeof(t_get_pokemon));
	get_pokemon->size = strlen(nombre) + 1;
	get_pokemon->nombre = malloc(get_pokemon->size);
	memcpy(get_pokemon->nombre, nombre, get_pokemon->size);

	enviar_mensaje(get_pokemon, conexion, GET_POKEMON, id_mensaje, id_correlativo);
	free(get_pokemon->nombre);
	free(get_pokemon);
}

void enviar_localized_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, t_list* coordenadas)
{
	t_localized_pokemon* localized_pokemon = malloc(sizeof(t_localized_pokemon));
	localized_pokemon->size = strlen(nombre) + 1;
	localized_pokemon->nombre = malloc(localized_pokemon->size);
	memcpy(localized_pokemon->nombre, nombre, localized_pokemon->size);
	localized_pokemon->coordenadas = coordenadas;

	enviar_mensaje(localized_pokemon, conexion, LOCALIZED_POKEMON, id_mensaje, id_correlativo);
	free(localized_pokemon->nombre);
	list_destroy_and_destroy_elements(localized_pokemon->coordenadas, free);
	free(localized_pokemon);
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

void suscribirse_a_cola(int socket_cliente, op_code codigo_operacion) {
	int estado = 0;
	estado = send(socket_cliente, &codigo_operacion, sizeof(op_code), 0);
	verificar_estado(estado);
}

void* recibir_mensaje(int socket_cliente, pthread_mutex_t* mutex) {
	int codigo_operacion = 0;
	if(recv(socket_cliente, &codigo_operacion, sizeof(op_code), 0) == -1) {
		pthread_mutex_unlock(mutex);
		pthread_exit(NULL);
	}
	int id_mensaje = 0;
	recv(socket_cliente, &id_mensaje, sizeof(op_code), 0);
	int id_correlativo = 0;
	recv(socket_cliente, &id_correlativo, sizeof(op_code), 0);

	int size; //Aca hay que liberar?
	void* mensaje; //Aca hay que liberar?
	recv(socket_cliente,&size, sizeof(int), 0);
	printf("Socket %d", socket_cliente);
	printf("RecibirMensaje -> Size: %d Bytes.\n", size);
	mensaje = malloc(size);
	recv(socket_cliente,mensaje, size, 0);

	void* response;

	switch (codigo_operacion){
				case NEW_POKEMON:
					printf("Leo un NEW_POKEMON\n");
					response = deserializar_new_pokemon(mensaje);
					break;

				case APPEARED_POKEMON:
					printf("Leo un APPEARED_POKEMON\n");
					response = deserializar_appeared_pokemon(mensaje);
					break;

				case CATCH_POKEMON:
					printf("Leo un CATCH_POKEMON\n");
					response = deserializar_catch_pokemon(mensaje);
					break;

				case CAUGHT_POKEMON:
					printf("Leo un CAUGHT_POKEMON\n");
					response = deserializar_caught_pokemon(mensaje);
					break;

				case GET_POKEMON:
					printf("Leo un GET_POKEMON\n");
					response = deserializar_get_pokemon(mensaje);
					break;

				case LOCALIZED_POKEMON:
					printf("Leo un LOCALIZED_POKEMON\n");
					response = deserializar_localized_pokemon(mensaje);
					break;
			}
	return response;
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}


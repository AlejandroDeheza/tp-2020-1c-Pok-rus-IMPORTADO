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
		printf("\n");
		error_show(" Error de conexion\n\n");
		exit(-1);
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

int iniciar_conexion(t_config* config, t_log* logger, char *nombre_proceso){
	char* ip = NULL;
	char* puerto = NULL;
	int conexion;
	leer_ip_y_puerto(&ip, &puerto, config, nombre_proceso);

	conexion = crear_conexion( ip, puerto);

	log_info(logger, "Se realizo una conexion con %s", nombre_proceso);

	return conexion;
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
		case IDENTIFICACION:
			printf("Creo un paquete para un identificarme\n");
			serializar_identificacion(&paquete, mensaje);
			break;
		case MENSAJE:
			printf("Creo un paquete para un MENSAJE\n");
			serializar_mensaje(&paquete, mensaje);

			break;
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
}


void enviar_mensaje2(void* mensaje, int size_mensaje, int socket_cliente, op_code codigo_operacion, int id_mensaje, int id_correlativo)
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

	paquete->buffer->size = sizeof(mensaje);
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

void suscribirse_a_cola(int socket_cliente, op_code codigo_operacion)
{   int estado = 0;
	estado = send(socket_cliente, &codigo_operacion, sizeof(op_code), 0);
	verificar_estado(estado);
}


void recibir_mensaje(int socket_cliente) {
	int codigo_operacion;
	codigo_operacion = 0;
	if(recv(socket_cliente, &codigo_operacion, sizeof(op_code), 0) == -1) {
		//pthread_exit(NULL);
		return;
	}
	printf("Recibiendo mensaje");
	int size; //Aca hay que liberar?
	void* mensaje; //Aca hay que liberar?
	//printf("Recibir Respuesta -> Operación: %s .\n", codigo_operacion);
	recv(socket_cliente,&size, sizeof(int), 0);
	printf("RecibirMensaje -> Size: %d Bytes.\n", size);
	mensaje = malloc(size);
	//string = malloc(size);
	recv(socket_cliente,mensaje, size, 0);


	switch (codigo_operacion){
				//case MENSAJE:
				//	printf("RecibirMensaje -> Operación: %d (1 = MENSAJE).\n", codigo_operacion);
				//	recv(socket_cliente,&size, sizeof(int), 0);
				//	printf("RecibirMensaje -> Size: %d Bytes.\n", size);
				//	stream = malloc(size);
				//	string = malloc(size);
				//	recv(socket_cliente,stream, size, 0);
				//	memcpy(string, stream, size);
				//	printf("RecibirMensaje -> Mensaje: \"%s\" - Longitud: %d.\n", string, strlen(string));
				//	break;
				case IDENTIFICACION:
					printf("Leo un paquete para un identificarme\n");
					deserializar_identificacion(mensaje);
					break;
				case NEW_POKEMON:
					printf("Leo un NEW_POKEMON\n");
					deserializar_new_pokemon(mensaje);
					break;

				case APPEARED_POKEMON:
					printf("Leo un APPEARED_POKEMON\n");
					deserializar_appeared_pokemon(mensaje);
					break;

				case CATCH_POKEMON:
					printf("Leo un CATCH_POKEMON\n");
					deserializar_catch_pokemon(mensaje);

					break;
				case CAUGHT_POKEMON:
					printf("Leo un CAUGHT_POKEMON\n");
					deserializar_caught_pokemon(mensaje);

					break;
				case GET_POKEMON:
					printf("Leo un GET_POKEMON\n");
					deserializar_get_pokemon(mensaje);
					break;

				case LOCALIZED_POKEMON:
					printf("Leo un LOCALIZED_POKEMON\n");
					deserializar_localized_pokemon(mensaje);
					break;
			}
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

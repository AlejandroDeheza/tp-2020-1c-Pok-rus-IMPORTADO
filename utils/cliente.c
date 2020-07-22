#include "cliente.h"

int crear_socket_como_cliente(char *ip, char* puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1) return -1;

	freeaddrinfo(server_info);

	return socket_cliente;
}

int iniciar_conexion_como_cliente(char *nombre_de_proceso_servidor, t_config* config){
	char* ip = NULL;
	char* puerto = NULL;
	int conexion = 0;

	leer_ip_y_puerto(&ip, &puerto, config, nombre_de_proceso_servidor);

	conexion = crear_socket_como_cliente(ip, puerto);

	return conexion;
}

int enviar_mensaje_como_cliente(void* mensaje, int socket_cliente, op_code codigo_operacion, int id_mensaje, int id_correlativo)
{
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
			serializar_new_pokemon(&paquete, mensaje);
			break;

		case APPEARED_POKEMON:
			serializar_appeared_pokemon(&paquete, mensaje);
			break;

		case CATCH_POKEMON:
			serializar_catch_pokemon(&paquete, mensaje);

			break;
		case CAUGHT_POKEMON:
			serializar_caught_pokemon(&paquete, mensaje);

			break;
		case GET_POKEMON:
			serializar_get_pokemon(&paquete, mensaje);
			break;

		case LOCALIZED_POKEMON:
			serializar_localized_pokemon(&paquete, mensaje);
			break;

		default:
			break;
	}
	int bytes = 0;
	void* aEnviar = serializar_paquete(paquete, &bytes);
	printf("EnviarMensaje -> Paquete Serializado - Tamaño Total: %d Bytes.\n", bytes);	//PARA LAS PRUEBAS COMENTAR ESTO TODO

	int estado = send(socket_cliente, aEnviar, bytes, 0);
	verificar_estado(estado);	//PARA LAS PRUEBAS COMENTAR ESTO TODO
	free(aEnviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
	printf("\n");

	return estado;
}

int generar_y_enviar_new_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, int posx, int posy, int cantidad)
{
	t_new_pokemon* new_pokemon = malloc(sizeof(t_new_pokemon));
	new_pokemon->size = strlen(nombre) + 1;
	new_pokemon->nombre = malloc(new_pokemon->size);
	memcpy(new_pokemon->nombre, nombre, new_pokemon->size);
	new_pokemon->coordenadas.posx = posx;
	new_pokemon->coordenadas.posy = posy;
	new_pokemon->cantidad = cantidad;

	int estado = enviar_mensaje_como_cliente(new_pokemon, conexion, NEW_POKEMON, id_mensaje, id_correlativo);
	free(new_pokemon->nombre);
	free(new_pokemon);

	return estado;
}

int generar_y_enviar_appeared_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, int posx, int posy)
{
	t_appeared_pokemon* appeared_pokemon = malloc(sizeof(t_appeared_pokemon));
	appeared_pokemon->size = strlen(nombre) + 1;
	appeared_pokemon->nombre = malloc(appeared_pokemon->size);
	memcpy(appeared_pokemon->nombre, nombre, appeared_pokemon->size);
	appeared_pokemon->coordenadas.posx = posx;
	appeared_pokemon->coordenadas.posy = posy;

	int estado = enviar_mensaje_como_cliente(appeared_pokemon, conexion, APPEARED_POKEMON, id_mensaje, id_correlativo);
	free(appeared_pokemon->nombre);
	free(appeared_pokemon);

	return estado;
}

int generar_y_enviar_catch_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, int posx, int posy)
{
	t_catch_pokemon* catch_pokemon = malloc(sizeof(t_catch_pokemon));
	catch_pokemon->size = strlen(nombre) + 1;
	catch_pokemon->nombre = malloc(catch_pokemon->size);
	memcpy(catch_pokemon->nombre, nombre, catch_pokemon->size);
	catch_pokemon->coordenadas.posx = posx;
	catch_pokemon->coordenadas.posy = posy;

	int estado = enviar_mensaje_como_cliente(catch_pokemon, conexion, CATCH_POKEMON, id_mensaje, id_correlativo);
	free(catch_pokemon->nombre);
	free(catch_pokemon);

	return estado;
}

int generar_y_enviar_caught_pokemon(int conexion, int id_mensaje, int id_correlativo, int resultado)
{
	t_caught_pokemon* caught_pokemon = malloc(sizeof(t_caught_pokemon));
	caught_pokemon->resultado= resultado;

	int estado = enviar_mensaje_como_cliente(caught_pokemon, conexion, CAUGHT_POKEMON, id_mensaje, id_correlativo);
	free(caught_pokemon);

	return estado;
}

int generar_y_enviar_get_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre)
{
	t_get_pokemon* get_pokemon = malloc(sizeof(t_get_pokemon));
	get_pokemon->size = strlen(nombre) + 1;
	get_pokemon->nombre = malloc(get_pokemon->size);
	memcpy(get_pokemon->nombre, nombre, get_pokemon->size);

	int estado = enviar_mensaje_como_cliente(get_pokemon, conexion, GET_POKEMON, id_mensaje, id_correlativo);
	free(get_pokemon->nombre);
	free(get_pokemon);

	return estado;
}

int generar_y_enviar_localized_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, t_list* coordenadas)
{
	t_localized_pokemon* localized_pokemon = malloc(sizeof(t_localized_pokemon));
	localized_pokemon->size = strlen(nombre) + 1;
	localized_pokemon->nombre = malloc(localized_pokemon->size);
	memcpy(localized_pokemon->nombre, nombre, localized_pokemon->size);
	localized_pokemon->coordenadas = coordenadas;

	int estado = enviar_mensaje_como_cliente(localized_pokemon, conexion, LOCALIZED_POKEMON, id_mensaje, id_correlativo);
	free(localized_pokemon->nombre);
	list_destroy_and_destroy_elements(localized_pokemon->coordenadas, free);
	free(localized_pokemon);

	return estado;
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

int enviar_mensaje_de_suscripcion(int socket_cliente, op_code codigo_operacion, int id_manual_del_proceso)
{	//si id_manual_del_proceso == 0, entonces el BROKER lo setea en 0
	int estado = 0;

	void* aEnviar = malloc(sizeof(op_code) + sizeof(int));
	int desplazamiento = 0;

	memcpy(aEnviar + desplazamiento, &(codigo_operacion), sizeof(op_code));
	desplazamiento+= sizeof(int);
	memcpy(aEnviar + desplazamiento, &(id_manual_del_proceso), sizeof(int));

	estado = send(socket_cliente, aEnviar, sizeof(sizeof(op_code) + sizeof(int)), 0);
	verificar_estado(estado);

	return estado;
}

int esperar_id_mensaje_enviado(int socket_cliente)
{
	int id_mensaje_enviado = 0;
	if(recv(socket_cliente, &id_mensaje_enviado, sizeof(int), 0) <= 0)return 0;

	return id_mensaje_enviado;
}

// es practiamente igual a mensaje_de_suscripcion() pero bueno.. asi deberia ser el ack?
int enviar_ack(int socket_cliente, int id_mensaje_recibido){
	int estado = 0;
	estado = send(socket_cliente, &id_mensaje_recibido, sizeof(int), 0);
	verificar_estado(estado);

	return estado;
}

//saco los mutex porque eso se deberia manejar por afuera de la funcion
//no necesitan estar aca adentro
//asi podemos reutilizar esta funcion
void* recibir_mensaje_como_cliente(int socket_cliente, int* id_correlativo, int* id_mensaje)
{
	int codigo_operacion = 0;	//POR COMO MODELAMOS EL TP, LOS CLIENTES NO NECESITAN ESTO
	if(recv(socket_cliente, &codigo_operacion, sizeof(op_code), 0) <= 0)return NULL;
	//SI SE RECIBEN MENOS BYTES DE LOS PEDIDOS, DEBERIA TIRAR ERROR... TODO EL RECV() NO ERA BLOQUEANTE?
	//SI CAMBIO ESO ACA, TENGO QUE CAMBIARLO EN TODOS LOS RECV() DEL TP...

	if(recv(socket_cliente, id_correlativo, sizeof(int), 0) <= 0)return NULL;

	if(recv(socket_cliente, id_mensaje, sizeof(int), 0) <= 0)return NULL;
	int size;
	if(recv(socket_cliente,&size, sizeof(int), 0) <= 0)return NULL;

	void* mensaje = malloc(size);
	if(recv(socket_cliente,mensaje, size, 0) <= 0)return NULL;

	void* response;

	switch (codigo_operacion){
		case NEW_POKEMON:
			response = deserializar_new_pokemon(mensaje);
			break;

		case APPEARED_POKEMON:
			response = deserializar_appeared_pokemon(mensaje);
			break;

		case CATCH_POKEMON:
			response = deserializar_catch_pokemon(mensaje);
			break;

		case CAUGHT_POKEMON:
			response = deserializar_caught_pokemon(mensaje);
			break;

		case GET_POKEMON:
			response = deserializar_get_pokemon(mensaje);
			break;

		case LOCALIZED_POKEMON:
			response = deserializar_localized_pokemon(mensaje);
			break;
	}

	free(mensaje);
	return response;
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}


#ifndef TEAM_UTILS_H_
#define TEAM_UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/collections/list.h>
#include<readline/readline.h>
#include "estructuras.h"
#include <commons/error.h>
#include "serializacion.h"
#include "config.h"
#include<semaphore.h>

int crear_conexion(char* ip, char* puerto);
void iniciar_conexion(int* conexion, t_config* config, t_log* logger, char *nombre_proceso, char *tipo_mensaje);

void enviar_mensaje(void* mensaje, int socket_cliente, op_code codigo_operacion, int id_mensaje, int id_correlativo);
void enviar_mensaje_a_suscriptores(void* mensaje, int size_mensaje, int socket_cliente, op_code codigo_operacion, int id_mensaje, int id_correlativo);
void enviar_mensaje_de_suscripcion(int socket_cliente, op_code codigo_operacion);

void enviar_new_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, int posx, int posy, int cantidad);
void enviar_appeared_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, int posx, int posy);
void enviar_catch_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, int posx, int posy);
void enviar_caught_pokemon(int conexion, int id_mensaje, int id_correlativo, int resultado);
void enviar_get_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre);
void enviar_localized_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, t_list* coordenadas);

void enviar_ack(int socket_cliente, op_code codigo_operacion);
void* recibir_mensaje(int socket_cliente);
void liberar_conexion(int socket_cliente);
void verificar_estado(int estado);



#endif /* TEAM_UTILS_H_ */

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
#include <commons/error.h>
#include<semaphore.h>

#include "estructuras.h"
#include "serializacion.h"
#include "config.h"


int crear_socket_como_cliente(char* ip, char* puerto);
int iniciar_conexion_como_cliente(char* nombre_de_proceso_servidor, t_config* config);

void generar_y_enviar_new_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, int posx, int posy, int cantidad);
void generar_y_enviar_appeared_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, int posx, int posy);
void generar_y_enviar_catch_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, int posx, int posy);
void generar_y_enviar_caught_pokemon(int conexion, int id_mensaje, int id_correlativo, int resultado);
void generar_y_enviar_get_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre);
void generar_y_enviar_localized_pokemon(int conexion, int id_mensaje, int id_correlativo, char* nombre, t_list* coordenadas);

void verificar_estado(int estado);
int enviar_mensaje_como_cliente(void* mensaje, int socket_cliente, op_code codigo_operacion, int id_mensaje, int id_correlativo);
int enviar_mensaje_de_suscripcion(int socket_cliente, op_code codigo_operacion);
int enviar_ack(int socket_cliente, op_code codigo_operacion);

void* recibir_mensaje_como_cliente(int socket_cliente);
void liberar_conexion(int socket_cliente);

#endif /* TEAM_UTILS_H_ */

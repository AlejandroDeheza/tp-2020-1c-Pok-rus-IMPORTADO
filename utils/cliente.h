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

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(void* mensaje, int socket_cliente, op_code codigo_operacion);

void enviar_new_pokemon(int conexion, char* nombre, int posx, int posy, int cantidad);
void enviar_appeared_pokemon(int conexion, char* nombre, int posx, int posy);
void enviar_catch_pokemon(int conexion, char* nombre, int posx, int posy);
void enviar_caught_pokemon(int conexion, int resultado);
void enviar_get_pokemon(int conexion, char* nombre);
void enviar_localized_pokemon(int conexion, char* nombre, t_list* coordenadas);

void* recibir_mensaje(int socket_cliente);
void liberar_conexion(int socket_cliente);
void verificar_estado(int estado);


#endif /* TEAM_UTILS_H_ */

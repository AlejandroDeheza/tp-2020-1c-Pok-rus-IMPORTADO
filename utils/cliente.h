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
t_new_pokemon* new_pokemon(char* nombre, int posx, int posy, int cantidad);
t_appeared_pokemon* appeared_pokemon(char* nombre, int posx, int posy);
t_catch_pokemon* catch_pokemon(char* nombre, int posx, int posy);
t_caught_pokemon* caught_pokemon(int resultado);
t_get_pokemon* get_pokemon(char* nombre);

void* recibir_mensaje(int socket_cliente);
void liberar_conexion(int socket_cliente);
t_log* iniciar_logger(char*, char*);
void verificar_estado(int estado);


#endif /* TEAM_UTILS_H_ */

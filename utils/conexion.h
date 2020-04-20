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
#include<readline/readline.h>

typedef enum
{
	TEAM = 1
}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;


int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
char* recibir_mensaje(int socket_cliente);
void liberar_conexion(int socket_cliente);
t_log* iniciar_logger(char*, char*);


#endif /* TEAM_UTILS_H_ */

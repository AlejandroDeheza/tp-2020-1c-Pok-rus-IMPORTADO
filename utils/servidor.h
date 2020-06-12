#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/collections/list.h>
#include<commons/log.h>
#include<string.h>
#include<pthread.h>
#include "estructuras.h"
#include "serializacion.h"

int crear_socket_para_escuchar(char *ipServidor, char* puertoServidor);
int aceptar_una_conexion(int socket_servidor);
int tomar_cod_op_del_mensaje(int* socket);
int tomar_id_correlativo(int* socket);
int tomar_id_mensaje(int* socket);

void iniciar_servidor(char *ip, char* puerto);
void* recibir_buffer(int socket_cliente, int* size);
void process_request(int cod_op, int cliente_fd);
void devolver_mensaje(void* payload, int size, int socket_cliente, op_code operacion);

#endif /* SERVIDOR_H_ */

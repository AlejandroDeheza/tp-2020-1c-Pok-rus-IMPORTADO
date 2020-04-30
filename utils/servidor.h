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

void iniciar_servidor(char *ip, char* puerto);
void esperar_cliente(int);
void* recibir_mensaje_servidor(int socket_cliente, int* size);
void process_request(int cod_op, int cliente_fd);
void serve_client(int *socket);
void devolver_mensaje(void* payload, int size, int socket_cliente);

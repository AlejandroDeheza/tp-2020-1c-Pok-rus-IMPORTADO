#ifndef BROKER_H_
#define BROKER_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/string.h>
#include<commons/config.h>
#include<commons/collections/queue.h>
#include<commons/collections/list.h>
#include<readline/readline.h>
#include<pthread.h>

#include "../utils/servidor.h"
#include "../utils/config.h"

int ID_MANUAL_DEL_PROCESO = 0;
char* ID_PROCESOS_TP;

typedef struct
{
	int socket;
} argumentos_leer_mensajes;

void ejecutar_antes_de_terminar(int numero_senial);

void leer_mensajes(void* socket);
void thread_process_request(int codigo_operacion, int cliente_fd);
void suscribir(int cliente_fd, t_list *lista);
void desuscribir(int cliente_fd, t_list *lista);
bool es_igual_a_socket_a_desuscribir(void* elemento_de_lista);

void dar_aviso(int cliente_fd, t_list *listaDeSuscriptores, int op_code);
void list_elements(int client);
void esperar_ack(t_list *listaDeSuscriptores);

#endif

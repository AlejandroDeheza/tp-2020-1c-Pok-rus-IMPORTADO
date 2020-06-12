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

void tomar_parte_del_mensaje(int* socket);
void process_request(int cod_op, int cliente_fd);
void suscribir(int cliente_fd, t_list *lista);
void dar_aviso(int cliente_fd, t_list *listaDeSuscriptores, int op_code);
void list_elements(int client);

#endif

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
#include "cliente.h"

int crear_socket_para_escuchar(char *ipServidor, char* puertoServidor);
int aceptar_una_conexion(int socket_servidor);

int enviar_mensaje_a_suscriptores(void* mensaje, int size_mensaje, int socket_cliente, op_code codigo_operacion, int id_mensaje, int id_correlativo);

bool es_codigo_operacion_valido(op_code codigo_operacion);

bool es_un_proceso_esperado(int socket_cliente, char* id_procesos_tp, pthread_mutex_t* mutex_id_procesos_tp,
		void(*funcion_para_finalizar)(void), pthread_mutex_t* mutex_logger);

/**************vv funciones anteriores vv*****************/

#endif /* SERVIDOR_H_ */
